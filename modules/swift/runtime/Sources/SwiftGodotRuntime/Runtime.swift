import Foundation

// Variant::Type values mirrored from the engine (core/variant/variant.h).
public enum VariantTypeMirror: Int32 {
	case nil = 0
	case bool = 1
	case int = 2
	case float = 3
	case string = 4
}

@MainActor final class CompiledScript {
	let path: String
	let source: String
	var className: String?
	var rootViewClass: String?
	var isTool: Bool = false
	var methods: [String] = []
	var signals: [String] = []
	var properties: [(name: String, type: Int32, usage: Int32)] = []
	var error: String?
	var libraryHandle: UnsafeMutableRawPointer?
	var libraryPath: String?
	var descriptor: UnsafePointer<SwiftGodotScriptDescriptor>?

	init(path: String, source: String) {
		self.path = path
		self.source = source
	}

	var isValid: Bool {
		return className != nil && error == nil
	}
}

@MainActor final class ScriptInstanceBox {
	let script: CompiledScript
	let owner: UnsafeMutableRawPointer?
	let object: AnyObject?

	init(script: CompiledScript, owner: UnsafeMutableRawPointer?, object: AnyObject?) {
		self.script = script
		self.owner = owner
		self.object = object
	}
}

@MainActor
public final class Runtime {
	@MainActor public static let shared = Runtime()

	public typealias GetProcAddress = @convention(c) (UnsafePointer<CChar>) -> UnsafeMutableRawPointer?

	private var getProcAddress: GetProcAddress?
	private var scripts: [UnsafeMutableRawPointer: CompiledScript] = [:]
	private var instances: [UnsafeMutableRawPointer: ScriptInstanceBox] = [:]
	private let lock = NSLock()

	public func initialize(getProcAddress: @convention(c) (UnsafePointer<CChar>) -> UnsafeMutableRawPointer?) {
		self.getProcAddress = getProcAddress
	}

	public func shutdown() {
		lock.lock()
		for (_, script) in scripts {
			if let handle = script.libraryHandle {
				dlclose(handle)
			}
		}
		scripts.removeAll()
		instances.removeAll()
		lock.unlock()
	}

	func resolve<T>(_ name: String) -> T? {
		guard let getProcAddress = getProcAddress else { return nil }
		guard let ptr = name.withCString({ getProcAddress($0) }) else { return nil }
		return unsafeBitCast(ptr, to: T.self)
	}

	// MARK: - Script lifecycle

	public func compileScript(path: String, source: String) -> UnsafeMutableRawPointer? {
		let script = CompiledScript(path: path, source: source)
		parseMetadata(script)
		if script.className != nil || script.rootViewClass != nil {
			ScriptCompiler.shared.tryCompile(script: script, className: script.className, rootView: script.rootViewClass)
		}
		return store(script)
	}

	// Loads a precompiled script library (exported ".swiftbin") and attaches
	// its descriptor / scene registration without invoking swiftc.
	public func loadPrecompiled(libraryPath: String, path: String, source: String) -> UnsafeMutableRawPointer? {
		let script = CompiledScript(path: path, source: source)
		parseMetadata(script)
		script.libraryPath = libraryPath
		if let handle = dlopen(libraryPath, RTLD_NOW | RTLD_LOCAL) {
			attachLibrary(handle: handle, script: script)
		} else {
			let message = dlerror().map { String(cString: $0) } ?? "unknown"
			script.error = "dlopen failed on precompiled library: \(message)"
		}
		return store(script)
	}

	public func getScriptLibraryPath(_ handle: UnsafeMutableRawPointer?, buffer: UnsafeMutablePointer<CChar>?, bufferSize: Int32) {
		let path = script(for: handle)?.libraryPath ?? ""
		guard let buffer = buffer, bufferSize > 0 else { return }
		path.withCString { cString in
			strncpy(buffer, cString, Int(bufferSize) - 1)
			buffer[Int(bufferSize) - 1] = 0
		}
	}

	// Attaches an already-dlopened library to `script`: reads the script
	// descriptor and, for scene scripts, calls swiftgodot_scene_register.
	func attachLibrary(handle: UnsafeMutableRawPointer, script: CompiledScript) {
		guard let descriptorPtr = dlsym(handle, "swiftgodot_script_descriptor") else {
			script.error = "Compiled library does not export swiftgodot_script_descriptor."
			dlclose(handle)
			return
		}
		typealias DescriptorFn = @convention(c) () -> UnsafeRawPointer?
		let descriptorFn = unsafeBitCast(descriptorPtr, to: DescriptorFn.self)
		script.libraryHandle = handle
		script.descriptor = unsafeBitCast(descriptorFn(), to: UnsafePointer<SwiftGodotScriptDescriptor>.self)
		if script.rootViewClass != nil, let registerPtr = dlsym(handle, "swiftgodot_scene_register") {
			typealias RegisterFn = @convention(c) () -> Void
			unsafeBitCast(registerPtr, to: RegisterFn.self)()
		}
	}

