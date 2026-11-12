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
	public var variant_as_vector2: (@convention(c) (UnsafeRawPointer?, UnsafeMutablePointer<Double>?) -> Void)?
	public var variant_set_vector2: (@convention(c) (UnsafeMutableRawPointer?, Double, Double) -> Void)?
	public var variant_as_vector3: (@convention(c) (UnsafeRawPointer?, UnsafeMutablePointer<Double>?) -> Void)?
	public var variant_set_vector3: (@convention(c) (UnsafeMutableRawPointer?, Double, Double, Double) -> Void)?
	public var variant_as_vector2i: (@convention(c) (UnsafeRawPointer?, UnsafeMutablePointer<Int64>?) -> Void)?
	public var variant_set_vector2i: (@convention(c) (UnsafeMutableRawPointer?, Int64, Int64) -> Void)?
	public var variant_as_vector3i: (@convention(c) (UnsafeRawPointer?, UnsafeMutablePointer<Int64>?) -> Void)?
	public var variant_set_vector3i: (@convention(c) (UnsafeMutableRawPointer?, Int64, Int64, Int64) -> Void)?
	public var variant_as_color: (@convention(c) (UnsafeRawPointer?, UnsafeMutablePointer<Float>?) -> Void)?
	public var variant_set_color: (@convention(c) (UnsafeMutableRawPointer?, Float, Float, Float, Float) -> Void)?
	public var variant_as_quaternion: (@convention(c) (UnsafeRawPointer?, UnsafeMutablePointer<Float>?) -> Void)?
	public var variant_set_quaternion: (@convention(c) (UnsafeMutableRawPointer?, Float, Float, Float, Float) -> Void)?
	public var variant_as_rect2: (@convention(c) (UnsafeRawPointer?, UnsafeMutablePointer<Float>?) -> Void)?
	public var variant_set_rect2: (@convention(c) (UnsafeMutableRawPointer?, Float, Float, Float, Float) -> Void)?
	public var variant_as_aabb: (@convention(c) (UnsafeRawPointer?, UnsafeMutablePointer<Float>?) -> Void)?
	public var variant_set_aabb: (@convention(c) (UnsafeMutableRawPointer?, UnsafePointer<Float>?) -> Void)?
	public var variant_as_plane: (@convention(c) (UnsafeRawPointer?, UnsafeMutablePointer<Float>?) -> Void)?
	public var variant_set_plane: (@convention(c) (UnsafeMutableRawPointer?, Float, Float, Float, Float) -> Void)?
	public var variant_as_transform2d: (@convention(c) (UnsafeRawPointer?, UnsafeMutablePointer<Float>?) -> Void)?
	public var variant_set_transform2d: (@convention(c) (UnsafeMutableRawPointer?, UnsafePointer<Float>?) -> Void)?
	public var variant_as_transform3d: (@convention(c) (UnsafeRawPointer?, UnsafeMutablePointer<Float>?) -> Void)?
	public var variant_set_transform3d: (@convention(c) (UnsafeMutableRawPointer?, UnsafePointer<Float>?) -> Void)?
	public var variant_as_basis: (@convention(c) (UnsafeRawPointer?, UnsafeMutablePointer<Float>?) -> Void)?
	public var variant_set_basis: (@convention(c) (UnsafeMutableRawPointer?, UnsafePointer<Float>?) -> Void)?
	public var variant_as_projection: (@convention(c) (UnsafeRawPointer?, UnsafeMutablePointer<Float>?) -> Void)?
	public var variant_set_projection: (@convention(c) (UnsafeMutableRawPointer?, UnsafePointer<Float>?) -> Void)?
	public var object_call: (@convention(c) (UnsafeMutableRawPointer?, UnsafePointer<CChar>?, UnsafePointer<UnsafeRawPointer?>?, Int32, UnsafeMutableRawPointer?, UnsafeMutablePointer<GDExtensionCallErrorMirror>?) -> UInt8)?
	public var object_get: (@convention(c) (UnsafeMutableRawPointer?, UnsafePointer<CChar>?, UnsafeMutableRawPointer?) -> UInt8)?
	public var object_set: (@convention(c) (UnsafeMutableRawPointer?, UnsafePointer<CChar>?, UnsafeRawPointer?) -> UInt8)?
	public var node_get_node: (@convention(c) (UnsafeMutableRawPointer?, UnsafePointer<CChar>?) -> UnsafeMutableRawPointer?)?
	public var node_get_parent: (@convention(c) (UnsafeMutableRawPointer?) -> UnsafeMutableRawPointer?)?
	public var print: (@convention(c) (UnsafePointer<CChar>?) -> Void)?
	public var push_error: (@convention(c) (UnsafePointer<CChar>?) -> Void)?
	public var object_create: (@convention(c) (UnsafePointer<CChar>?) -> UnsafeMutableRawPointer?)?
}

