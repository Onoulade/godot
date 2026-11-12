import Foundation

// C entry points loaded by SwiftBridge (modules/swift/swift_bridge.cpp).
// The engine calls these on the main thread, so the @MainActor runtime
// singletons are entered through MainActor.assumeIsolated.

@_cdecl("swiftgodot_runtime_version")
public func swiftgodot_runtime_version() -> UInt32 {
	return 1
}

@_cdecl("swiftgodot_init")
public func swiftgodot_init(_ getProcAddress: @convention(c) (UnsafePointer<CChar>) -> UnsafeMutableRawPointer?) {
	MainActor.assumeIsolated {
		Runtime.shared.initialize(getProcAddress: getProcAddress)
	}
}

@_cdecl("swiftgodot_host_init")
public func swiftgodot_host_init(_ host: UnsafePointer<SwiftGodotHostMirror>) {
	MainActor.assumeIsolated {
		Host.shared.setHost(host)
	}
}

@_cdecl("swiftgodot_shutdown")
public func swiftgodot_shutdown() {
	MainActor.assumeIsolated {
		Runtime.shared.shutdown()
	}
}

@_cdecl("swiftgodot_script_compile")
public func swiftgodot_script_compile(_ path: UnsafePointer<CChar>, _ source: UnsafePointer<CChar>) -> UnsafeMutableRawPointer? {
	MainActor.assumeIsolated {
		Runtime.shared.compileScript(path: String(cString: path), source: String(cString: source))
	}
}

@_cdecl("swiftgodot_script_load_precompiled")
public func swiftgodot_script_load_precompiled(_ libraryPath: UnsafePointer<CChar>, _ path: UnsafePointer<CChar>, _ source: UnsafePointer<CChar>) -> UnsafeMutableRawPointer? {
	MainActor.assumeIsolated {
		Runtime.shared.loadPrecompiled(libraryPath: String(cString: libraryPath), path: String(cString: path), source: String(cString: source))
	}
}

@_cdecl("swiftgodot_script_get_library_path")
public func swiftgodot_script_get_library_path(_ script: UnsafeMutableRawPointer?, _ buffer: UnsafeMutablePointer<CChar>?, _ bufferSize: Int32) {
	MainActor.assumeIsolated {
		Runtime.shared.getScriptLibraryPath(script, buffer: buffer, bufferSize: bufferSize)
	}
}

@_cdecl("swiftgodot_script_free")
public func swiftgodot_script_free(_ script: UnsafeMutableRawPointer?) {
	MainActor.assumeIsolated {
		Runtime.shared.freeScript(script)
	}
}

@_cdecl("swiftgodot_script_is_valid")
public func swiftgodot_script_is_valid(_ script: UnsafeMutableRawPointer?) -> UInt8 {
	MainActor.assumeIsolated {
		Runtime.shared.isScriptValid(script) ? 1 : 0
	}
}

@_cdecl("swiftgodot_script_get_error")
public func swiftgodot_script_get_error(_ script: UnsafeMutableRawPointer?, _ buffer: UnsafeMutablePointer<CChar>?, _ bufferSize: Int32) {
	MainActor.assumeIsolated {
		Runtime.shared.getScriptError(script, buffer: buffer, bufferSize: bufferSize)
	}
}

@_cdecl("swiftgodot_script_get_class_name")
public func swiftgodot_script_get_class_name(_ script: UnsafeMutableRawPointer?) -> UnsafePointer<CChar>? {
	MainActor.assumeIsolated {
		Runtime.shared.getScriptClassName(script)
	}
}

@_cdecl("swiftgodot_script_is_tool")
public func swiftgodot_script_is_tool(_ script: UnsafeMutableRawPointer?) -> UInt8 {
	MainActor.assumeIsolated {
		Runtime.shared.isScriptTool(script) ? 1 : 0
	}
}

@_cdecl("swiftgodot_script_get_method_count")
public func swiftgodot_script_get_method_count(_ script: UnsafeMutableRawPointer?) -> Int32 {
	MainActor.assumeIsolated {
		Runtime.shared.getScriptMethodCount(script)
	}
}

@_cdecl("swiftgodot_script_get_method_name")
public func swiftgodot_script_get_method_name(_ script: UnsafeMutableRawPointer?, _ index: Int32) -> UnsafePointer<CChar>? {
	MainActor.assumeIsolated {
		Runtime.shared.getScriptMethodName(script, index: index)
	}
}

@_cdecl("swiftgodot_script_has_method")
public func swiftgodot_script_has_method(_ script: UnsafeMutableRawPointer?, _ method: UnsafePointer<CChar>) -> UInt8 {
	MainActor.assumeIsolated {
		Runtime.shared.scriptHasMethod(script, method: String(cString: method)) ? 1 : 0
	}
}

