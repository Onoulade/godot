import Foundation
import SwiftGodotRuntime

// Scene entry points called by the engine (SwiftViewHost) through the ABI:
// swiftgodot_scene_build / swiftgodot_scene_invalidate.

@_cdecl("swiftgodot_scene_build")
public func swiftgodot_scene_build(_ host: UnsafeMutableRawPointer?) -> UInt {
	let h = SendableRawPointer(host)
	MainActor.assumeIsolated {
		SceneRegistry.shared.build(host: h.ptr).map { UInt(bitPattern: $0) } ?? 0
	}
}

@_cdecl("swiftgodot_scene_invalidate")
public func swiftgodot_scene_invalidate(_ host: UnsafeMutableRawPointer?) {
	let h = SendableRawPointer(host)
	MainActor.assumeIsolated {
		SceneRegistry.shared.invalidate(host: h.ptr)
	}
}

@MainActor
public final class SceneRegistry {
	public static let shared = SceneRegistry()

	private var namedRoots: [String: any View] = [:]
	private var lastRoot: (any View)?
	private var trees: [ObjectIdentifier: RenderedView] = [:]

	private init() {}

	// Called by the per-script compiled library (swiftgodot_scene_register)
	// when the script declares a root view (`struct MyScene: View`).
	public func registerRoot<V: View>(_ view: V, named name: String) {
		namedRoots[name] = view
		lastRoot = view
	}

	// The root view for a host: its "root_view" property names the Swift View
	// struct to render; with no name set, the last registered root is used.
	private func rootView(for host: UnsafeMutableRawPointer?) -> (any View)? {
		if let host {
			let name = Host.shared.getProperty(object: host, name: "root_view").asString()
			if !name.isEmpty, let view = namedRoots[name] {
				return view
			}
		}
		return lastRoot
	}

	public func build(host: UnsafeMutableRawPointer?) -> UnsafeMutableRawPointer? {
		guard let host, let hostNode = Node(raw: host), let view = rootView(for: host) else { return nil }
		let id = ObjectIdentifier(hostNode)
		if let existing = trees[id] {
			return existing.node?.ptr
		}
		let tree = view.renderTree()
		trees[id] = tree
		return tree.node?.ptr
	}

	public func invalidate(host: UnsafeMutableRawPointer?) {
		guard let host, let hostNode = Node(raw: host), let view = rootView(for: host) else { return }
		let id = ObjectIdentifier(hostNode)
		// Scoped, propagated invalidation: the root body is re-evaluated as an
		// attribute of the graph; Observation tracks the @State/@Observable
		// reads it performs, so only the dependents of a change are re-run,
		// coalesced on the main actor, then diffed into the node tree.
		let attribute = AttributeGraph.AttributeID(self)
		AttributeGraph.shared.register(id: attribute) { [weak self] in
			guard let self else { return }
			let newTree = view.renderTree()
			Differ.shared.apply(old: trees[id], new: newTree, parent: hostNode, at: 0)
			trees[id] = newTree
		}
		AttributeGraph.shared.evaluate(attribute)
	}
}
