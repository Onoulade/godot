import Foundation

// C entry points loaded by SwiftBridge (modules/swift/swift_bridge.cpp).
// The engine calls these on the main thread, so the @MainActor runtime
// singletons are entered through MainActor.assumeIsolated.
// Note: @_cdecl requires C-representable types and Sendable returns (raw
// pointers are not Sendable in Swift 6), so pointer parameters are wrapped in
// SendableRawPointer/SendableConstPointer before crossing into the
// assumeIsolated closure, and pointer returns use UInt (same ABI as a
// pointer).

@_cdecl("swiftgodot_runtime_version")
public func swiftgodot_runtime_version() -> UInt32 {
	return 1
}

@_cdecl("swiftgodot_init")
public func swiftgodot_init(_ getProcAddress: UnsafeMutableRawPointer?) {
	let fn = SendableRawPointer(getProcAddress)
	MainActor.assumeIsolated {
		let typed = unsafeBitCast(fn.ptr, to: Runtime.GetProcAddress.self)
		Runtime.shared.initialize(getProcAddress: typed)
	}
}

@_cdecl("swiftgodot_host_init")
public func swiftgodot_host_init(_ host: UnsafeRawPointer?) {
	let h = SendableConstPointer(host)
	MainActor.assumeIsolated {
		Host.shared.setHost(h.ptr?.assumingMemoryBound(to: SwiftGodotHostMirror.self))
	}
}

@_cdecl("swiftgodot_shutdown")
public func swiftgodot_shutdown() {
	MainActor.assumeIsolated {
		Runtime.shared.shutdown()
	}
}

@_cdecl("swiftgodot_script_compile")
public func swiftgodot_script_compile(_ path: UnsafePointer<CChar>, _ source: UnsafePointer<CChar>) -> UInt {
	let p = String(cString: path)
	let s = String(cString: source)
	MainActor.assumeIsolated {
		Runtime.shared.compileScript(path: p, source: s).map { UInt(bitPattern: $0) } ?? 0
	}
}

@_cdecl("swiftgodot_script_load_precompiled")
public func swiftgodot_script_load_precompiled(_ libraryPath: UnsafePointer<CChar>, _ path: UnsafePointer<CChar>, _ source: UnsafePointer<CChar>) -> UInt {
	let lib = String(cString: libraryPath)
	let p = String(cString: path)
	let s = String(cString: source)
	MainActor.assumeIsolated {
		Runtime.shared.loadPrecompiled(libraryPath: lib, path: p, source: s).map { UInt(bitPattern: $0) } ?? 0
	}
}

@_cdecl("swiftgodot_script_get_library_path")
public func swiftgodot_script_get_library_path(_ script: UnsafeMutableRawPointer?, _ buffer: UnsafeMutableRawPointer?, _ bufferSize: Int32) {
	let s = SendableRawPointer(script)
	let b = SendableRawPointer(buffer)
	MainActor.assumeIsolated {
		Runtime.shared.getScriptLibraryPath(s.ptr, buffer: b.ptr?.assumingMemoryBound(to: CChar.self), bufferSize: bufferSize)
	}
}

@_cdecl("swiftgodot_script_free")
public func swiftgodot_script_free(_ script: UnsafeMutableRawPointer?) {
	let s = SendableRawPointer(script)
	MainActor.assumeIsolated {
		Runtime.shared.freeScript(s.ptr)
	}
}

@_cdecl("swiftgodot_script_is_valid")
public func swiftgodot_script_is_valid(_ script: UnsafeMutableRawPointer?) -> UInt8 {
	let s = SendableRawPointer(script)
	MainActor.assumeIsolated {
		Runtime.shared.isScriptValid(s.ptr) ? 1 : 0
	}
}

