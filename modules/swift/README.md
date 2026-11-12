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
   **Remaining:** per-platform runtime builds in CI, precompiling scripts at
   export time (an exported game has no `swiftc`), user docs, LSP-based
   completion.

## Building

```sh
scons platform=<platform> module_swift_enabled=yes
```

The Swift runtime itself is built separately with the Swift toolchain:

```sh
cd modules/swift/runtime
swift build -c release
# then place the produced libswiftgodot_runtime.{so,dylib,dll}
# next to the Godot executable or in res://bin of the project.
```

Regenerating the bindings (needs an editor build to dump the API):

```sh
godot --headless --dump-extension-api   # produces extension_api.json
python3 modules/swift/tools/generate_bindings.py --api extension_api.json --all \
    --out modules/swift/runtime/Sources/SwiftGodotRuntime/Generated/GodotBindings.swift
```
