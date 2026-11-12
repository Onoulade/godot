import Foundation
import Observation
import SwiftGodotRuntime

// Declarative UI for Godot, SwiftUI-style but with native Godot nodes as the
// leaf primitives (the generated bindings in GodotBindings.swift).
// Everything is @MainActor: Godot's node tree is main-thread-only.

// A rendered view: the Godot node it produced (nil for EmptyView) plus its
// component identity (view type + children), which the differ uses to update
// the node tree in place.
@MainActor
public struct RenderedView {
	public let viewType: String
	public let node: Node?
	public let children: [RenderedView]

	public init(viewType: String, node: Node?, children: [RenderedView] = []) {
		self.viewType = viewType
		self.node = node
		self.children = children
	}
}

@MainActor
public protocol View {
	associatedtype Body: View
	@ViewBuilder var body: Body { get }
	func renderTree() -> RenderedView
}

extension View {
	// Default: a view renders as its body (passthrough).
	public func renderTree() -> RenderedView {
		body.renderTree()
	}

	public func render() -> Node? {
		renderTree().node
	}
}

public struct EmptyView: View {
	public init() {}
	public var body: some View { EmptyView() }
	public func renderTree() -> RenderedView {
		RenderedView(viewType: "EmptyView", node: nil)
	}
}

// A leaf view wrapping a native Godot node (Label, Button, VBoxContainer...).
public struct NodeView<Root: Node>: View {
	public let node: Root

	public init(_ node: Root) {
		self.node = node
	}

	public var body: some View { EmptyView() }

	public func renderTree() -> RenderedView {
		RenderedView(viewType: String(describing: Root.self), node: node)
	}
}

// Several children in one body: rendered into a plain Node group (kept stable
// across re-renders; layout containers get their own wrappers later).
public struct TupleView: View {
	public let content: [any View]

	public init(content: [any View]) {
		self.content = content
	}

	public var body: some View { EmptyView() }

	public func renderTree() -> RenderedView {
		switch content.count {
		case 0:
			return RenderedView(viewType: "TupleView", node: nil)
		case 1:
			return content[0].renderTree()
		default:
			ComponentStack.shared.push("TupleView")
			defer { ComponentStack.shared.pop() }
			let rendered = content.map { $0.renderTree() }
			let group = Node()
			for child in rendered {
				if let node = child.node {
					group.add_child(node: node)
				}
			}
			return RenderedView(viewType: "TupleView", node: group, children: rendered)
		}
	}
}

@resultBuilder
public enum ViewBuilder {
	public static func buildBlock() -> EmptyView {
		EmptyView()
	}

	public static func buildBlock<each C: View>(repeat each component: each C) -> TupleView {
		TupleView(content: [repeat each component])
	}

	public static func buildOptional<C: View>(_ component: C?) -> OptionalView<C> {
		OptionalView(content: component)
	}

	public static func buildEither<C0: View, C1: View>(first: C0) -> EitherView<C0, C1> {
		EitherView(first: first)
	}

	public static func buildEither<C0: View, C1: View>(second: C1) -> EitherView<C0, C1> {
		EitherView(second: second)
	}
}

public struct OptionalView<Content: View>: View {
	public let content: Content?

	public init(content: Content?) {
		self.content = content
	}

	public var body: some View {
		if let content {
			content.body
		} else {
			EmptyView()
		}
	}

	public func renderTree() -> RenderedView {
		content?.renderTree() ?? RenderedView(viewType: "OptionalView", node: nil)
	}
}

public struct EitherView<C0: View, C1: View>: View {
	public let value: Either<C0, C1>

	public init(first: C0) {
		value = .first(first)
	}

	public init(second: C1) {
		value = .second(second)
	}

	public enum Either {
		case first(C0)
		case second(C1)
	}

	public var body: some View {
		switch value {
		case .first(let c0):
			c0.body
		case .second(let c1):
			c1.body
		}
	}

	public func renderTree() -> RenderedView {
		switch value {
		case .first(let c0):
			return c0.renderTree()
		case .second(let c1):
			return c1.renderTree()
		}
	}
}
