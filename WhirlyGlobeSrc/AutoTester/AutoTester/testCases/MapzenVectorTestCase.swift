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
		let mainBundle = NSBundle.mainBundle()
		let styleData = NSData.init(contentsOfFile: mainBundle.pathForResource("MapzenGLStyle", ofType: "json")!)
		let cacheDir = NSSearchPathForDirectoriesInDomains(.CachesDirectory, .UserDomainMask, true)[0]

		let thisCacheDir = "\(cacheDir)/mapzen-vectiles/"
		
		let mzSource = MapzenSource.init(base: "http://vector.mapzen.com/osm", layers: ["all"], apiKey: "vector-tiles-ejNTZ28", sourceType: MapzenSourcePBF, styleData: styleData, styleType: MapnikStyleType.MapboxGLStyle, viewC: mapVC)
		
		mzSource.minZoom = Int32(0)
		mzSource.maxZoom = Int32(24)
		
		let pageLayer = MaplyQuadPagingLayer.init(coordSystem: MaplySphericalMercator.init(webStandard: ()), delegate: mzSource)
		
		pageLayer?.numSimultaneousFetches = Int32(8)
		pageLayer?.flipY = false
		pageLayer?.importance = 512*512
		pageLayer?.useTargetZoomLevel = true
		pageLayer?.singleLevelLoading = true
		mapVC.addLayer(pageLayer!)
		mapVC.animateToPosition(MaplyCoordinateMakeWithDegrees(-3.6704803, 40.50230), height: 0.03, time: 1)

		return true
	}

}
