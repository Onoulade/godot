import Foundation
import SwiftGodotRuntime

// Component stack + structural differ. While a `body` chain evaluates, the
// component stack tracks the path of composite views being rendered; the
// differ then reconciles the freshly rendered tree against the previous one
// and applies the minimal set of node mutations to the Godot tree: same view
// type at the same position keeps the existing container node and recurses,
// a different type replaces just that child, leaves are swapped in place so
// new properties apply.

@MainActor
public final class ComponentStack {
	public static let shared = ComponentStack()

	private var stack: [String] = []

	private init() {}

	public func push(_ viewType: String) {
		stack.append(viewType)
	}

	public func pop() {
		_ = stack.popLast()
	}

	public var current: String? {
		stack.last
	}

	public var depth: Int {
		stack.count
	}
}

@MainActor
public final class Differ {
	public static let shared = Differ()

	private init() {}

	// Applies `new` to `parent` at child position `index`, given the previous
	// render `old` (nil if none).
	public func apply(old: RenderedView?, new: RenderedView, parent: Node, at index: Int) {
		let existing: Node? = index < parent.get_child_count() ? parent.get_child(idx: index) : nil
		if let old, old.viewType == new.viewType, let existing {
			if new.children.isEmpty {
				// Leaf at the same position: swap the node in place.
				replaceChild(at: index, with: new.node, in: parent)
			} else {
				// Same container type: keep the node, reconcile children pairwise.
				reconcile(old: old.children, new: new.children, parent: existing)
			}
			return
		}
		replaceChild(at: index, with: new.node, in: parent)
	}

	private func reconcile(old: [RenderedView], new: [RenderedView], parent: Node) {
		for i in 0..<new.count {
			let oldChild = i < old.count ? old[i] : nil
			if let oldChild {
				apply(old: oldChild, new: new[i], parent: parent, at: i)
			} else if let node = new[i].node {
				parent.add_child(node: node)
			}
		}
		while parent.get_child_count() > new.count {
			guard let child = parent.get_child(idx: parent.get_child_count() - 1) else { break }
			parent.remove_child(node: child)
		}
	}

	private func replaceChild(at index: Int, with node: Node?, in parent: Node) {
		if index < parent.get_child_count(), let old = parent.get_child(idx: index) {
			parent.remove_child(node: old)
		}
		if let node {
			parent.add_child(node: node)
			if index < parent.get_child_count() - 1 {
				parent.move_child(child_node: node, to_index: index)
			}
		}
	}
}
