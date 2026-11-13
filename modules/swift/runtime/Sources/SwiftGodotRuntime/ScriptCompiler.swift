import Foundation

// AOT-compiles a project script with `swiftc` into a temporary shared
// library exposing a SwiftGodotScriptDescriptor, then dlopens it.
// When no Swift toolchain is available, the script keeps its parsed
// metadata (valid for the editor) but instances cannot execute methods.

@MainActor
public final class ScriptCompiler {
	@MainActor public static let shared = ScriptCompiler()

	private let swiftcPath: String?

	public init() {
		swiftcPath = ProcessInfo.processInfo.environment["PATH"]?.split(separator: ":")
			.map { String($0) + "/swiftc" }
			.first { FileManager.default.isExecutableFile(atPath: $0) }
	}

	public var isAvailable: Bool { swiftcPath != nil }

	func tryCompile(script: CompiledScript, className: String?, rootView: String?) {
		guard let swiftc = swiftcPath else {
			script.error = "swiftc not found in PATH; script metadata parsed but not compiled for execution."
			return
		}

		let dir = FileManager.default.temporaryDirectory
			.appendingPathComponent("swiftgodot-\(UUID().uuidString)", isDirectory: true)
		do {
			try FileManager.default.createDirectory(at: dir, withIntermediateDirectories: true)
			let sourceURL = dir.appendingPathComponent("script.swift")
			var imports = "import SwiftGodotRuntime\n"
			if rootView != nil {
				imports += "import SwiftGodotUI\n"
			}
			try (imports + script.source).write(to: sourceURL, atomically: true, encoding: .utf8)

			let methods = className != nil ? script.methods : []
			let entry = generateEntry(className: className, methods: methods, rootView: rootView)
			let entryURL = dir.appendingPathComponent("entry.swift")
			try entry.write(to: entryURL, atomically: true, encoding: .utf8)

			let libName = className ?? rootView ?? "script"
			let libURL = dir.appendingPathComponent("lib\(libName).so")
			// The runtime package must be pre-built (`swift build` in modules/swift/runtime):
			// its .build/debug holds the swiftmodules and libraries we compile against.
			let packageRoot = URL(fileURLWithPath: #filePath)
				.deletingLastPathComponent().deletingLastPathComponent().deletingLastPathComponent()
			let buildDir = packageRoot.appendingPathComponent(".build/debug", isDirectory: true)
			let arguments = [
				"-parse-as-library", "-emit-library",
				"-I", buildDir.appendingPathComponent("Modules", isDirectory: true).path,
				"-o", libURL.path,
				sourceURL.path, entryURL.path,
				"-L", buildDir.path,
				"-rpath", buildDir.path,
				"-lswiftgodot_runtime",
			]
			// SwiftGodotUI's symbols live in the same libswiftgodot_runtime
			// dylib (single dynamic product), so no extra -l flag is needed.
			let process = Process()
			process.executableURL = URL(fileURLWithPath: swiftc)
			process.arguments = arguments
			try process.run()
			process.waitUntilExit()
		 guard process.terminationStatus == 0 else {
				script.error = "swiftc failed with exit code \(process.terminationStatus)."
				return
			}

			guard let handle = dlopen(libURL.path, RTLD_NOW | RTLD_LOCAL) else {
				let message = dlerror().map { String(cString: $0) } ?? "unknown"
				script.error = "dlopen failed: \(message)"
				return
			}
			script.libraryPath = libURL.path
			Runtime.shared.attachLibrary(handle: handle, script: script)
		} catch {
			script.error = "Compilation failed: \(error.localizedDescription)"
		}
	}

	private func generateEntry(className: String?, methods: [String], rootView: String?) -> String {
		// methods is empty when className is nil, so classNameText is never emitted then.
		let classNameText = className ?? "AnyObject"
		var lines: [String] = []
		lines.append("import Foundation")
		lines.append("import SwiftGodotRuntime")
		if rootView != nil {
			lines.append("import SwiftGodotUI")
		}
		lines.append("")
		lines.append("private var methodNames: [UnsafePointer<CChar>?] = [")
		for method in methods {
			lines.append("\t\"\(method)\".withCString { strdup($0) },")
		}
		lines.append("]")
		lines.append("private var methodFns: [SwiftGodotMethodFn?] = [")
		for method in methods {
			lines.append("\t{ instance, args, argCount, ret, error in")
			lines.append("\t\treturn MainActor.assumeIsolated { () -> UInt8 in")
			lines.append("\t\t\tguard let instance = instance else { return 0 }")
			lines.append("\t\t\tlet object = instance.assumingMemoryBound(to: \(classNameText).self).pointee")
			if method == "_process" || method == "_physics_process" {
				lines.append("\t\t\tlet delta = argCount > 0 && args != nil ? Host.shared.variantAsDouble(args!.assumingMemoryBound(to: UnsafeRawPointer?.self)[0]) : 0.0")
				lines.append("\t\t\tobject.\(method)(delta: delta)")
			} else {
				lines.append("\t\t\tobject.\(method)()")
			}
			lines.append("\t\t\treturn 1")
			lines.append("\t\t}")
			lines.append("\t},")
		}
		lines.append("]")
		lines.append("private let newInstance: @convention(c) () -> UnsafeRawPointer? = {")
		if let className {
			lines.append("\tMainActor.assumeIsolated { Unmanaged.passRetained(\(className)()).toOpaque() }")
		} else {
			lines.append("\tnil")
		}
		lines.append("}")
		lines.append("private var descriptor = SwiftGodotScriptDescriptor()")
		lines.append("")
		lines.append("@_cdecl(\"swiftgodot_script_descriptor\")")
		lines.append("public func swiftgodot_script_descriptor() -> UnsafePointer<SwiftGodotScriptDescriptor> {")
		lines.append("\tdescriptor.class_name = \"\(className ?? rootView ?? "Anonymous")\".withCString { strdup($0) }")
		lines.append("\tdescriptor.new_instance = newInstance")
		lines.append("\tdescriptor.method_count = Int32(methodNames.count)")
		lines.append("\tdescriptor.method_names = methodNames.withUnsafeBufferPointer { UnsafeMutablePointer(mutating: $0.baseAddress) }")
		lines.append("\tdescriptor.method_fns = methodFns.withUnsafeBufferPointer { UnsafeMutablePointer(mutating: $0.baseAddress) }")
		lines.append("\treturn withUnsafePointer(to: &descriptor) { UnsafePointer($0) }")
		lines.append("}")
		if let rootView {
			lines.append("")
			lines.append("@_cdecl(\"swiftgodot_scene_register\")")
			lines.append("public func swiftgodot_scene_register() {")
			lines.append("\tMainActor.assumeIsolated { SceneRegistry.shared.registerRoot(\(rootView)(), named: \"\(rootView)\") }")
			lines.append("}")
		}
		return lines.joined(separator: "\n")
	}
}
