//
//  AnimatedBasemapTestCase.swift
//  AutoTester
//
//  Created by jmnavarro on 26/10/15.
//  Copyright © 2015 mousebird consulting. All rights reserved.
//

import UIKit

class AnimatedBasemapTestCase: MaplyTestCase {

	override init() {
		super.init()

		self.name = "Animated basemap"
		self.captureDelay = 2
	}

	override func setUpWithGlobe(globeVC: WhirlyGlobeViewController) -> Bool {
		let geographyClass = GeographyClassTestCase()
		geographyClass.setUpWithGlobe(globeVC)

		let cacheDir = NSSearchPathForDirectoriesInDomains(.CachesDirectory, .UserDomainMask, true)[0]

		// Collect up the various precipitation sources
		var tileSources = [MaplyRemoteTileInfo]()
		for i in 0...4 {
			let precipTileSource = MaplyRemoteTileInfo(
				baseURL: "http://a.tiles.mapbox.com/v3/mousebird.precip-example-layer\(i)/",
				ext: "png",
				minZoom: 0,
				maxZoom: 6)
			precipTileSource.cacheDir = "\(cacheDir)/forecast_io_weather_layer\(i)/"
			tileSources.append(precipTileSource)
		}

		let precipTileSource = MaplyMultiplexTileSource(sources: tileSources)
		// Create a precipitation layer that animates
		let precipLayer = MaplyQuadImageTilesLayer(tileSource: precipTileSource!)
		precipLayer?.imageDepth = UInt32(tileSources.count)
		precipLayer?.animationPeriod = 6.0
		precipLayer?.imageFormat = MaplyQuadImageFormat.ImageUByteRed
		precipLayer?.numSimultaneousFetches = 4
		precipLayer?.handleEdges = false
		precipLayer?.coverPoles = false
		precipLayer?.shaderProgramName = WeatherShader.setupWeatherShader(globeVC)
		precipLayer?.fade = 0.5
		globeVC.addLayer(precipLayer!)

		return true
	}

	override func setUpWithMap(mapVC: MaplyViewController) -> Bool {
		let geographyClass = GeographyClassTestCase()
		geographyClass.setUpWithMap(mapVC)

		let cacheDir = NSSearchPathForDirectoriesInDomains(.CachesDirectory, .UserDomainMask, true)[0]

		// Collect up the various precipitation sources
		var tileSources = [MaplyRemoteTileInfo]()
		for i in 0...4 {
			let precipTileSource = MaplyRemoteTileInfo(
				baseURL: "http://a.tiles.mapbox.com/v3/mousebird.precip-example-layer\(i)/",
				ext: "png",
				minZoom: 0,
				maxZoom: 6)
			precipTileSource.cacheDir = "\(cacheDir)/forecast_io_weather_layer\(i)/"
			tileSources.append(precipTileSource)
		}

		let precipTileSource = MaplyMultiplexTileSource(sources: tileSources)
		// Create a precipitation layer that animates
		let precipLayer = MaplyQuadImageTilesLayer(tileSource: precipTileSource!)
		precipLayer?.imageDepth = UInt32(tileSources.count)
		precipLayer?.animationPeriod = 6.0
		precipLayer?.imageFormat = MaplyQuadImageFormat.ImageUByteRed
		precipLayer?.numSimultaneousFetches = 4
		precipLayer?.handleEdges = false
		precipLayer?.coverPoles = false
		precipLayer?.shaderProgramName = WeatherShader.setupWeatherShader(mapVC)
		precipLayer?.fade = 0.5
		mapVC.addLayer(precipLayer!)

		return true
	}

}