	private func store(_ script: CompiledScript) -> UnsafeMutableRawPointer? {
		lock.lock()
		let handle = UnsafeMutableRawPointer(bitPattern: ObjectIdentifier(script).hashValue & 0x7FFF_FFFF)!
		scripts[handle] = script
		lock.unlock()
		return handle
	}

	public func freeScript(_ handle: UnsafeMutableRawPointer?) {
		guard let handle = handle else { return }
		lock.lock()
		if let script = scripts.removeValue(forKey: handle), let library = script.libraryHandle {
			dlclose(library)
		}
		lock.unlock()
	}

	public func isScriptValid(_ handle: UnsafeMutableRawPointer?) -> Bool {
		return script(for: handle)?.isValid ?? false
	}

	public func getScriptError(_ handle: UnsafeMutableRawPointer?, buffer: UnsafeMutablePointer<CChar>?, bufferSize: Int32) {
		let message = script(for: handle)?.error ?? ""
		guard let buffer = buffer, bufferSize > 0 else { return }
		message.withCString { cString in
			strncpy(buffer, cString, Int(bufferSize) - 1)
			buffer[Int(bufferSize) - 1] = 0
		}
	}

	public func getScriptClassName(_ handle: UnsafeMutableRawPointer?) -> UnsafeRawPointer? {
		guard let className = script(for: handle)?.className else { return nil }
		return className.withCString { strdup($0) }
	}

	public func isScriptTool(_ handle: UnsafeMutableRawPointer?) -> Bool {
		return script(for: handle)?.isTool ?? false
	}

	public func getScriptMethodCount(_ handle: UnsafeMutableRawPointer?) -> Int32 {
		return Int32(script(for: handle)?.methods.count ?? 0)
	}

	public func getScriptMethodName(_ handle: UnsafeMutableRawPointer?, index: Int32) -> UnsafeRawPointer? {
		guard let methods = script(for: handle)?.methods, index >= 0, index < methods.count else { return nil }
		return methods[Int(index)].withCString { strdup($0) }
	}

	public func scriptHasMethod(_ handle: UnsafeMutableRawPointer?, method: String) -> Bool {
		return script(for: handle)?.methods.contains(method) ?? false
	}

	public func getScriptSignalCount(_ handle: UnsafeMutableRawPointer?) -> Int32 {
		return Int32(script(for: handle)?.signals.count ?? 0)
	}

	public func getScriptSignalName(_ handle: UnsafeMutableRawPointer?, index: Int32) -> UnsafeRawPointer? {
		guard let signals = script(for: handle)?.signals, index >= 0, index < signals.count else { return nil }
		return signals[Int(index)].withCString { strdup($0) }
	}

	public func scriptHasSignal(_ handle: UnsafeMutableRawPointer?, signal: String) -> Bool {
		return script(for: handle)?.signals.contains(signal) ?? false
	}

	public func getScriptPropertyCount(_ handle: UnsafeMutableRawPointer?) -> Int32 {
		return Int32(script(for: handle)?.properties.count ?? 0)
	}

	public func getScriptPropertyName(_ handle: UnsafeMutableRawPointer?, index: Int32) -> UnsafeRawPointer? {
		guard let properties = script(for: handle)?.properties, index >= 0, index < properties.count else { return nil }
		return properties[Int(index)].name.withCString { strdup($0) }
	}

	public func getScriptPropertyType(_ handle: UnsafeMutableRawPointer?, index: Int32) -> Int32 {
		guard let properties = script(for: handle)?.properties, index >= 0, index < properties.count else { return 0 }
		return properties[Int(index)].type
	}

	public func getScriptPropertyUsage(_ handle: UnsafeMutableRawPointer?, index: Int32) -> Int32 {
		guard let properties = script(for: handle)?.properties, index >= 0, index < properties.count else { return 2 }
		return properties[Int(index)].usage
	}

	// MARK: - Instance lifecycle

	public func createInstance(script handle: UnsafeMutableRawPointer?, owner: UnsafeMutableRawPointer?) -> UnsafeMutableRawPointer? {
		guard let script = script(for: handle) else { return nil }
		var object: AnyObject?
		if let descriptor = script.descriptor, let newInstance = descriptor.pointee.new_instance, let raw = newInstance() {
			object = Unmanaged<AnyObject>.fromOpaque(UnsafeMutableRawPointer(mutating: raw)).takeUnretainedValue()
		}
		let box = ScriptInstanceBox(script: script, owner: owner, object: object)
		lock.lock()
		let instanceHandle = UnsafeMutableRawPointer(bitPattern: ObjectIdentifier(box).hashValue & 0x7FFF_FFFF)!
		instances[instanceHandle] = box
		lock.unlock()
		return instanceHandle
	}

	public func freeInstance(_ handle: UnsafeMutableRawPointer?) {
		guard let handle = handle else { return }
		lock.lock()
		instances.removeValue(forKey: handle)
		lock.unlock()
	}

