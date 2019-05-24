//
//  MarkersTestCase.m
//  AutoTester
//
//  Created by jmnavarro on 2/11/15.
//  Copyright © 2015-2017 mousebird consulting.
//

import UIKit

class MarkersTestCase: MaplyTestCase {

	override init() {
		super.init()

		self.name = "Markers"
		self.implementations = [.globe, .map]
	}
    let startImage = UIImage(named: "airfield-24@2x")
    var compObjs: MaplyComponentObject? = nil

	func insertMarkers (_ arrayComp: NSArray, theViewC: MaplyBaseViewController) {
		let size = CGSize(width: 0.05, height: 0.05);
		var markers = [MaplyMarker]()
		for i in 0 ..< arrayComp.count {
			let object = arrayComp[i]
			let marker = MaplyMarker()
			marker.image = startImage
			marker.loc = (object as! MaplyVectorObject).center()
			marker.size = size
			marker.userObject = (object as AnyObject).userObject!
			markers.append(marker)
		}
        // Note: We have to sit on this so we don't lose our MaplyTexture.  Need to fix that.
		compObjs = theViewC.addMarkers(markers, desc: nil)
	}
    
    let baseCase = VectorsTestCase()

	override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
        baseCase.setUpWithGlobe(globeVC)
		insertMarkers(baseCase.vecList!, theViewC: globeVC)
		globeVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(151.211111, -33.859972), time: 1.0)
	}

	override func setUpWithMap(_ mapVC: MaplyViewController) {
		baseCase.setUpWithMap(mapVC)
		insertMarkers(baseCase.vecList!, theViewC: mapVC)
		mapVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(151.211111, -33.859972), time: 1.0)
	}

}
