# Swift module

Native Swift scripting language support for Godot, as a built-in module
(enabled with `module_swift_enabled=yes`, disabled by default like `mono`).

## Architecture

The module is split in two halves that only meet through a small C ABI:

- **C++ side (this directory):** registers the `Swift` language with
  `ScriptServer`, recognizes `.swift` files (`ResourceFormatLoaderSwift` /
  `ResourceFormatSaverSwift`), and exposes the `SwiftScript` resource type.
  It never includes Swift headers, so the engine builds on machines without a
  Swift toolchain.
- **Swift side (separate package, built with `swiftc`):** a shared library
  (`swiftgodot_runtime`, see `SwiftBridge::get_runtime_library_name()`) that
  compiles Swift sources, instantiates script classes, and marshals calls
  and properties with the engine. The C++ side loads it at runtime with
  `OS::open_dynamic_library()` (searched next to the executable, then in
  `res://bin` and `res://`) and resolves its entry points in
  `swift_bridge.cpp`.

If the runtime library is missing, the language still registers and `.swift`
files still load/edit fine, but `SwiftScript.can_instantiate()` returns
`false` and a warning is printed at startup.

## Roadmap (piliers)

1. **Module skeleton (this change):** language registration, `.swift`
   resource loading/saving, editor metadata (keywords, delimiters, templates),
   C ABI bridge loading. Scripts load but cannot run yet.
2. **Swift runtime package:** `swiftc` build of `swiftgodot_runtime`
   exporting the C ABI (`swiftgodot_runtime_version`, `swiftgodot_init`,
   `swiftgodot_shutdown`, plus parse/compile entry points), with a Package.swift
   and CI job.
3. **Script instances:** `SwiftScript.instance_create()` returning a
   `SwiftInstance : ScriptInstance` that forwards `callp`, `set`/`get`,
   `notification`, properties and signals through the bridge.
4. **Editor integration:** `SwiftEditorLanguage : EditorLanguage`
   (validation, completion, lookup), syntax highlighter, script templates
   with `extends`, global class registration (`class_name` equivalent).
5. **Tooling and CI:** cross-platform runtime builds, debugger/profiler
   hooks (`debug_get_stack_level_*`, `profiling_*`), export templates, and
   documentation.

## Building

```sh
scons platform=<platform> module_swift_enabled=yes
```

The Swift runtime library itself is built separately with the Swift toolchain
(see pilier 2) and must be placed next to the Godot executable or in
`res://bin` of the project.
