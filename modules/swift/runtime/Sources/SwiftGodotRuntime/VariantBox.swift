import Foundation

// Owns (or borrows) a heap Variant allocated through the host vtable, with
// typed conversions used by the generated bindings.
@MainActor
public class VariantBox {
	public let ptr: UnsafeMutableRawPointer
	private let owned: Bool

	public init() {
		owned = true
		ptr = Host.shared.variantNew()
	}

	public init(borrowed: UnsafeMutableRawPointer) {
		owned = false
		ptr = borrowed
	}

	deinit {
		if owned {
			Host.shared.variantFree(ptr)
		}
	}

	public init(_ value: Int) {
		self.init()
		Host.shared.variantSetInt(ptr, Int64(value))
	}

	public init(_ value: Double) {
		self.init()
		Host.shared.variantSetDouble(ptr, value)
	}

	public init(_ value: Bool) {
		self.init()
		Host.shared.variantSetBool(ptr, value)
	}

	public init(_ value: String) {
		self.init()
		Host.shared.variantSetString(ptr, value)
	}

	public init(_ box: VariantBox) {
		self.init()
		Host.shared.variantCopy(ptr, box.ptr)
	}

	public init(_ object: GodotObject) {
		self.init()
		Host.shared.variantSetObject(ptr, object.ptr)
	}

	public var borrowedPtr: UnsafeRawPointer { ptr }

	public func asInt() -> Int {
		Int(Host.shared.variantAsInt(ptr))
	}

	public func asDouble() -> Double {
		Host.shared.variantAsDouble(ptr)
	}

	public func asBool() -> Bool {
		Host.shared.variantAsBool(ptr)
	}

	public func asString() -> String {
		Host.shared.variantAsString(ptr)
	}

	public func asObject<T: GodotObject>(_ type: T.Type) -> T? {
		guard let object = Host.shared.variantGetObject(ptr) else { return nil }
		return T(ptr: object)
	}
}
