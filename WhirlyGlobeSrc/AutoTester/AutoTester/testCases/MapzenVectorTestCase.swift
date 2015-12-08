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
		let baseLayer = MapBoxSatelliteTestCase()
		baseLayer.setUpWithMap(mapVC)

		let styleData = NSData(contentsOfFile: NSBundle.mainBundle().pathForResource("MapzenGLStyle", ofType: "json")!)

		let mzSource = MapzenSource(
			base: "http://vector.mapzen.com/osm",
			layers: ["all"],
			apiKey: "vector-tiles-ejNTZ28",
			sourceType: MapzenSourcePBF,
			styleData: styleData,
			styleType: .MapboxGLStyle,
			viewC: mapVC)

		mzSource.minZoom = Int32(0)
		mzSource.maxZoom = Int32(24)

		let pageLayer = MaplyQuadPagingLayer(
			coordSystem: MaplySphericalMercator(),
			delegate: mzSource)

		pageLayer?.numSimultaneousFetches = Int32(8)
		pageLayer?.flipY = false
		pageLayer?.importance = 512*512
		pageLayer?.useTargetZoomLevel = true
		pageLayer?.singleLevelLoading = true
		mapVC.addLayer(pageLayer!)
		mapVC.animateToPosition(MaplyCoordinateMakeWithDegrees(-122.290,37.7793), height: 0.0005, time: 0.1)

		return true
	}

}
