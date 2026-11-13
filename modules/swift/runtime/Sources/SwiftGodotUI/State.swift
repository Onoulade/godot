import Foundation
import Observation

// Backing storage for @State, built on Apple's real Observation framework:
// the wrapped value lives in an @Observable box, so reads performed while an
// attribute evaluates are tracked and any write invalidates exactly the
// dependent attributes (scoped, propagated invalidation).

@Observable
public final class StateBox<Value> {
	public var value: Value

	public init(value: Value) {
		self.value = value
	}
}

@propertyWrapper
public struct State<Value> {
	private let box: StateBox<Value>

	public init(wrappedValue: Value) {
		box = StateBox(value: wrappedValue)
	}

	public var wrappedValue: Value {
		get { box.value }
		set { box.value = newValue }
	}

	public var projectedValue: StateBox<Value> {
		box
	}
}
