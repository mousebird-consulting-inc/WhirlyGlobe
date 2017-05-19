//
//  NASAGIBSTestCase.swift
//  AutoTester
//
//  Created by jmnavarro on 6/12/15.
//  Copyright © 2015-2017 mousebird consulting. All rights reserved.
//

import UIKit

class NASAGIBSTestCase: MaplyTestCase {

	override init() {
		super.init()
		
		self.name = "NASA GIBS"
		self.captureDelay = 4
		self.implementations = [.globe, .map]
	}
	
	override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
		setupBaseLayer(globeVC)
		setupOverlaysLayer(globeVC)
		globeVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(-122.4192,37.7793), time: 1)
	}
	
	override func setUpWithMap(_ mapVC: MaplyViewController) {
		setupBaseLayer(mapVC)
		setupOverlaysLayer(mapVC)
		mapVC.height = 0.5
		mapVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(-122.4192,37.7793), time: 1)
	}
	
	func setupBaseLayer (_ baseLayer: MaplyBaseViewController) {
		let baseCacheDir = NSSearchPathForDirectoriesInDomains(FileManager.SearchPathDirectory.cachesDirectory, FileManager.SearchPathDomainMask.userDomainMask, true)
		let nasaTilescacheDir = "\(baseCacheDir)/nasatiles/"
		let maxZoom = Int32(9)
		let tileSource = MaplyRemoteTileSource(baseURL: "http://s.basemaps.cartocdn.com/dark_all/", ext: "png", minZoom: 0, maxZoom: maxZoom)
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
		
		baseLayer.add(layer!)
	}
	
	func setupOverlaysLayer (_ baseLayer: MaplyBaseViewController) {
		let cacheDir = NSSearchPathForDirectoriesInDomains(FileManager.SearchPathDirectory.cachesDirectory, FileManager.SearchPathDomainMask.userDomainMask, true)
        let tileInfo = MaplyRemoteTileInfo(baseURL: "http://map1.vis.earthdata.nasa.gov/wmts-webmerc/VIIRS_CityLights_2012/default/2015-07-01/GoogleMapsCompatible_Level8/{z}/{y}/{x}", ext: "jpg", minZoom: 0, maxZoom: 8)
        let tileSource = MaplyRemoteTileSource(info: tileInfo)
		tileSource?.cacheDir = "\(cacheDir)/sea_temperature"
		tileInfo.cachedFileLifetime = 3 //Invalidate OWM data after three secs
		let temperatureLayer = MaplyQuadImageTilesLayer(coordSystem: tileSource!.coordSys, tileSource: tileSource!)
		temperatureLayer?.coverPoles = false
		temperatureLayer?.handleEdges = false
		
		baseLayer.add(temperatureLayer!)
	}

}
