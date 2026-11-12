import Foundation

// Mirror of the C SwiftGodotHost struct (modules/swift/swift_host.h).
// Field order must match the C struct exactly.
public struct SwiftGodotHostMirror {
	public var variant_get_type: (@convention(c) (UnsafeRawPointer?) -> Int32)?
	public var variant_as_bool: (@convention(c) (UnsafeRawPointer?) -> UInt8)?
	public var variant_as_int: (@convention(c) (UnsafeRawPointer?) -> Int64)?
	public var variant_as_double: (@convention(c) (UnsafeRawPointer?) -> Double)?
	public var variant_as_string: (@convention(c) (UnsafeRawPointer?, UnsafeMutablePointer<CChar>?, Int32) -> Void)?
	public var variant_new: (@convention(c) () -> UnsafeMutableRawPointer?)?
	public var variant_free: (@convention(c) (UnsafeMutableRawPointer?) -> Void)?
	public var variant_set_nil: (@convention(c) (UnsafeMutableRawPointer?) -> Void)?
	public var variant_set_bool: (@convention(c) (UnsafeMutableRawPointer?, UInt8) -> Void)?
	public var variant_set_int: (@convention(c) (UnsafeMutableRawPointer?, Int64) -> Void)?
	public var variant_set_double: (@convention(c) (UnsafeMutableRawPointer?, Double) -> Void)?
	public var variant_set_string: (@convention(c) (UnsafeMutableRawPointer?, UnsafePointer<CChar>?) -> Void)?
	public var variant_copy: (@convention(c) (UnsafeMutableRawPointer?, UnsafeRawPointer?) -> Void)?
	public var variant_set_object: (@convention(c) (UnsafeMutableRawPointer?, UnsafeMutableRawPointer?) -> Void)?
	public var variant_get_object: (@convention(c) (UnsafeRawPointer?) -> UnsafeMutableRawPointer?)?
	public var object_call: (@convention(c) (UnsafeMutableRawPointer?, UnsafePointer<CChar>?, UnsafePointer<UnsafeRawPointer?>?, Int32, UnsafeMutableRawPointer?, UnsafeMutablePointer<GDExtensionCallErrorMirror>?) -> UInt8)?
	public var object_get: (@convention(c) (UnsafeMutableRawPointer?, UnsafePointer<CChar>?, UnsafeMutableRawPointer?) -> UInt8)?
	public var object_set: (@convention(c) (UnsafeMutableRawPointer?, UnsafePointer<CChar>?, UnsafeRawPointer?) -> UInt8)?
	public var node_get_node: (@convention(c) (UnsafeMutableRawPointer?, UnsafePointer<CChar>?) -> UnsafeMutableRawPointer?)?
	public var node_get_parent: (@convention(c) (UnsafeMutableRawPointer?) -> UnsafeMutableRawPointer?)?
	public var print: (@convention(c) (UnsafePointer<CChar>?) -> Void)?
	public var push_error: (@convention(c) (UnsafePointer<CChar>?) -> Void)?
}

public final class Host {
	public static let shared = Host()

	private var host: UnsafePointer<SwiftGodotHostMirror>?

	public func setHost(_ h: UnsafePointer<SwiftGodotHostMirror>) {
		host = h
	}

	public var isAvailable: Bool { host != nil }

	private func requireHost() -> UnsafePointer<SwiftGodotHostMirror> {
		guard let host else { fatalError("SwiftGodotHost not initialized") }
		return host
	}

	public func variantNew() -> UnsafeMutableRawPointer {
		requireHost().pointee.variant_new!()
	}

	public func variantFree(_ ptr: UnsafeMutableRawPointer?) {
		requireHost().pointee.variant_free!(ptr)
	}

	public func variantSetInt(_ ptr: UnsafeMutableRawPointer?, _ v: Int64) {
		requireHost().pointee.variant_set_int!(ptr, v)
	}

	public func variantSetDouble(_ ptr: UnsafeMutableRawPointer?, _ v: Double) {
		requireHost().pointee.variant_set_double!(ptr, v)
	}

	public func variantSetBool(_ ptr: UnsafeMutableRawPointer?, _ v: Bool) {
		requireHost().pointee.variant_set_bool!(ptr, v ? 1 : 0)
	}

	public func variantSetString(_ ptr: UnsafeMutableRawPointer?, _ v: String) {
		v.withCString { requireHost().pointee.variant_set_string!(ptr, $0) }
	}

	public func variantSetObject(_ ptr: UnsafeMutableRawPointer?, _ object: UnsafeMutableRawPointer?) {
		requireHost().pointee.variant_set_object!(ptr, object)
	}

	public func variantCopy(_ dst: UnsafeMutableRawPointer?, _ src: UnsafeRawPointer?) {
		requireHost().pointee.variant_copy!(dst, src)
	}

	public func variantAsInt(_ ptr: UnsafeRawPointer?) -> Int64 {
		requireHost().pointee.variant_as_int!(ptr)
	}

	public func variantAsDouble(_ ptr: UnsafeRawPointer?) -> Double {
		requireHost().pointee.variant_as_double!(ptr)
	}

	public func variantAsBool(_ ptr: UnsafeRawPointer?) -> Bool {
		requireHost().pointee.variant_as_bool!(ptr) != 0
	}

	public func variantAsString(_ ptr: UnsafeRawPointer?) -> String {
		var buffer = [CChar](repeating: 0, count: 4096)
		requireHost().pointee.variant_as_string!(ptr, &buffer, Int32(buffer.count))
		return String(cString: buffer)
	}

	public func variantGetObject(_ ptr: UnsafeRawPointer?) -> UnsafeMutableRawPointer? {
		requireHost().pointee.variant_get_object!(ptr)
	}

	@discardableResult
	public func callMethod(object: UnsafeMutableRawPointer, method: String, args: [VariantBox]) -> VariantBox {
		let ret = VariantBox()
		var err = GDExtensionCallErrorMirror()
		method.withCString { m in
			args.map { $0.borrowedPtr }.withUnsafeBufferPointer { buf in
				_ = requireHost().pointee.object_call!(object, m, buf.baseAddress, Int32(args.count), ret.ptr, &err)
			}
		}
		return ret
	}

	public func getProperty(object: UnsafeMutableRawPointer, name: String) -> VariantBox {
		let ret = VariantBox()
		name.withCString { requireHost().pointee.object_get!(object, $0, ret.ptr) }
		return ret
	}

	public func setProperty(object: UnsafeMutableRawPointer, name: String, value: VariantBox) {
		name.withCString { requireHost().pointee.object_set!(object, $0, value.borrowedPtr) }
	}

	public func getNode(_ node: UnsafeMutableRawPointer, _ path: String) -> UnsafeMutableRawPointer? {
		path.withCString { requireHost().pointee.node_get_node!(node, $0) }
	}

	public func print(_ message: String) {
		message.withCString { requireHost().pointee.print!($0) }
	}
}