@MainActor
public final class Host {
	@MainActor public static let shared = Host()

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

	public func variantAsVector2(_ ptr: UnsafeRawPointer?) -> (Double, Double) {
		var o = [Double](repeating: 0, count: 2)
		requireHost().pointee.variant_as_vector2!(ptr, &o)
		return (o[0], o[1])
	}

	public func variantSetVector2(_ ptr: UnsafeMutableRawPointer?, _ x: Double, _ y: Double) {
		requireHost().pointee.variant_set_vector2!(ptr, x, y)
	}

	public func variantAsVector3(_ ptr: UnsafeRawPointer?) -> (Double, Double, Double) {
		var o = [Double](repeating: 0, count: 3)
		requireHost().pointee.variant_as_vector3!(ptr, &o)
		return (o[0], o[1], o[2])
	}

	public func variantSetVector3(_ ptr: UnsafeMutableRawPointer?, _ x: Double, _ y: Double, _ z: Double) {
		requireHost().pointee.variant_set_vector3!(ptr, x, y, z)
	}

	public func variantAsVector2i(_ ptr: UnsafeRawPointer?) -> (Int, Int) {
		var o = [Int64](repeating: 0, count: 2)
		requireHost().pointee.variant_as_vector2i!(ptr, &o)
		return (Int(o[0]), Int(o[1]))
	}

	public func variantSetVector2i(_ ptr: UnsafeMutableRawPointer?, _ x: Int, _ y: Int) {
		requireHost().pointee.variant_set_vector2i!(ptr, Int64(x), Int64(y))
	}

	public func variantAsVector3i(_ ptr: UnsafeRawPointer?) -> (Int, Int, Int) {
		var o = [Int64](repeating: 0, count: 3)
		requireHost().pointee.variant_as_vector3i!(ptr, &o)
		return (Int(o[0]), Int(o[1]), Int(o[2]))
	}

	public func variantSetVector3i(_ ptr: UnsafeMutableRawPointer?, _ x: Int, _ y: Int, _ z: Int) {
		requireHost().pointee.variant_set_vector3i!(ptr, Int64(x), Int64(y), Int64(z))
	}

	public func variantAsColor(_ ptr: UnsafeRawPointer?) -> (Float, Float, Float, Float) {
		var o = [Float](repeating: 0, count: 4)
		requireHost().pointee.variant_as_color!(ptr, &o)
		return (o[0], o[1], o[2], o[3])
	}

	public func variantSetColor(_ ptr: UnsafeMutableRawPointer?, _ r: Float, _ g: Float, _ b: Float, _ a: Float) {
		requireHost().pointee.variant_set_color!(ptr, r, g, b, a)
	}

	public func variantAsQuaternion(_ ptr: UnsafeRawPointer?) -> (Float, Float, Float, Float) {
		var o = [Float](repeating: 0, count: 4)
		requireHost().pointee.variant_as_quaternion!(ptr, &o)
		return (o[0], o[1], o[2], o[3])
	}

