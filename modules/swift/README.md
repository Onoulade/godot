# Swift module

Native Swift scripting language support for Godot, as a built-in module
(enabled with `module_swift_enabled=yes`, disabled by default like `mono`).

## Architecture

The module is split in two halves that only meet through a small C ABI
(`swift_bridge.h`):

- **C++ side (this directory):** registers the `Swift` language with
  `ScriptServer`, recognizes `.swift` files (`ResourceFormatLoaderSwift` /
  `ResourceFormatSaverSwift`), exposes the `SwiftScript` resource type,
  implements `SwiftInstance` (a `ScriptInstance` that forwards calls,
  properties, signals and notifications to the runtime), and provides the
  editor integration (`SwiftEditorLanguage` validation/outline, syntax
  highlighting through `EditorStandardSyntaxHighlighter`, global class
  detection from `class Name: Base` declarations).
- **Swift side (`runtime/`, a Swift package built with `swiftc`):** a shared
  library (`swiftgodot_runtime`) exporting the C ABI. It receives the
  engine's GDExtension function resolver at init (like a GDExtension
  library), parses script metadata, and AOT-compiles project scripts with
  `swiftc` into per-script shared libraries exposing a
  `SwiftGodotScriptDescriptor` used for method dispatch.

The C++ side loads the runtime at startup with `OS::open_dynamic_library()`
(searched next to the executable, then `res://bin`, then `res://`), so the
engine compiles on machines without a Swift toolchain. Without the runtime,
`.swift` files still load and edit, but `SwiftScript.can_instantiate()`
returns `false` and a warning is printed.

## Status (piliers)

1. **Module skeleton** — language registration, `.swift` resource I/O, C ABI
   bridge loading. Done.
2. **Swift runtime package** — `runtime/` (Package.swift + ABI entry points +
   metadata parsing + `swiftc` AOT compilation of project scripts). Done;
   requires a Swift toolchain to produce `swiftgodot_runtime` (see below).
3. **Script instances** — `SwiftInstance` forwarding `callp`/`set`/`get`/
   `notification`/method & property lists through the bridge; common engine
   notifications are mapped to `_ready`, `_process`, `_physics_process`,
   `_enter_tree`, `_exit_tree`, `_draw`. Done.
4. **Editor integration** — `SwiftEditorLanguage` (validation with brace
   balancing, function list for the outline, `find_function`),
   syntax highlighting via `EditorStandardSyntaxHighlighter` fed by the
   language metadata, global class cache from `class` declarations,
   built-in script template, PascalCase file naming. Done.
