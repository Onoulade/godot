import Foundation
import Observation

// Attribute graph with scoped, propagated invalidation (SwiftUI-style):
// each unit of work (a `body` evaluation, a computed value) is an Attribute;
// reads of @Observable/@State values inside it are tracked with Apple's
// Observation framework, and a change marks only the dependent attributes
// dirty, then re-evaluates them coalesced on the main actor.

@MainActor
public final class AttributeGraph {
	public static let shared = AttributeGraph()

	public typealias AttributeID = ObjectIdentifier

	private var recomputes: [AttributeID: @MainActor () -> Void] = [:]
	private var dirty: Set<AttributeID> = []
	private var flushScheduled = false

	private init() {}

	public func register(id: AttributeID, recompute: @escaping @MainActor () -> Void) {
		recomputes[id] = recompute
	}

	public func unregister(id: AttributeID) {
		recomputes.removeValue(forKey: id)
		dirty.remove(id)
	}

	// Evaluates an attribute, tracking the @Observable reads it performs so
	// that any later change re-invalidates exactly this attribute.
	public func evaluate(_ id: AttributeID) {
		guard let recompute = recomputes[id] else { return }
		withObservationTracking {
			recompute()
		} onChange: { [weak self] in
			self?.invalidate(id)
		}
	}

	public func invalidate(_ id: AttributeID) {
		dirty.insert(id)
		scheduleFlush()
	}

	private func scheduleFlush() {
		guard !flushScheduled else { return }
		flushScheduled = true
		Task { @MainActor in
			self.flushScheduled = false
			self.flush()
		}
	}

	// Re-evaluates all dirty attributes (scoped: only what changed).
	public func flush() {
		while let id = dirty.popFirst() {
			evaluate(id)
		}
	}
}

extension Set {
	mutating func popFirst() -> Element? {
		guard let first = first else { return nil }
		remove(first)
		return first
	}
}