	public func variantSetQuaternion(_ ptr: UnsafeMutableRawPointer?, _ x: Float, _ y: Float, _ z: Float, _ w: Float) {
		requireHost().pointee.variant_set_quaternion!(ptr, x, y, z, w)
	}

	public func variantAsRect2(_ ptr: UnsafeRawPointer?) -> (Float, Float, Float, Float) {
		var o = [Float](repeating: 0, count: 4)
		requireHost().pointee.variant_as_rect2!(ptr, &o)
		return (o[0], o[1], o[2], o[3])
	}

	public func variantSetRect2(_ ptr: UnsafeMutableRawPointer?, _ px: Float, _ py: Float, _ sx: Float, _ sy: Float) {
		requireHost().pointee.variant_set_rect2!(ptr, px, py, sx, sy)
	}

	public func variantAsAABB(_ ptr: UnsafeRawPointer?) -> [Float] {
		var o = [Float](repeating: 0, count: 6)
		requireHost().pointee.variant_as_aabb!(ptr, &o)
		return o
	}

	public func variantSetAABB(_ ptr: UnsafeMutableRawPointer?, _ v: [Float]) {
		v.withUnsafeBufferPointer { requireHost().pointee.variant_set_aabb!(ptr, $0.baseAddress) }
	}

	public func variantAsPlane(_ ptr: UnsafeRawPointer?) -> (Float, Float, Float, Float) {
		var o = [Float](repeating: 0, count: 4)
		requireHost().pointee.variant_as_plane!(ptr, &o)
		return (o[0], o[1], o[2], o[3])
	}

	public func variantSetPlane(_ ptr: UnsafeMutableRawPointer?, _ a: Float, _ b: Float, _ c: Float, _ d: Float) {
		requireHost().pointee.variant_set_plane!(ptr, a, b, c, d)
	}

	public func variantAsTransform2D(_ ptr: UnsafeRawPointer?) -> [Float] {
		var o = [Float](repeating: 0, count: 6)
		requireHost().pointee.variant_as_transform2d!(ptr, &o)
		return o
	}

	public func variantSetTransform2D(_ ptr: UnsafeMutableRawPointer?, _ v: [Float]) {
		v.withUnsafeBufferPointer { requireHost().pointee.variant_set_transform2d!(ptr, $0.baseAddress) }
	}

	public func variantAsTransform3D(_ ptr: UnsafeRawPointer?) -> [Float] {
		var o = [Float](repeating: 0, count: 12)
		requireHost().pointee.variant_as_transform3d!(ptr, &o)
		return o
	}

	public func variantSetTransform3D(_ ptr: UnsafeMutableRawPointer?, _ v: [Float]) {
		v.withUnsafeBufferPointer { requireHost().pointee.variant_set_transform3d!(ptr, $0.baseAddress) }
	}

	public func variantAsBasis(_ ptr: UnsafeRawPointer?) -> [Float] {
		var o = [Float](repeating: 0, count: 9)
		requireHost().pointee.variant_as_basis!(ptr, &o)
		return o
	}

	public func variantSetBasis(_ ptr: UnsafeMutableRawPointer?, _ v: [Float]) {
		v.withUnsafeBufferPointer { requireHost().pointee.variant_set_basis!(ptr, $0.baseAddress) }
	}

	public func variantAsProjection(_ ptr: UnsafeRawPointer?) -> [Float] {
		var o = [Float](repeating: 0, count: 16)
		requireHost().pointee.variant_as_projection!(ptr, &o)
		return o
	}

	public func variantSetProjection(_ ptr: UnsafeMutableRawPointer?, _ v: [Float]) {
		v.withUnsafeBufferPointer { requireHost().pointee.variant_set_projection!(ptr, $0.baseAddress) }
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

	// Instantiates a registered engine class (e.g. "Label") through ClassDB.
	public func createObject(_ className: String) -> UnsafeMutableRawPointer? {
		className.withCString { requireHost().pointee.object_create!($0) }
	}
}
