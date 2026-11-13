// swift-tools-version:6.0
import PackageDescription

let package = Package(
	name: "swiftgodot-runtime",
	// Observation (@Observable / withObservationTracking) needs macOS 14+ on
	// Apple platforms. On Linux/Windows it ships inside the Swift toolchain
	// itself, so this only raises the Apple deployment target.
	platforms: [.macOS(.v14)],
	products: [
		// One single dynamic library for both targets: a separate UI dylib
		// would statically embed SwiftGodotRuntime's objects, and loading both
		// dylibs would register every Swift/ObjC class twice (objc
		// "implemented in both" warnings and spurious cast crashes on macOS).
		.library(name: "swiftgodot_runtime", type: .dynamic, targets: ["SwiftGodotRuntime", "SwiftGodotUI"]),
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