5. **Bindings, Variant marshaling and host services** — `swift_host.h/.cpp`
   exposes a typed host vtable to the runtime (Variant scalars/copy/object,
   `object_call`/`object_get`/`object_set`, `node_get_node`/`node_get_parent`,
   print/errors), handed over via `swiftgodot_host_init`. On the Swift side,
   `Host`/`VariantBox` wrap it, and `tools/generate_bindings.py` generates
   Swift classes from `extension_api.json` (checked-in generated file:
   `runtime/Sources/SwiftGodotRuntime/Generated/GodotBindings.swift`;
   regenerate in CI with an editor build's `--dump-extension-api`). Scene
   linking works through the generated bindings (`get_node`, properties,
   `emit_*` signals); `@export var` is detected and reported with editor
   property usage. `_process`/`_physics_process` receive the delta.
   The checked-in bindings cover the full runtime API (1002 classes,
   regenerated in CI from `--dump-extension-api`); typed Swift value types
   (`Vector2`, `Vector3`, `Color`, `Quaternion`, `Rect2`, `AABB`, `Plane`,
   `Transform2D/3D`, `Basis`, `Projection`, ...) are marshaled through
   dedicated host vtable entries (`ValueTypes.swift`).
6. **Debugger/profiler, completion, export, CI** — call-stack tracking and
   per-method profiling in `SwiftLanguage` (fed by `SwiftInstance::callp`),
   editor completion (keywords, builtin types, global classes, owner
   members) and symbol lookup in `SwiftEditorLanguage`, a `SwiftExportPlugin`
   that bundles `.swift` sources and the runtime library into exports, and
   `.github/workflows/swift_module.yml` (runtime build + symbol checks,
   engine build with the module, bindings freshness check).
7. **Swift 6 / @MainActor thread safety** — the runtime package is
   swift-tools 6.0; every generated binding class (`GodotObject` and all
   1002 engine classes) is `@MainActor`, so Swift 6 strict concurrency
   rejects off-main-thread engine calls at compile time. The runtime
   singletons (`Host`, `Runtime`, `ScriptCompiler`) are `@MainActor` and the
   C entry points enter them via `MainActor.assumeIsolated` (the engine only
   calls scripts on the main thread). Value types are `Sendable`; their
   marshaling entry points are `@MainActor`. User scripts can therefore use
   `@MainActor`, `async`/`await` and actors freely.
   **Remaining:** user docs (separate documentation repo). Class enums are nested `enum: Int, Sendable, CaseIterable`
   (e.g. `Node.ProcessMode.inherit`); global enums (Key, Error,
   MouseButton, PropertyHint, ...) are extracted from the core headers
   (`tools/generate_bindings.py --global-enums-headers`) and emitted as
   top-level `enum: Int, Sendable, CaseIterable`, with signatures mapped
   to them (e.g. `is_key_pressed(key: Key)`, `-> Error`).

## Declarative UI (SwiftUI-style scenes)

On top of the script layer, `SwiftGodotUI` (in `runtime/`, depending on
`SwiftGodotRuntime` and on Apple's
[swift-observation](https://github.com/apple/swift-observation) package)
adds a SwiftUI-style declarative layer. Design decisions:

- **Leaf primitives are native Godot nodes.** Scenes are built from the
  generated bindings (`Label()`, `Button()`, `VBoxContainer()`, ...) wrapped
  in `NodeView<Root: Node>`. SwiftUI-like container wrappers
  (VStack/HStack/...) will be layered on top later. To make this possible,
  the host vtable gained `object_create` (ClassDB::instantiate) and every
  generated class gets a `public convenience init()` (e.g. `Label()`);
  enum-typed arguments now also get their default value mapped to a Swift
  case (`add_child(node:)` just works).
- **Diff engine = component stack + attribute graph** with scoped,
  propagated invalidation: `ComponentStack`/`ComponentFrame` give each
  component a stable identity (type + position) while `body` evaluates,
  and `Differ` reconciles the rendered Godot node tree in place instead of
  rebuilding it.
- **Observation = Apple's real dependency** (`@Observable`,
  `withObservationTracking` from swift-observation), not a hand-rolled
  didSet. `@State` is a property wrapper over an `@Observable` box.

Files: `View.swift` (`@MainActor protocol View`, `@ViewBuilder`,
`EmptyView`, `NodeView`, `TupleView`, `OptionalView`, `EitherView`),
`AttributeGraph.swift` (tracked evaluation + coalesced main-actor flush),
`State.swift`, `Differ.swift`, `Scene.swift` (the `swiftgodot_scene_build` /
`swiftgodot_scene_invalidate` C entry points and `SceneRegistry`).

Engine side: `SwiftViewHost` (a `Node`) calls `scene_build` on
`NOTIFICATION_READY` and exposes `invalidate()`; the ABI gained
`scene_build`/`scene_invalidate`. A script declaring `struct MyScene: View`
gets a generated `swiftgodot_scene_register` entry point that registers the
root view (by name) in `SceneRegistry`; each host renders the view named by
its `root_view` property (several hosts can host different scenes) and
re-renders through the attribute graph + differ on invalidation: `renderTree()`
produces a `RenderedView` (node + component identity), and the differ keeps
same-type containers stable, reconciles children pairwise by position, and
swaps leaves in place. Compiling scene scripts requires the runtime package
to be pre-built (`swift build` in `modules/swift/runtime`); `ScriptCompiler`
links each per-script library against its `.build/debug`.

**Precompiled scripts at export (`.swiftbin`):** exported games have no
`swiftc`. In the editor, every successful compile is cached next to the
source as `<script>.swiftbin` (via the ABI's `script_get_library_path`);
outside the editor, `SwiftScript::reload` loads the `.swiftbin` through
`script_load_precompiled` instead of compiling, and `SwiftExportPlugin`
bundles the `.swiftbin` files into the export. The editor always recompiles
from source, so a stale cache is never used there.

**Status:** done except container wrappers (VStack/HStack/... on top of
`TupleView`) and user docs (separate documentation repo).

## Editor completion (LSP) and typed collections

- **LSP completion** — `swift_lsp.h/.cpp` (editor builds only) runs a
  persistent `sourcekit-lsp` process (stdio pipes via `OS::execute_with_pipe`,
  LSP 3.17 JSON-RPC with `Content-Length` framing) and answers
  `textDocument/completion` for `.swift` files, merged with the local
  keyword/type/global-class/owner-member lists in
  `SwiftEditorLanguage::complete_code`. The binary is looked up in the
  `swift/lsp_path` project setting, the `SOURCEKIT_LSP` environment variable,
  then the `PATH`; documents are synced with `didOpen`/`didChange` (full
  text) and the server is shut down with the editor. Without sourcekit-lsp,
  completion silently falls back to the local lists.
- **Typed Array/Dictionary wrappers** — the host vtable gained Array and
  Dictionary variant operations, exposed in Swift as `GodotArray` /
  `GodotDictionary` (`Collections.swift`) with `VariantBoxConvertible`
  conformances for scalars, value types, engine objects and the collections
  themselves; `VariantBox.asArray()` / `.asDictionary()` give borrowed views
  of Array/Dictionary variants (e.g. method returns).
- **CI fix** — Apple's Observation framework is part of the Swift toolchain
  standard library (`stdlib/public/Observation` in swiftlang/swift) on all
  platforms, so the runtime package has no external dependency: `import
  Observation` resolves against the toolchain directly (the previously used
  `apple/swift-observation` package URL does not exist and broke CI).

## Building

```sh
scons platform=<platform> module_swift_enabled=yes
```

The Swift runtime package is built automatically by `scons` when a Swift
toolchain (`swift` in `PATH`) is available: the produced
`libswiftgodot_runtime.{so,dylib,dll}` and `libSwiftGodotUI.{so,dylib,dll}`
are copied next to the Godot executable in `bin/`, where the module loads
them at startup (it also searches `../Frameworks/` inside macOS `.app`
bundles and `res://bin`). Set `SWIFTGODOT_SKIP_RUNTIME_BUILD=1` to skip this
step (e.g. on CI builders without Swift). To do it by hand instead (watch the `swift module:` lines in the scons output
to see whether the toolchain was found):

```sh
swift build -c debug --package-path modules/swift/runtime   # editor/template_debug; use -c release otherwise
cp modules/swift/runtime/.build/debug/libswiftgodot_runtime.{so,dylib,dll} \
   modules/swift/runtime/.build/debug/libSwiftGodotUI.{so,dylib,dll} \
   bin/
# for a macOS .app bundle, put them in Godot.app/Contents/Frameworks/ instead
```

If the runtime library is missing at startup, the editor prints a warning and
Swift scripts load but cannot be instantiated (no crash): attach the runtime
libraries and restart.

Regenerating the bindings (needs an editor build to dump the API):

```sh
godot --headless --dump-extension-api   # produces extension_api.json
python3 modules/swift/tools/generate_bindings.py --api extension_api.json --all \
    --exclude SwiftViewHost \
    --out modules/swift/runtime/Sources/SwiftGodotRuntime/Generated/GodotBindings.swift
```

(`--exclude` skips module-provided classes such as `SwiftViewHost` that the
editor dump also contains.)
