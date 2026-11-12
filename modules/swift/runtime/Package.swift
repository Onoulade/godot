// swift-tools-version:5.9
import PackageDescription

let package = Package(
	name: "swiftgodot-runtime",
	products: [
		.library(name: "swiftgodot_runtime", type: .dynamic, targets: ["SwiftGodotRuntime"]),
	],
	targets: [
		.target(name: "SwiftGodotRuntime", path: "Sources/SwiftGodotRuntime"),
	]
)
