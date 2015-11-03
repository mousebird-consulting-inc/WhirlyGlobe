//
//  MapzenVectorTestCase.swift
//  AutoTester
//
//  Created by jmnavarro on 29/10/15.
//  Copyright © 2015 mousebird consulting. All rights reserved.
//

import UIKit

class MapzenVectorTestCase: MaplyTestCase, MaplyViewControllerDelegate {

	override init() {
		super.init()
		
		self.name = "Mapzen Vectors"
		self.captureDelay = 5
	}
	
	override func setUpWithMap(mapVC: MaplyViewController) -> Bool {
		let baseLayer = GeographyClassTestCase()
		baseLayer.setUpWithMap(mapVC)
		let cacheDir = NSSearchPathForDirectoriesInDomains(.CachesDirectory, .UserDomainMask, true)[0]

		let thisCacheDir = "\(cacheDir)/mapzen-vectiles/"
		let bundle = NSBundle.mainBundle()
		let path = bundle.pathForResource("MapzenStyles", ofType: "json")
		
		MaplyMapnikVectorTiles.StartRemoteVectorTilesWithURL(
			"http://vector.mapzen.com/osm/all/",
			ext: "mapbox",
			minZoom: Int32(8),
			maxZoom: Int32(14),
			accessToken: "vector-tiles-ejNTZ28",
			style: path!,
			styleType: MapnikStyleType.MapboxGLStyle,
			cacheDir: thisCacheDir, viewC: mapVC,
			success: { vecTiles in
				let pagelayer = MaplyQuadPagingLayer.init(coordSystem: MaplySphericalMercator.init(webStandard: ()), delegate: vecTiles)
				pagelayer?.numSimultaneousFetches = 4
				pagelayer?.flipY = false;
				pagelayer?.importance = 1024*1024
				pagelayer?.useTargetZoomLevel = true
				pagelayer?.singleLevelLoading = true
				mapVC.addLayer(pagelayer!)
				mapVC.animateToPosition(MaplyCoordinateMakeWithDegrees(-3.6704803, 40.5023056), height: 0.02, time: 1.0)
			}) {
				NSLog("Failed to load Mapnik vector tiles because: %@", $0)
			}
		return true
	}

}
