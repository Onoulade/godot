import Foundation
import SwiftGodotRuntime

// Scene entry points called by the engine (SwiftViewHost) through the ABI:
// swiftgodot_scene_build / swiftgodot_scene_invalidate.

@_cdecl("swiftgodot_scene_build")
public func swiftgodot_scene_build(_ host: UnsafeMutableRawPointer?) -> UnsafeMutableRawPointer? {
	MainActor.assumeIsolated {
		SceneRegistry.shared.build(host: host)
	}
}

@_cdecl("swiftgodot_scene_invalidate")
public func swiftgodot_scene_invalidate(_ host: UnsafeMutableRawPointer?) {
	MainActor.assumeIsolated {
		SceneRegistry.shared.invalidate(host: host)
	}
}

@MainActor
public final class SceneRegistry {
	public static let shared = SceneRegistry()

	private var rootView: (any View)?
	private var roots: [ObjectIdentifier: Node] = [:]

	private init() {}

	// Called by the per-script compiled library (swiftgodot_scene_register)
	// when the script declares a root view (`struct MyScene: View`).
	public func registerRoot<V: View>(_ view: V) {
		rootView = view
	}

	public func build(host: UnsafeMutableRawPointer?) -> UnsafeMutableRawPointer? {
		guard let rootView, let host, let hostNode = Node(raw: host) else { return nil }
		let id = ObjectIdentifier(hostNode.ptr)
		if let existing = roots[id] {
			return existing.ptr
		}
		guard let node = rootView.render() else { return nil }
		roots[id] = node
		return node.ptr
	}

	public func invalidate(host: UnsafeMutableRawPointer?) {
		guard let rootView, let host, let hostNode = Node(raw: host) else { return }
		let id = ObjectIdentifier(hostNode.ptr)
		// Scoped, propagated invalidation: the root body is re-evaluated as an
		// attribute of the graph; Observation tracks the @State/@Observable
		// reads it performs, so only the dependents of a change are re-run,
		// coalesced on the main actor, then diffed into the node tree.
		let attribute = AttributeGraph.AttributeID(self)
		AttributeGraph.shared.register(id: attribute) { [weak self] in
			guard let self, let newNode = rootView.render() else { return }
			let frame = ComponentFrame(id: "root", viewType: "\(type(of: rootView))")
			Differ.shared.apply(old: nil, new: frame, newNode: newNode, parent: hostNode)
			roots[id] = newNode
		}
		AttributeGraph.shared.evaluate(attribute)
	}
}
