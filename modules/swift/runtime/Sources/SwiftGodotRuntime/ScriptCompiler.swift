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

	public func tryCompile(script: CompiledScript, className: String) {
		guard let swiftc = swiftcPath else {
			script.error = "swiftc not found in PATH; script metadata parsed but not compiled for execution."
			return
		}

		let dir = FileManager.default.temporaryDirectory
			.appendingPathComponent("swiftgodot-\(UUID().uuidString)", isDirectory: true)
		do {
			try FileManager.default.createDirectory(at: dir, withIntermediateDirectories: true)
			let sourceURL = dir.appendingPathComponent("script.swift")
			try script.source.write(to: sourceURL, atomically: true, encoding: .utf8)

			let entry = generateEntry(className: className, methods: script.methods)
			let entryURL = dir.appendingPathComponent("entry.swift")
			try entry.write(to: entryURL, atomically: true, encoding: .utf8)

			let libURL = dir.appendingPathComponent("lib\(className).so")
			let process = Process()
			process.executableURL = URL(fileURLWithPath: swiftc)
			process.arguments = [
				"-parse-as-library", "-emit-library",
				"-o", libURL.path,
				sourceURL.path, entryURL.path,
			]
			try process.run()
			process.waitUntilExit()
		 guard process.terminationStatus == 0 else {
				script.error = "swiftc failed with exit code \(process.terminationStatus)."
				return
			}

			guard let handle = dlopen(libURL.path, RTLD_NOW | RTLD_LOCAL) else {
				script.error = "dlopen failed: \(String(cString: dlerror() ?? "unknown"))"
				return
			}
			guard let descriptorPtr = dlsym(handle, "swiftgodot_script_descriptor") else {
				script.error = "Compiled library does not export swiftgodot_script_descriptor."
				dlclose(handle)
				return
			}
			typealias DescriptorFn = @convention(c) () -> UnsafePointer<SwiftGodotScriptDescriptor>
			let descriptorFn = unsafeBitCast(descriptorPtr, to: DescriptorFn.self)
			script.libraryHandle = handle
			script.descriptor = descriptorFn()
		} catch {
			script.error = "Compilation failed: \(error.localizedDescription)"
		}
	}

	private func generateEntry(className: String, methods: [String]) -> String {
		var lines: [String] = []
		lines.append("import Foundation")
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
			lines.append("\t\t\tlet object = instance.assumingMemoryBound(to: \(className).self).pointee")
			if method == "_process" || method == "_physics_process" {
				lines.append("\t\t\tlet delta = argCount > 0 && args != nil ? Host.shared.variantAsDouble(args![0]) : 0.0")
				lines.append("\t\t\tobject.\(method)(delta: delta)")
			} else {
				lines.append("\t\t\tobject.\(method)()")
			}
			lines.append("\t\t\treturn 1")
			lines.append("\t\t}")
			lines.append("\t},")
		}
		lines.append("]")
		lines.append("private let newInstance: @convention(c) () -> UnsafeMutableRawPointer? = {")
		lines.append("\tMainActor.assumeIsolated { Unmanaged.passRetained(\(className)()).toOpaque() }")
		lines.append("}")
		lines.append("private var descriptor = SwiftGodotScriptDescriptor()")
		lines.append("")
		lines.append("@_cdecl(\"swiftgodot_script_descriptor\")")
		lines.append("public func swiftgodot_script_descriptor() -> UnsafePointer<SwiftGodotScriptDescriptor> {")
		lines.append("\tdescriptor.class_name = \"\(className)\".withCString { strdup($0) }")
		lines.append("\tdescriptor.new_instance = newInstance")
		lines.append("\tdescriptor.method_count = Int32(methodNames.count)")
		lines.append("\tdescriptor.method_names = methodNames.withUnsafeBufferPointer { UnsafeMutablePointer(mutating: $0.baseAddress) }")
		lines.append("\tdescriptor.method_fns = methodFns.withUnsafeBufferPointer { UnsafeMutablePointer(mutating: $0.baseAddress) }")
		lines.append("\treturn withUnsafePointer(to: &descriptor) { UnsafePointer($0) }")
		lines.append("}")
		return lines.joined(separator: "\n")
	}
}
