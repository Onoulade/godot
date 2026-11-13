import Foundation

// Typed Swift wrappers over Godot Array / Dictionary variants, marshaled
// through dedicated host vtable entries. Elements cross the boundary as
// VariantBox; VariantBoxConvertible gives typed access for scalars, value
// types, engine objects and whole collections.

public protocol VariantBoxConvertible {
	@MainActor func toVariantBox() -> VariantBox
	@MainActor static func fromVariantBox(_ box: VariantBox) -> Self
}

extension Int: VariantBoxConvertible {
	public func toVariantBox() -> VariantBox { VariantBox(self) }
	public static func fromVariantBox(_ box: VariantBox) -> Int { box.asInt() }
}

extension Double: VariantBoxConvertible {
	public func toVariantBox() -> VariantBox { VariantBox(self) }
	public static func fromVariantBox(_ box: VariantBox) -> Double { box.asDouble() }
}

extension Bool: VariantBoxConvertible {
	public func toVariantBox() -> VariantBox { VariantBox(self) }
	public static func fromVariantBox(_ box: VariantBox) -> Bool { box.asBool() }
}

extension String: VariantBoxConvertible {
	public func toVariantBox() -> VariantBox { VariantBox(self) }
	public static func fromVariantBox(_ box: VariantBox) -> String { box.asString() }
}

extension GodotObject: VariantBoxConvertible {
	public func toVariantBox() -> VariantBox { VariantBox(self) }
	public static func fromVariantBox(_ box: VariantBox) -> Self {
		guard let object = box.asObject(Self.self) else {
			fatalError("VariantBox does not hold a \(String(describing: Self.self)) object")
		}
		return object
	}
}

// Value types already declare toVariantBox()/init?(box:) in GodotValueType
// (which inherits VariantBoxConvertible); provide the static conversion here.
extension GodotValueType {
	public static func fromVariantBox(_ box: VariantBox) -> Self {
		guard let value = Self(box: box) else {
			fatalError("VariantBox does not hold the expected Godot value type")
		}
		return value
	}
}

// A Godot Array held in a Variant. `borrowed` instances (from VariantBox.asArray)
// alias the caller's variant and are not freed.
@MainActor
public final class GodotArray: VariantBoxConvertible {
	public let ptr: UnsafeMutableRawPointer
	private let owned: Bool

	public init() {
		owned = true
		ptr = Host.shared.variantNewArray()
	}

	public init(borrowed: UnsafeMutableRawPointer) {
		owned = false
		ptr = borrowed
	}

	deinit {
		MainActor.assumeIsolated {
			if owned {
				Host.shared.variantFree(ptr)
			}
		}
	}

	public var count: Int {
		Host.shared.variantArraySize(ptr)
	}

	public func append(_ value: VariantBox) {
		Host.shared.variantArrayAppend(ptr, value.borrowedPtr)
	}

	public func append<T: VariantBoxConvertible>(_ value: T) {
		append(value.toVariantBox())
	}

	public subscript(index: Int) -> VariantBox {
		get {
			if let raw = Host.shared.variantArrayGet(ptr, index) {
				return VariantBox(borrowed: UnsafeMutableRawPointer(mutating: raw))
			}
			return VariantBox()
		}
		set { Host.shared.variantArraySet(ptr, index, newValue.borrowedPtr) }
	}

	public func get<T: VariantBoxConvertible>(_ index: Int) -> T {
		T.fromVariantBox(self[index])
	}

	public func toVariantBox() -> VariantBox {
		let box = VariantBox()
		Host.shared.variantCopy(box.ptr, ptr)
		return box
	}

	public static func fromVariantBox(_ box: VariantBox) -> GodotArray {
		GodotArray(borrowed: box.ptr)
	}
}

// A Godot Dictionary held in a Variant.
@MainActor
public final class GodotDictionary: VariantBoxConvertible {
	public let ptr: UnsafeMutableRawPointer
	private let owned: Bool

	public init() {
		owned = true
		ptr = Host.shared.variantNewDictionary()
	}

	public init(borrowed: UnsafeMutableRawPointer) {
		owned = false
		ptr = borrowed
	}

	deinit {
		MainActor.assumeIsolated {
			if owned {
				Host.shared.variantFree(ptr)
			}
		}
	}

	public var count: Int {
		Host.shared.variantDictionarySize(ptr)
	}

	public func has<K: VariantBoxConvertible>(_ key: K) -> Bool {
		Host.shared.variantDictionaryHas(ptr, key.toVariantBox().borrowedPtr)
	}

	public func set<K: VariantBoxConvertible, V: VariantBoxConvertible>(_ key: K, _ value: V) {
		Host.shared.variantDictionarySet(ptr, key.toVariantBox().borrowedPtr, value.toVariantBox().borrowedPtr)
	}

	public func get<V: VariantBoxConvertible, K: VariantBoxConvertible>(_ key: K) -> V? {
		guard let raw = Host.shared.variantDictionaryGet(ptr, key.toVariantBox().borrowedPtr) else { return nil }
		return V.fromVariantBox(VariantBox(borrowed: UnsafeMutableRawPointer(mutating: raw)))
	}

	public func toVariantBox() -> VariantBox {
		let box = VariantBox()
		Host.shared.variantCopy(box.ptr, ptr)
		return box
	}

	public static func fromVariantBox(_ box: VariantBox) -> GodotDictionary {
		GodotDictionary(borrowed: box.ptr)
	}
}

extension VariantBox {
	// Borrowed view of this variant's Array (the variant must outlive it).
	public func asArray() -> GodotArray {
		GodotArray(borrowed: ptr)
	}

	// Borrowed view of this variant's Dictionary.
	public func asDictionary() -> GodotDictionary {
		GodotDictionary(borrowed: ptr)
	}
}