@_cdecl("swiftgodot_script_get_signal_count")
public func swiftgodot_script_get_signal_count(_ script: UnsafeMutableRawPointer?) -> Int32 {
	MainActor.assumeIsolated {
		Runtime.shared.getScriptSignalCount(script)
	}
}

@_cdecl("swiftgodot_script_get_signal_name")
public func swiftgodot_script_get_signal_name(_ script: UnsafeMutableRawPointer?, _ index: Int32) -> UnsafePointer<CChar>? {
	MainActor.assumeIsolated {
		Runtime.shared.getScriptSignalName(script, index: index)
	}
}

@_cdecl("swiftgodot_script_has_signal")
public func swiftgodot_script_has_signal(_ script: UnsafeMutableRawPointer?, _ signal: UnsafePointer<CChar>) -> UInt8 {
	MainActor.assumeIsolated {
		Runtime.shared.scriptHasSignal(script, signal: String(cString: signal)) ? 1 : 0
	}
}

@_cdecl("swiftgodot_script_get_property_count")
public func swiftgodot_script_get_property_count(_ script: UnsafeMutableRawPointer?) -> Int32 {
	MainActor.assumeIsolated {
		Runtime.shared.getScriptPropertyCount(script)
	}
}

@_cdecl("swiftgodot_script_get_property_name")
public func swiftgodot_script_get_property_name(_ script: UnsafeMutableRawPointer?, _ index: Int32) -> UnsafePointer<CChar>? {
	MainActor.assumeIsolated {
		Runtime.shared.getScriptPropertyName(script, index: index)
	}
}

@_cdecl("swiftgodot_script_get_property_type")
public func swiftgodot_script_get_property_type(_ script: UnsafeMutableRawPointer?, _ index: Int32) -> Int32 {
	MainActor.assumeIsolated {
		Runtime.shared.getScriptPropertyType(script, index: index)
	}
}

@_cdecl("swiftgodot_script_get_property_usage")
public func swiftgodot_script_get_property_usage(_ script: UnsafeMutableRawPointer?, _ index: Int32) -> Int32 {
	MainActor.assumeIsolated {
		Runtime.shared.getScriptPropertyUsage(script, index: index)
	}
}

@_cdecl("swiftgodot_instance_create")
public func swiftgodot_instance_create(_ script: UnsafeMutableRawPointer?, _ owner: UnsafeMutableRawPointer?) -> UnsafeMutableRawPointer? {
	MainActor.assumeIsolated {
		Runtime.shared.createInstance(script: script, owner: owner)
	}
}

@_cdecl("swiftgodot_instance_free")
public func swiftgodot_instance_free(_ instance: UnsafeMutableRawPointer?) {
	MainActor.assumeIsolated {
		Runtime.shared.freeInstance(instance)
	}
}

@_cdecl("swiftgodot_instance_call")
public func swiftgodot_instance_call(
	_ instance: UnsafeMutableRawPointer?,
	_ method: UnsafePointer<CChar>,
	_ args: UnsafePointer<UnsafeRawPointer?>?,
	_ argCount: Int32,
	_ ret: UnsafeMutableRawPointer?,
	_ error: UnsafeMutablePointer<GDExtensionCallErrorMirror>?
) -> UInt8 {
	MainActor.assumeIsolated {
		Runtime.shared.instanceCall(instance, method: String(cString: method), args: args, argCount: argCount, ret: ret, error: error)
	}
}

@_cdecl("swiftgodot_instance_notification")
public func swiftgodot_instance_notification(_ instance: UnsafeMutableRawPointer?, _ notification: Int32, _ reversed: UInt8) {
	MainActor.assumeIsolated {
		Runtime.shared.instanceNotification(instance, notification: notification, reversed: reversed != 0)
	}
}

@_cdecl("swiftgodot_instance_set")
public func swiftgodot_instance_set(_ instance: UnsafeMutableRawPointer?, _ name: UnsafePointer<CChar>, _ value: UnsafeRawPointer?) -> UInt8 {
	MainActor.assumeIsolated {
		Runtime.shared.instanceSet(instance, name: String(cString: name), value: value) ? 1 : 0
	}
}

@_cdecl("swiftgodot_instance_get")
public func swiftgodot_instance_get(_ instance: UnsafeMutableRawPointer?, _ name: UnsafePointer<CChar>, _ ret: UnsafeMutableRawPointer?) -> UInt8 {
	MainActor.assumeIsolated {
		Runtime.shared.instanceGet(instance, name: String(cString: name), ret: ret) ? 1 : 0
	}
}
