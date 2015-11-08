//
//  ModelsTestCase.swift
//  AutoTester
//
//  Created by jmnavarro on 2/11/15.
//  Copyright © 2015 mousebird consulting. All rights reserved.
//

import UIKit


struct LocationInfo {
	var name: String
	var lat: Float
	var lon: Float
}

let locations = [
	LocationInfo (name: "Kansas City", lat: 39.1, lon: -94.58),
	LocationInfo (name: "Washington, DC", lat: 38.895111, lon: -77.036667),
	LocationInfo (name: "Manila", lat: 14.583333, lon: 120.966667),
	LocationInfo (name: "Moscow", lat: 55.75, lon: 37.616667),
	LocationInfo (name: "London", lat: 51.507222, lon: -0.1275),
	LocationInfo (name: "Caracas", lat: 10.5, lon: -66.916667),
	LocationInfo (name: "Lagos", lat: 6.453056, lon: 3.395833),
	LocationInfo (name: "Sydney", lat: -33.859972, lon: 151.211111),
	LocationInfo (name: "Seattle", lat: 47.609722, lon: -122.333056)
]

class ModelsTestCase: MaplyTestCase {

	override init() {
		super.init()
		
		self.name = "Models"
		self.captureDelay = 4
	}

	override func setUpWithGlobe(globeVC: WhirlyGlobeViewController) -> Bool {
		let baseLayer = VectorsTestCase()
		baseLayer.setUpWithGlobe(globeVC)

		let fullPath = NSBundle.mainBundle().pathForResource("cessna", ofType: "obj")
		if let fullPath = fullPath {
			let model = MaplyGeomModel(obj: fullPath)
			if let model = model {
				var modelInstances = [MaplyMovingGeomModelInstance]()
				let scaletMat = MaplyMatrix(scale: 1000.0/6371000.0)
				let rotMat = MaplyMatrix(angle: M_PI/2.0, axisX: 1.0, axisY: 0.0, axisZ: 0.0)
				let localMat = rotMat.multiplyWith(scaletMat)
				for loc in locations {
					let mInst = MaplyMovingGeomModelInstance()
					mInst.model = model
					mInst.transform = localMat
					let loc2d = MaplyCoordinateMakeWithDegrees(loc.lon, loc.lat)
					mInst.center = MaplyCoordinate3dMake(loc2d.x, loc2d.y, 10000)
					mInst.endCenter = MaplyCoordinate3dMake(loc2d.x+0.1, loc2d.y+0.1, 10000)
					mInst.duration = 100.0
					mInst.selectable = true
					modelInstances.append(mInst)

				}
				globeVC.addModelInstances(modelInstances, desc: [:], mode: MaplyThreadMode.Current)
				globeVC.animateToPosition(MaplyCoordinateMakeWithDegrees(-94.58, 39.1) ,time: 1.0)
				globeVC.height = 0.1
			}
		}
		return true
	}

}
