// swift-tools-version: 5.9
// The swift-tools-version declares the minimum version of Swift required to build this package.

import PackageDescription

let package = Package(
    name: "WhirlyGlobe",
    platforms: [
        .iOS(.v14)
    ],
    products: [
        .library(
            name: "WhirlyGlobe",
            targets: ["WhirlyGlobe"]
        )
    ],
    dependencies: [
        .package(name: "WhirlyGlobe-common", path: "../../common/")
    ],
    targets: [
        .target(
            name: "WhirlyGlobe",
            dependencies: [
                "WhirlyGlobe-MaplyComponent-ObjC"
            ],
            path: ".",
            sources: []
        ),
        .target(
            name: "WhirlyGlobeLib-iOS",
            dependencies: [
                .product(name: "WhirlyGlobe-common", package: "WhirlyGlobe-common"),
            ],
            path: "WhirlyGlobeLib",
            publicHeadersPath: "include"
        ),
        .target(
            name: "WhirlyGlobe-MaplyComponent-ObjC",
            dependencies: [
                "WhirlyGlobeLib-iOS"
            ],
            path: "WhirlyGlobe-MaplyComponent",
            exclude: ["src/helpers/MapboxKindaMap.swift"],
            publicHeadersPath: "include"
        )
    ],
    cxxLanguageStandard: .cxx14
)

