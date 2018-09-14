//
//  AnimatedBasemapTestCase.swift
//  AutoTester
//
//  Created by jmnavarro on 26/10/15.
//  Copyright © 2015-2017 mousebird consulting. All rights reserved.
//

import UIKit

class AnimatedBasemapTestCase: MaplyTestCase {

	let geographyClass = GeographyClassTestCase()
	let cacheDir = NSSearchPathForDirectoriesInDomains(.cachesDirectory, .userDomainMask, true)[0]
    var imageLayer : MaplyQuadImageFrameLoader? = nil

	override init() {
		super.init()

		self.name = "Animated basemap"
		self.captureDelay = 2
		self.implementations = [.globe, .map]
	}
    
    // Put together a sampling layer and loader
    func setupLoader(_ baseVC: MaplyBaseViewController) {
        var tileSources : [MaplyRemoteTileInfoNew] = []

        for i in 0...4 {
            let precipSource = MaplyRemoteTileInfoNew(baseURL: "http://a.tiles.mapbox.com/v3/mousebird.precip-example-layer\(i)/{z}/{x}/{y}.png",
                minZoom: 0,
                maxZoom: 6)
            precipSource.cacheDir = "\(cacheDir)/forecast_io_weather_layer\(i)/"
            tileSources.append(precipSource)
        }

        // Parameters describing how we want a globe broken down
        let sampleParams = MaplySamplingParams()
        sampleParams.coordSys = MaplySphericalMercator(webStandard: ())
        sampleParams.coverPoles = false
        sampleParams.edgeMatching = true
        sampleParams.minZoom = 0
        sampleParams.maxZoom = 6
        sampleParams.singleLevel = true

        imageLayer = MaplyQuadImageFrameLoader(params: sampleParams, tileInfos: tileSources, viewC: baseVC)
    }

	override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
		geographyClass.setUpWithGlobe(globeVC)
        setupLoader(globeVC)
	}

	override func setUpWithMap(_ mapVC: MaplyViewController) {
		geographyClass.setUpWithMap(mapVC)
        setupLoader(mapVC)
	}

}
