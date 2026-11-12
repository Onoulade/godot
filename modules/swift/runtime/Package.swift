// swift-tools-version:6.0
import PackageDescription

let package = Package(
	name: "swiftgodot-runtime",
	products: [
		.library(name: "swiftgodot_runtime", type: .dynamic, targets: ["SwiftGodotRuntime"]),
		.library(name: "SwiftGodotUI", type: .dynamic, targets: ["SwiftGodotUI"]),
	],
	dependencies: [
		// Apple's Observation framework (@Observable, withObservationTracking),
		// cross-platform, used for scoped dependency tracking in the UI layer.
		.package(url: "https://github.com/apple/swift-observation.git", from: "0.1.0"),
	],
	targets: [
		.target(name: "SwiftGodotRuntime", path: "Sources/SwiftGodotRuntime"),
		.target(
			name: "SwiftGodotUI",
			dependencies: [
				"SwiftGodotRuntime",
				.product(name: "Observation", package: "swift-observation"),
			],
			path: "Sources/SwiftGodotUI"
		),
	]
)
