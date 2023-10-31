// swift-tools-version: 5.9
// The swift-tools-version declares the minimum version of Swift required to build this package.

import PackageDescription

let package = Package(
    name: "WhirlyGlobe-common",
    platforms: [
        .iOS(.v14)
    ],
    products: [
        .library(
            name: "WhirlyGlobe-common",
            targets: [
                "WhirlyGlobeLib",
                "GeographicLib", 
//                "KissXML",
                "SMCalloutView",
                "aaplus",
                "clipper",
                "eigen",
                "glues",
                "libjson",
                "lodepng",
                "nanopb",
                "proj-4",
                "shapefile"
            ]
        )
    ],
    dependencies: [],
    targets: [
        .target(
            name: "WhirlyGlobeLib",
            path: "WhirlyGlobeLib",
            publicHeadersPath: "include"
        ),
        .target(
            name: "GeographicLib",
            path: "local_libs/GeographicLib",
            publicHeadersPath: "include"
        ),
//        .target(
//            name: "KissXML",
//            path: "local_libs/KissXML",
//            exclude: ["DDXML.swift"],
//            publicHeadersPath: "."
//        ),
        .target(
            name: "SMCalloutView",
            path: "local_libs/SMCalloutView",
            publicHeadersPath: "."
        ),
        .target(
            name: "aaplus",
            path: "local_libs/aaplus",
            publicHeadersPath: "."
        ),
        .target(
            name: "clipper",
            path: "local_libs/clipper/cpp",
            publicHeadersPath: "."
        ),
        .target(
            name: "eigen",
            path: "local_libs/eigen",
            publicHeadersPath: "."
        ),
        .target(
            name: "glues",
            path: "local_libs/glues",
            sources: ["source/libtess"],
            exclude: ["WrapperGLES.h"],
            publicHeadersPath: "include"
        ),
        .target(
            name: "libjson",
            path: "local_libs/libjson/_internal/Source",
            publicHeadersPath: "."
        ),
        .target(
            name: "lodepng",
            path: "local_libs/lodepng",
            publicHeadersPath: "."
        ),
        .target(
            name: "nanopb",
            path: "local_libs/nanopb",
            exclude: ["generator", "tools"],
            publicHeadersPath: "."
        ),
        .target(
            name: "proj-4",
            path: "local_libs/proj-4/src/",
            publicHeadersPath: "."
        ),
        .target(
            name: "shapefile",
            path: "local_libs/shapefile",
            publicHeadersPath: "."
        )
    ],
    cxxLanguageStandard: .cxx14
)
