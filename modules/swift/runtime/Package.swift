// swift-tools-version:6.0
import PackageDescription

let package = Package(
	name: "swiftgodot-runtime",
	products: [
		.library(name: "swiftgodot_runtime", type: .dynamic, targets: ["SwiftGodotRuntime"]),
		.library(name: "SwiftGodotUI", type: .dynamic, targets: ["SwiftGodotUI"]),
	],
	// No external dependencies: Apple's Observation framework (@Observable,
	// withObservationTracking) ships inside the Swift toolchain's standard
	// library (swiftlang/swift, stdlib/public/Observation) on every platform
	// Swift supports, including Linux and Windows — `import Observation` just
	// works with Swift >= 5.9.
	targets: [
		.target(name: "SwiftGodotRuntime", path: "Sources/SwiftGodotRuntime"),
		.target(
			name: "SwiftGodotUI",
			dependencies: [
				"SwiftGodotRuntime",
			],
			path: "Sources/SwiftGodotUI"
		),
	]
)