@_cdecl("swiftgodot_script_get_error")
public func swiftgodot_script_get_error(_ script: UnsafeMutableRawPointer?, _ buffer: UnsafeMutableRawPointer?, _ bufferSize: Int32) {
	let s = SendableRawPointer(script)
	let b = SendableRawPointer(buffer)
	MainActor.assumeIsolated {
		Runtime.shared.getScriptError(s.ptr, buffer: b.ptr?.assumingMemoryBound(to: CChar.self), bufferSize: bufferSize)
	}
}

@_cdecl("swiftgodot_script_get_class_name")
public func swiftgodot_script_get_class_name(_ script: UnsafeMutableRawPointer?) -> UInt {
	let s = SendableRawPointer(script)
	MainActor.assumeIsolated {
		Runtime.shared.getScriptClassName(s.ptr).map { UInt(bitPattern: $0) } ?? 0
	}
}

@_cdecl("swiftgodot_script_is_tool")
public func swiftgodot_script_is_tool(_ script: UnsafeMutableRawPointer?) -> UInt8 {
	let s = SendableRawPointer(script)
	MainActor.assumeIsolated {
		Runtime.shared.isScriptTool(s.ptr) ? 1 : 0
	}
}

@_cdecl("swiftgodot_script_get_method_count")
public func swiftgodot_script_get_method_count(_ script: UnsafeMutableRawPointer?) -> Int32 {
	let s = SendableRawPointer(script)
	MainActor.assumeIsolated {
		Runtime.shared.getScriptMethodCount(s.ptr)
	}
}

@_cdecl("swiftgodot_script_get_method_name")
public func swiftgodot_script_get_method_name(_ script: UnsafeMutableRawPointer?, _ index: Int32) -> UInt {
	let s = SendableRawPointer(script)
	MainActor.assumeIsolated {
		Runtime.shared.getScriptMethodName(s.ptr, index: index).map { UInt(bitPattern: $0) } ?? 0
	}
}

@_cdecl("swiftgodot_script_has_method")
public func swiftgodot_script_has_method(_ script: UnsafeMutableRawPointer?, _ method: UnsafePointer<CChar>) -> UInt8 {
	let s = SendableRawPointer(script)
	let m = String(cString: method)
	MainActor.assumeIsolated {
		Runtime.shared.scriptHasMethod(s.ptr, method: m) ? 1 : 0
	}
}

@_cdecl("swiftgodot_script_get_signal_count")
public func swiftgodot_script_get_signal_count(_ script: UnsafeMutableRawPointer?) -> Int32 {
	let s = SendableRawPointer(script)
	MainActor.assumeIsolated {
		Runtime.shared.getScriptSignalCount(s.ptr)
	}
}

@_cdecl("swiftgodot_script_get_signal_name")
public func swiftgodot_script_get_signal_name(_ script: UnsafeMutableRawPointer?, _ index: Int32) -> UInt {
	let s = SendableRawPointer(script)
	MainActor.assumeIsolated {
		Runtime.shared.getScriptSignalName(s.ptr, index: index).map { UInt(bitPattern: $0) } ?? 0
	}
}

@_cdecl("swiftgodot_script_has_signal")
public func swiftgodot_script_has_signal(_ script: UnsafeMutableRawPointer?, _ signal: UnsafePointer<CChar>) -> UInt8 {
	let s = SendableRawPointer(script)
	let sig = String(cString: signal)
	MainActor.assumeIsolated {
		Runtime.shared.scriptHasSignal(s.ptr, signal: sig) ? 1 : 0
	}
}

@_cdecl("swiftgodot_script_get_property_count")
public func swiftgodot_script_get_property_count(_ script: UnsafeMutableRawPointer?) -> Int32 {
	let s = SendableRawPointer(script)
	MainActor.assumeIsolated {
		Runtime.shared.getScriptPropertyCount(s.ptr)
	}
}

@_cdecl("swiftgodot_script_get_property_name")
public func swiftgodot_script_get_property_name(_ script: UnsafeMutableRawPointer?, _ index: Int32) -> UInt {
	let s = SendableRawPointer(script)
	MainActor.assumeIsolated {
		Runtime.shared.getScriptPropertyName(s.ptr, index: index).map { UInt(bitPattern: $0) } ?? 0
	}
}

