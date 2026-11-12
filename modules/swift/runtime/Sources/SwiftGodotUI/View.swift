import Foundation
import Observation
import SwiftGodotRuntime

// Declarative UI for Godot, SwiftUI-style but with native Godot nodes as the
// leaf primitives (the generated bindings in GodotBindings.swift).
// Everything is @MainActor: Godot's node tree is main-thread-only.

@MainActor
public protocol View {
	associatedtype Body: View
	@ViewBuilder var body: Body { get }
}

extension View {
	// Renders this view into a Godot node. Default: render the body.
	public func render() -> Node? {
		body.render()
	}
}

public struct EmptyView: View {
	public init() {}
	public var body: some View { EmptyView() }
	// Overridden to avoid infinite recursion through the default render().
	public func render() -> Node? { nil }
}

// A leaf view wrapping a native Godot node (Label, Button, VBoxContainer...).
public struct NodeView<Root: Node>: View {
	public let node: Root

	public init(_ node: Root) {
		self.node = node
	}

	public var body: some View { EmptyView() }

	public func render() -> Node? { node }
}

// Several children in one body: rendered into a plain Node group. Layout
// containers (VBox/HStack-like wrappers) come later on top of this.
public struct TupleView: View {
	public let content: [any View]

	public init(content: [any View]) {
		self.content = content
	}

	public var body: some View { EmptyView() }

	public func render() -> Node? {
		switch content.count {
		case 0:
			return nil
		case 1:
			return content[0].render()
		default:
			let group = Node()
			for child in content {
				if let node = child.render() {
					group.add_child(node: node)
				}
			}
			return group
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

	public func render() -> Node? { content?.render() }
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

	public func render() -> Node? {
		switch value {
		case .first(let c0):
			return c0.render()
		case .second(let c1):
			return c1.render()
		}
	}
}
