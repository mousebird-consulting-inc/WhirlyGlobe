//
//  NASAGIBSTestCase.swift
//  AutoTester
//
//  Created by jmnavarro on 6/12/15.
//  Copyright © 2015 mousebird consulting. All rights reserved.
//

import UIKit

class NASAGIBSTestCase: MaplyTestCase {

	override init() {
		super.init()
		
		self.name = "NASA GIBS"
		self.captureDelay = 4
	}
	
	override func setUpWithGlobe(globeVC: WhirlyGlobeViewController) -> Bool {
		setupBaseLayer(globeVC)
		setupOverlaysLayer(globeVC)
		globeVC.animateToPosition(MaplyCoordinateMakeWithDegrees(-122.4192,37.7793), time: 1)
		return true
	}
	
	override func setUpWithMap(mapVC: MaplyViewController) -> Bool {
		setupBaseLayer(mapVC)
		setupOverlaysLayer(mapVC)
		mapVC.height = 0.5
		mapVC.animateToPosition(MaplyCoordinateMakeWithDegrees(-122.4192,37.7793), time: 1)
		return true
	}
	
	func setupBaseLayer (baseLayer: MaplyBaseViewController) {
		let baseCacheDir = NSSearchPathForDirectoriesInDomains(NSSearchPathDirectory.CachesDirectory, NSSearchPathDomainMask.UserDomainMask, true)
		let nasaTilescacheDir = "\(baseCacheDir)/nasatiles/"
		let maxZoom = Int32(9)
		let tileSource = MaplyRemoteTileSource(baseURL: "http://otile1.mqcdn.com/tiles/1.0.0/sat/", ext: "jpg", minZoom: 0, maxZoom: maxZoom)
		tileSource?.cacheDir = nasaTilescacheDir
		let layer = MaplyQuadImageTilesLayer(coordSystem: tileSource!.coordSys, tileSource: tileSource!)
		
		if baseLayer is WhirlyGlobeViewController {
			layer?.handleEdges = true
			layer?.coverPoles = true
		}
		else {
			layer?.handleEdges = false
			layer?.coverPoles = false
		}
		layer?.requireElev = false
		layer?.waitLoad = false
		layer?.drawPriority = 0
		layer?.singleLevelLoading = false
		
		baseLayer.addLayer(layer!)
	}
	
	func setupOverlaysLayer (baseLayer: MaplyBaseViewController) {
		let cacheDir = NSSearchPathForDirectoriesInDomains(NSSearchPathDirectory.CachesDirectory, NSSearchPathDomainMask.UserDomainMask, true)
		let tileSource = MaplyRemoteTileSource(baseURL: "http://map1.vis.earthdata.nasa.gov/wmts-webmerc/Sea_Surface_Temp_Blended/default/2015-06-25/GoogleMapsCompatible_Level7/{z}/{y}/{x}", ext: "png", minZoom: 0, maxZoom: 7)
		tileSource?.cacheDir = "\(cacheDir)/sea_temperature"
		tileSource?.tileInfo.cachedFileLifetime = 3 //Invalidate OWM data after three secs
		let temperatureLayer = MaplyQuadImageTilesLayer(coordSystem: tileSource!.coordSys, tileSource: tileSource!)
		temperatureLayer?.coverPoles = false
		temperatureLayer?.handleEdges = false
		
		baseLayer.addLayer(temperatureLayer!)
	}

}