@_cdecl("swiftgodot_script_get_property_type")
public func swiftgodot_script_get_property_type(_ script: UnsafeMutableRawPointer?, _ index: Int32) -> Int32 {
	let s = SendableRawPointer(script)
	MainActor.assumeIsolated {
		Runtime.shared.getScriptPropertyType(s.ptr, index: index)
	}
}

@_cdecl("swiftgodot_script_get_property_usage")
public func swiftgodot_script_get_property_usage(_ script: UnsafeMutableRawPointer?, _ index: Int32) -> Int32 {
	let s = SendableRawPointer(script)
	MainActor.assumeIsolated {
		Runtime.shared.getScriptPropertyUsage(s.ptr, index: index)
	}
}

@_cdecl("swiftgodot_instance_create")
public func swiftgodot_instance_create(_ script: UnsafeMutableRawPointer?, _ owner: UnsafeMutableRawPointer?) -> UInt {
	let s = SendableRawPointer(script)
	let o = SendableRawPointer(owner)
	MainActor.assumeIsolated {
		Runtime.shared.createInstance(script: s.ptr, owner: o.ptr).map { UInt(bitPattern: $0) } ?? 0
	}
}

@_cdecl("swiftgodot_instance_free")
public func swiftgodot_instance_free(_ instance: UnsafeMutableRawPointer?) {
	let i = SendableRawPointer(instance)
	MainActor.assumeIsolated {
		Runtime.shared.freeInstance(i.ptr)
	}
}

@_cdecl("swiftgodot_instance_call")
public func swiftgodot_instance_call(
	_ instance: UnsafeMutableRawPointer?,
	_ method: UnsafePointer<CChar>,
	_ args: UnsafeMutableRawPointer?,
	_ argCount: Int32,
	_ ret: UnsafeMutableRawPointer?,
	_ error: UnsafeMutableRawPointer?
) -> UInt8 {
	let i = SendableRawPointer(instance)
	let a = SendableRawPointer(args)
	let r = SendableRawPointer(ret)
	let e = SendableRawPointer(error)
	let m = String(cString: method)
	MainActor.assumeIsolated {
		Runtime.shared.instanceCall(i.ptr, method: m, args: a.ptr, argCount: argCount, ret: r.ptr, error: e.ptr)
	}
}

@_cdecl("swiftgodot_instance_notification")
public func swiftgodot_instance_notification(_ instance: UnsafeMutableRawPointer?, _ notification: Int32, _ reversed: UInt8) {
	let i = SendableRawPointer(instance)
	MainActor.assumeIsolated {
		Runtime.shared.instanceNotification(i.ptr, notification: notification, reversed: reversed != 0)
	}
}

@_cdecl("swiftgodot_instance_set")
public func swiftgodot_instance_set(_ instance: UnsafeMutableRawPointer?, _ name: UnsafePointer<CChar>, _ value: UnsafeRawPointer?) -> UInt8 {
	let i = SendableRawPointer(instance)
	let v = SendableConstPointer(value)
	let n = String(cString: name)
	MainActor.assumeIsolated {
		Runtime.shared.instanceSet(i.ptr, name: n, value: v.ptr) ? 1 : 0
	}
}

@_cdecl("swiftgodot_instance_get")
public func swiftgodot_instance_get(_ instance: UnsafeMutableRawPointer?, _ name: UnsafePointer<CChar>, _ ret: UnsafeMutableRawPointer?) -> UInt8 {
	let i = SendableRawPointer(instance)
	let r = SendableRawPointer(ret)
	let n = String(cString: name)
	MainActor.assumeIsolated {
		Runtime.shared.instanceGet(i.ptr, name: n, ret: r.ptr) ? 1 : 0
	}
}
