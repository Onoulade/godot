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
5. **Remaining** — full Variant marshaling for method arguments (the ABI
   carries them; the generated entry points currently ignore them),
   debugger/profiler hooks (`debug_get_stack_level_*`, `profiling_*`),
   CI jobs building the runtime per platform, export templates, docs.

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
