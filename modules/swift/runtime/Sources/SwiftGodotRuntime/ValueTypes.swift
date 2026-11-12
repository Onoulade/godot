import Foundation

// Typed Swift wrappers for Godot builtin value types, marshaled through the
// host vtable (see swift_host.h). The generated bindings use these types.

@MainActor public protocol GodotValueType {
	init?(box: VariantBox)
	func toVariantBox() -> VariantBox
}

public struct Vector2: GodotValueType, Equatable, Sendable {
	public var x: Double
	public var y: Double

	public init(_ x: Double, _ y: Double) {
		self.x = x
		self.y = y
	}

	@MainActor public init?(box: VariantBox) {
		let v = Host.shared.variantAsVector2(box.ptr)
		self.init(v.0, v.1)
	}

	@MainActor public func toVariantBox() -> VariantBox {
		let b = VariantBox()
		Host.shared.variantSetVector2(b.ptr, x, y)
		return b
	}
}

public struct Vector3: GodotValueType, Equatable, Sendable {
	public var x: Double
	public var y: Double
	public var z: Double

	public init(_ x: Double, _ y: Double, _ z: Double) {
		self.x = x
		self.y = y
		self.z = z
	}

	@MainActor public init?(box: VariantBox) {
		let v = Host.shared.variantAsVector3(box.ptr)
		self.init(v.0, v.1, v.2)
	}

	@MainActor public func toVariantBox() -> VariantBox {
		let b = VariantBox()
		Host.shared.variantSetVector3(b.ptr, x, y, z)
		return b
	}
}

public struct Vector2i: GodotValueType, Equatable, Sendable {
	public var x: Int
	public var y: Int

	public init(_ x: Int, _ y: Int) {
		self.x = x
		self.y = y
	}

	@MainActor public init?(box: VariantBox) {
		let v = Host.shared.variantAsVector2i(box.ptr)
		self.init(v.0, v.1)
	}

	@MainActor public func toVariantBox() -> VariantBox {
		let b = VariantBox()
		Host.shared.variantSetVector2i(b.ptr, x, y)
		return b
	}
}

public struct Vector3i: GodotValueType, Equatable, Sendable {
	public var x: Int
	public var y: Int
	public var z: Int

	public init(_ x: Int, _ y: Int, _ z: Int) {
		self.x = x
		self.y = y
		self.z = z
	}

	@MainActor public init?(box: VariantBox) {
		let v = Host.shared.variantAsVector3i(box.ptr)
		self.init(v.0, v.1, v.2)
	}

	@MainActor public func toVariantBox() -> VariantBox {
		let b = VariantBox()
		Host.shared.variantSetVector3i(b.ptr, x, y, z)
		return b
	}
}

public struct Vector4: GodotValueType, Sendable {
	public var x: Double
	public var y: Double
	public var z: Double
	public var w: Double

	public init(_ x: Double, _ y: Double, _ z: Double, _ w: Double) {
		self.x = x
		self.y = y
		self.z = z
		self.w = w
	}

	@MainActor public init?(box: VariantBox) {
		let s = Host.shared.variantAsString(box.ptr)
		let parts = s.replacingOccurrences(of: "(", with: "").replacingOccurrences(of: ")", with: "").split(separator: ",").compactMap { Double($0) }
		guard parts.count == 4 else { return nil }
		self.init(parts[0], parts[1], parts[2], parts[3])
	}

	@MainActor public func toVariantBox() -> VariantBox {
		VariantBox("(\(x), \(y), \(z), \(w))")
	}
}

public struct Vector4i: GodotValueType, Sendable {
	public var x: Int
	public var y: Int
	public var z: Int
	public var w: Int

	public init(_ x: Int, _ y: Int, _ z: Int, _ w: Int) {
		self.x = x
		self.y = y
		self.z = z
		self.w = w
	}

	@MainActor public init?(box: VariantBox) {
		let s = Host.shared.variantAsString(box.ptr)
		let parts = s.replacingOccurrences(of: "(", with: "").replacingOccurrences(of: ")", with: "").split(separator: ",").compactMap { Int($0) }
		guard parts.count == 4 else { return nil }
		self.init(parts[0], parts[1], parts[2], parts[3])
	}

	@MainActor public func toVariantBox() -> VariantBox {
		VariantBox("(\(x), \(y), \(z), \(w))")
	}
}

public struct Color: GodotValueType, Equatable, Sendable {
	public var r: Float
	public var g: Float
	public var b: Float
	public var a: Float

	public init(_ r: Float, _ g: Float, _ b: Float, _ a: Float = 1) {
		self.r = r
		self.g = g
		self.b = b
		self.a = a
	}

