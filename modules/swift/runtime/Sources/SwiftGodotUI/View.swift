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
	@ViewBuilder public var body: some View { EmptyView() }
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

	@ViewBuilder public var body: some View { EmptyView() }

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

	@ViewBuilder public var body: some View { EmptyView() }

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

@MainActor
@resultBuilder
public enum ViewBuilder {
	public static func buildBlock() -> EmptyView {
		EmptyView()
	}

	// Heterogeneous children without parameter packs (VariadicGenerics is
	// still experimental in Swift 6): fixed-arity overloads up to 10.
	public static func buildBlock<C0: View>(_ c0: C0) -> TupleView {
		TupleView(content: [c0])
	}

	public static func buildBlock<C0: View, C1: View>(_ c0: C0, _ c1: C1) -> TupleView {
		TupleView(content: [c0, c1])
	}

	public static func buildBlock<C0: View, C1: View, C2: View>(_ c0: C0, _ c1: C1, _ c2: C2) -> TupleView {
		TupleView(content: [c0, c1, c2])
	}

	public static func buildBlock<C0: View, C1: View, C2: View, C3: View>(_ c0: C0, _ c1: C1, _ c2: C2, _ c3: C3) -> TupleView {
		TupleView(content: [c0, c1, c2, c3])
	}

	public static func buildBlock<C0: View, C1: View, C2: View, C3: View, C4: View>(_ c0: C0, _ c1: C1, _ c2: C2, _ c3: C3, _ c4: C4) -> TupleView {
		TupleView(content: [c0, c1, c2, c3, c4])
	}

	public static func buildBlock<C0: View, C1: View, C2: View, C3: View, C4: View, C5: View>(_ c0: C0, _ c1: C1, _ c2: C2, _ c3: C3, _ c4: C4, _ c5: C5) -> TupleView {
		TupleView(content: [c0, c1, c2, c3, c4, c5])
	}

	public static func buildBlock<C0: View, C1: View, C2: View, C3: View, C4: View, C5: View, C6: View>(_ c0: C0, _ c1: C1, _ c2: C2, _ c3: C3, _ c4: C4, _ c5: C5, _ c6: C6) -> TupleView {
		TupleView(content: [c0, c1, c2, c3, c4, c5, c6])
	}

	public static func buildBlock<C0: View, C1: View, C2: View, C3: View, C4: View, C5: View, C6: View, C7: View>(_ c0: C0, _ c1: C1, _ c2: C2, _ c3: C3, _ c4: C4, _ c5: C5, _ c6: C6, _ c7: C7) -> TupleView {
		TupleView(content: [c0, c1, c2, c3, c4, c5, c6, c7])
	}

	public static func buildBlock<C0: View, C1: View, C2: View, C3: View, C4: View, C5: View, C6: View, C7: View, C8: View>(_ c0: C0, _ c1: C1, _ c2: C2, _ c3: C3, _ c4: C4, _ c5: C5, _ c6: C6, _ c7: C7, _ c8: C8) -> TupleView {
		TupleView(content: [c0, c1, c2, c3, c4, c5, c6, c7, c8])
	}

	public static func buildBlock<C0: View, C1: View, C2: View, C3: View, C4: View, C5: View, C6: View, C7: View, C8: View, C9: View>(_ c0: C0, _ c1: C1, _ c2: C2, _ c3: C3, _ c4: C4, _ c5: C5, _ c6: C6, _ c7: C7, _ c8: C8, _ c9: C9) -> TupleView {
		TupleView(content: [c0, c1, c2, c3, c4, c5, c6, c7, c8, c9])
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

	@ViewBuilder public var body: some View {
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

	public enum Either<First, Second> {
		case first(First)
		case second(Second)
	}

	@ViewBuilder public var body: some View {
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