	public func instanceCall(_ handle: UnsafeMutableRawPointer?, method: String, args: UnsafeMutableRawPointer?, argCount: Int32, ret: UnsafeMutableRawPointer?, error: UnsafeMutableRawPointer?) -> UInt8 {
		let errorMirror = error?.assumingMemoryBound(to: GDExtensionCallErrorMirror.self)
		guard let box = instance(for: handle), let descriptor = box.script.descriptor else {
			if let errorMirror = errorMirror {
				errorMirror.pointee.error = 1 // CALL_ERROR_INVALID_METHOD
			}
			return 0
		}
		let methodIndex = box.script.methods.firstIndex(of: method) ?? -1
		guard methodIndex >= 0, let fns = descriptor.pointee.method_fns, let fn = fns[methodIndex] else {
			if let errorMirror = errorMirror {
				errorMirror.pointee.error = 1
			}
			return 0
		}
		return fn(box.object.map { Unmanaged.passUnretained($0).toOpaque() }, args, argCount, ret, error)
	}

	public func instanceNotification(_ handle: UnsafeMutableRawPointer?, notification: Int32, reversed: Bool) {
		_ = instance(for: handle)
	}

	public func instanceSet(_ handle: UnsafeMutableRawPointer?, name: String, value: UnsafeRawPointer?) -> Bool {
		return instance(for: handle) != nil
	}

	public func instanceGet(_ handle: UnsafeMutableRawPointer?, name: String, ret: UnsafeMutableRawPointer?) -> Bool {
		return instance(for: handle) != nil
	}

	// MARK: - Metadata parsing

	private func parseMetadata(_ script: CompiledScript) {
		let source = script.source
		script.isTool = source.contains("@tool")

		if let match = firstMatch(of: "\\bclass\\s+([A-Za-z_][A-Za-z0-9_]*)", in: source) {
			script.className = match
		}
		if let view = firstMatch(of: "\\bstruct\\s+([A-Za-z_][A-Za-z0-9_]*)\\s*:[^{]*\\bView\\b", in: source) {
			script.rootViewClass = view
		}
		if script.className == nil && script.rootViewClass == nil {
			script.error = "No class or View declaration found."
			return
		}

		for match in allMatches(of: "\\bfunc\\s+([A-Za-z_][A-Za-z0-9_]*)\\s*\\(", in: source) {
			script.methods.append(match)
		}
		for match in allMatches(of: "\\bsignal\\s+([A-Za-z_][A-Za-z0-9_]*)", in: source) {
			script.signals.append(match)
		}
		for m in allMatchesWithGroups(of: "(@export\\s+)?\\bvar\\s+([A-Za-z_][A-Za-z0-9_]*)\\s*:\\s*([A-Za-z_][A-Za-z0-9_.]*)", in: source) {
			// PROPERTY_USAGE_STORAGE = 2, PROPERTY_USAGE_EDITOR = 4 (core/object/property_info.h).
			let usage: Int32 = m[0].contains("@export") ? 6 : 2
			script.properties.append((name: m[1], type: variantType(forSwiftType: m[2]), usage: usage))
		}
	}

	private func variantType(forSwiftType type: String) -> Int32 {
		switch type {
		case "Bool": return VariantTypeMirror.bool.rawValue
		case "Int", "Int32", "Int64", "UInt", "UInt32", "UInt64": return VariantTypeMirror.int.rawValue
		case "Float", "Double": return VariantTypeMirror.float.rawValue
		case "String": return VariantTypeMirror.string.rawValue
		default: return VariantTypeMirror.nil.rawValue
		}
	}

	private func firstMatch(of pattern: String, in source: String) -> String? {
		guard let regex = try? NSRegularExpression(pattern: pattern) else { return nil }
		let range = NSRange(source.startIndex..., in: source)
		guard let match = regex.firstMatch(in: source, range: range), match.numberOfRanges > 1 else { return nil }
		return String(source[Range(match.range(at: 1), in: source)!])
	}

	private func allMatches(of pattern: String, in source: String) -> [String] {
		guard let regex = try? NSRegularExpression(pattern: pattern) else { return [] }
		let range = NSRange(source.startIndex..., in: source)
		return regex.matches(in: source, range: range).compactMap { match in
			match.numberOfRanges > 1 ? String(source[Range(match.range(at: 1), in: source)!]) : nil
		}
	}

	private func allMatchesWithGroups(of pattern: String, in source: String) -> [[String]] {
		guard let regex = try? NSRegularExpression(pattern: pattern) else { return [] }
		let range = NSRange(source.startIndex..., in: source)
		return regex.matches(in: source, range: range).map { match in
			(1..<match.numberOfRanges).map { group in
				String(source[Range(match.range(at: group), in: source)!])
			}
		}
	}

	private func script(for handle: UnsafeMutableRawPointer?) -> CompiledScript? {
		guard let handle = handle else { return nil }
		lock.lock()
		defer { lock.unlock() }
		return scripts[handle]
	}

	private func instance(for handle: UnsafeMutableRawPointer?) -> ScriptInstanceBox? {
		guard let handle = handle else { return nil }
		lock.lock()
		defer { lock.unlock() }
		return instances[handle]
	}
}