	@MainActor public init?(box: VariantBox) {
		let v = Host.shared.variantAsColor(box.ptr)
		self.init(v.0, v.1, v.2, v.3)
	}

	@MainActor public func toVariantBox() -> VariantBox {
		let b = VariantBox()
		Host.shared.variantSetColor(b.ptr, r, g, b, a)
		return b
	}
}

public struct Quaternion: GodotValueType, Equatable, Sendable {
	public var x: Float
	public var y: Float
	public var z: Float
	public var w: Float

	public init(_ x: Float, _ y: Float, _ z: Float, _ w: Float) {
		self.x = x
		self.y = y
		self.z = z
		self.w = w
	}

	@MainActor public init?(box: VariantBox) {
		let v = Host.shared.variantAsQuaternion(box.ptr)
		self.init(v.0, v.1, v.2, v.3)
	}

	@MainActor public func toVariantBox() -> VariantBox {
		let b = VariantBox()
		Host.shared.variantSetQuaternion(b.ptr, x, y, z, w)
		return b
	}
}

public struct Rect2: GodotValueType, Sendable {
	public var position: Vector2
	public var size: Vector2

	public init(_ position: Vector2, _ size: Vector2) {
		self.position = position
		self.size = size
	}

	@MainActor public init?(box: VariantBox) {
		let v = Host.shared.variantAsRect2(box.ptr)
		self.init(Vector2(Double(v.0), Double(v.1)), Vector2(Double(v.2), Double(v.3)))
	}

	@MainActor public func toVariantBox() -> VariantBox {
		let b = VariantBox()
		Host.shared.variantSetRect2(b.ptr, Float(position.x), Float(position.y), Float(size.x), Float(size.y))
		return b
	}
}

public struct Rect2i: GodotValueType, Sendable {
	public var position: Vector2i
	public var size: Vector2i

	public init(_ position: Vector2i, _ size: Vector2i) {
		self.position = position
		self.size = size
	}

	@MainActor public init?(box: VariantBox) {
		let s = Host.shared.variantAsString(box.ptr)
		let nums = s.replacingOccurrences(of: "(", with: "").replacingOccurrences(of: ")", with: "").split(separator: ",").compactMap { Int($0) }
		guard nums.count == 4 else { return nil }
		self.init(Vector2i(nums[0], nums[1]), Vector2i(nums[2], nums[3]))
	}

	@MainActor public func toVariantBox() -> VariantBox {
		VariantBox("(\(position.x), \(position.y), \(size.x), \(size.y))")
	}
}

public struct AABB: GodotValueType, Sendable {
	public var position: Vector3
	public var size: Vector3

	public init(_ position: Vector3, _ size: Vector3) {
		self.position = position
		self.size = size
	}

	@MainActor public init?(box: VariantBox) {
		let v = Host.shared.variantAsAABB(box.ptr)
		guard v.count == 6 else { return nil }
		self.init(Vector3(Double(v[0]), Double(v[1]), Double(v[2])), Vector3(Double(v[3]), Double(v[4]), Double(v[5])))
	}

	@MainActor public func toVariantBox() -> VariantBox {
		let b = VariantBox()
		Host.shared.variantSetAABB(b.ptr, [Float(position.x), Float(position.y), Float(position.z), Float(size.x), Float(size.y), Float(size.z)])
		return b
	}
}

public struct Plane: GodotValueType, Sendable {
	public var normal: Vector3
	public var d: Float

	public init(_ normal: Vector3, _ d: Float) {
		self.normal = normal
		self.d = d
	}

	@MainActor public init?(box: VariantBox) {
		let v = Host.shared.variantAsPlane(box.ptr)
		self.init(Vector3(Double(v.0), Double(v.1), Double(v.2)), v.3)
	}

	@MainActor public func toVariantBox() -> VariantBox {
		let b = VariantBox()
		Host.shared.variantSetPlane(b.ptr, Float(normal.x), Float(normal.y), Float(normal.z), d)
		return b
	}
}

public struct Transform2D: GodotValueType, Sendable {
	public var x: Vector2
	public var y: Vector2
	public var origin: Vector2

	public init(_ x: Vector2, _ y: Vector2, _ origin: Vector2) {
		self.x = x
		self.y = y
		self.origin = origin
	}

	@MainActor public init?(box: VariantBox) {
		let v = Host.shared.variantAsTransform2D(box.ptr)
		guard v.count == 6 else { return nil }
		self.init(Vector2(Double(v[0]), Double(v[1])), Vector2(Double(v[2]), Double(v[3])), Vector2(Double(v[4]), Double(v[5])))
	}

