//
//  MarkersTestCase.swift
//  AutoTester
//
//  Created by jmnavarro on 2/11/15.
//  Copyright © 2015 mousebird consulting. All rights reserved.
//

import UIKit

class MarkersTestCase: MaplyTestCase {

	override init() {
		super.init()

		self.name = "Markers"
		self.captureDelay = 4
	}

	func insertMarkers (arrayComp: NSArray, theViewC: MaplyBaseViewController) {
		let size = CGSizeMake(0.05, 0.05);
		let startImage = UIImage(named: "airfield-24@2x")
		var markers = [MaplyMarker]()
		for var i=0; i < arrayComp.count; ++i{
			let object = arrayComp[i]
			let marker = MaplyMarker()
			marker.image = startImage
			marker.loc = (object as! MaplyVectorObject).center()
			marker.size = size
			marker.userObject = object.userObject
			markers.append(marker)
		}
		theViewC.addMarkers(markers, desc: nil)
	}

	override func setUpWithGlobe(globeVC: WhirlyGlobeViewController) -> Bool {
		let baseLayer  = VectorsTestCase()
		baseLayer.setUpWithGlobe(globeVC)
		insertMarkers(baseLayer.compList!, theViewC: globeVC)
		globeVC.animateToPosition(MaplyCoordinateMakeWithDegrees(151.211111, -33.859972), time: 1.0)
		return true
	}

	override func setUpWithMap(mapVC: MaplyViewController) -> Bool {
		let baseLayer = VectorsTestCase()
		baseLayer.setUpWithMap(mapVC)
		insertMarkers(baseLayer.compList!, theViewC: mapVC)
		mapVC.animateToPosition(MaplyCoordinateMakeWithDegrees(151.211111, -33.859972), time: 1.0)
		return true
	}

}
