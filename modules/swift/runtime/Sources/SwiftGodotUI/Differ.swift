import Foundation
import SwiftGodotRuntime

// Component stack + structural differ. While a `body` chain evaluates, the
// component stack gives every component a stable identity (type + position),
// which the differ uses to update the Godot node tree in place instead of
// rebuilding it.

@MainActor
public struct ComponentFrame {
	public let id: String
	public let viewType: String
	public var children: [ComponentFrame] = []

	public init(id: String, viewType: String, children: [ComponentFrame] = []) {
		self.id = id
		self.viewType = viewType
		self.children = children
	}
}

@MainActor
public final class ComponentStack {
	public static let shared = ComponentStack()

	private var stack: [ComponentFrame] = []

	private init() {}

	public func push(_ frame: ComponentFrame) {
		stack.append(frame)
	}

	public func pop() -> ComponentFrame? {
		stack.popLast()
	}

	public var current: ComponentFrame? {
		stack.last
	}
}

@MainActor
public final class Differ {
	public static let shared = Differ()

	private init() {}

	// Reconciles `parent`'s children with a freshly rendered subtree: the same
	// component type at the same position keeps the existing node (fine-grained
	// in-place updates land with the component stack); a different type or a
	// new component replaces the subtree.
	public func apply(old: ComponentFrame?, new: ComponentFrame, newNode: Node?, parent: Node) {
		if let old, old.viewType == new.viewType, parent.get_child_count() > 0 {
			return
		}
		removeAllChildren(of: parent)
		if let newNode {
			parent.add_child(node: newNode)
		}
	}

	private func removeAllChildren(of parent: Node) {
		while parent.get_child_count() > 0, let child = parent.get_child(idx: 0) {
			parent.remove_child(node: child)
		}
	}
}