	@MainActor public func toVariantBox() -> VariantBox {
		let b = VariantBox()
		Host.shared.variantSetTransform2D(b.ptr, [Float(x.x), Float(x.y), Float(y.x), Float(y.y), Float(origin.x), Float(origin.y)])
		return b
	}
}

public struct Transform3D: GodotValueType, Sendable {
	public var basis: Basis
	public var origin: Vector3

	public init(_ basis: Basis, _ origin: Vector3) {
		self.basis = basis
		self.origin = origin
	}

	@MainActor public init?(box: VariantBox) {
		let v = Host.shared.variantAsTransform3D(box.ptr)
		guard v.count == 12 else { return nil }
		var rows: [Vector3] = []
		for i in 0..<3 {
			rows.append(Vector3(Double(v[i * 3]), Double(v[i * 3 + 1]), Double(v[i * 3 + 2])))
		}
		self.init(Basis(rows), Vector3(Double(v[9]), Double(v[10]), Double(v[11])))
	}

	@MainActor public func toVariantBox() -> VariantBox {
		let b = VariantBox()
		var v: [Float] = []
		for r in basis.rows {
			v.append(contentsOf: [Float(r.x), Float(r.y), Float(r.z)])
		}
		v.append(contentsOf: [Float(origin.x), Float(origin.y), Float(origin.z)])
		Host.shared.variantSetTransform3D(b.ptr, v)
		return b
	}
}

public struct Basis: GodotValueType, Sendable {
	public var rows: [Vector3]

	public init(_ rows: [Vector3]) {
		self.rows = rows
	}

	@MainActor public init?(box: VariantBox) {
		let v = Host.shared.variantAsBasis(box.ptr)
		guard v.count == 9 else { return nil }
		var rows: [Vector3] = []
		for i in 0..<3 {
			rows.append(Vector3(Double(v[i * 3]), Double(v[i * 3 + 1]), Double(v[i * 3 + 2])))
		}
		self.init(rows)
	}

	@MainActor public func toVariantBox() -> VariantBox {
		let b = VariantBox()
		var v: [Float] = []
		for r in rows {
			v.append(contentsOf: [Float(r.x), Float(r.y), Float(r.z)])
		}
		Host.shared.variantSetBasis(b.ptr, v)
		return b
	}
}

public struct Projection: GodotValueType, Sendable {
	public var columns: [Vector4]

	public init(_ columns: [Vector4]) {
		self.columns = columns
	}

	@MainActor public init?(box: VariantBox) {
		let v = Host.shared.variantAsProjection(box.ptr)
		guard v.count == 16 else { return nil }
		var cols: [Vector4] = []
		for i in 0..<4 {
			cols.append(Vector4(Double(v[i * 4]), Double(v[i * 4 + 1]), Double(v[i * 4 + 2]), Double(v[i * 4 + 3])))
		}
		self.init(cols)
	}

	@MainActor public func toVariantBox() -> VariantBox {
		let b = VariantBox()
		var v: [Float] = []
		for c in columns {
			v.append(contentsOf: [Float(c.x), Float(c.y), Float(c.z), Float(c.w)])
		}
		Host.shared.variantSetProjection(b.ptr, v)
		return b
	}
}

extension VariantBox {
	@MainActor public convenience init<T: GodotValueType>(_ value: T) {
		self.init()
		Host.shared.variantCopy(ptr, value.toVariantBox().ptr)
	}

	public func asVector2() -> Vector2 { Vector2(box: self)! }
	public func asVector3() -> Vector3 { Vector3(box: self)! }
	public func asVector2i() -> Vector2i { Vector2i(box: self)! }
	public func asVector3i() -> Vector3i { Vector3i(box: self)! }
	public func asVector4() -> Vector4 { Vector4(box: self)! }
	public func asVector4i() -> Vector4i { Vector4i(box: self)! }
	public func asColor() -> Color { Color(box: self)! }
	public func asQuaternion() -> Quaternion { Quaternion(box: self)! }
	public func asRect2() -> Rect2 { Rect2(box: self)! }
	public func asRect2i() -> Rect2i { Rect2i(box: self)! }
	public func asAABB() -> AABB { AABB(box: self)! }
	public func asPlane() -> Plane { Plane(box: self)! }
	public func asTransform2D() -> Transform2D { Transform2D(box: self)! }
	public func asTransform3D() -> Transform3D { Transform3D(box: self)! }
	public func asBasis() -> Basis { Basis(box: self)! }
	public func asProjection() -> Projection { Projection(box: self)! }
}
