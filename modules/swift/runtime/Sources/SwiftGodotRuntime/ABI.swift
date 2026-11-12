import Foundation

// C ABI mirrored from modules/swift/swift_bridge.h (SwiftGodotABI).
// Keep both sides in sync when changing this file.

public typealias SwiftGodotScriptHandle = UnsafeMutableRawPointer
public typealias SwiftGodotInstanceHandle = UnsafeMutableRawPointer

public struct GDExtensionCallErrorMirror {
	public var error: Int32
	public var argument: Int32
	public var expected: Int32

	public init() {
		error = 0
		argument = 0
		expected = 0
	}
}

// Raw pointers only: @convention(c) requires C-representable types
// (UnsafePointer<Optional<...>> is not representable).
public typealias SwiftGodotMethodFn = @convention(c) (
	UnsafeMutableRawPointer?,
	UnsafeMutableRawPointer?,
	Int32,
	UnsafeMutableRawPointer?,
	UnsafeMutableRawPointer?
) -> UInt8

public struct SwiftGodotScriptDescriptor {
	public var class_name: UnsafePointer<CChar>?
	public var is_tool: UInt8
	public var new_instance: (@convention(c) () -> UnsafeRawPointer?)?
	public var method_count: Int32
	public var method_names: UnsafeMutablePointer<UnsafePointer<CChar>?>?
	public var method_fns: UnsafeMutablePointer<SwiftGodotMethodFn?>?

	public init() {
		class_name = nil
		is_tool = 0
		new_instance = nil
		method_count = 0
		method_names = nil
		method_fns = nil
	}
}
