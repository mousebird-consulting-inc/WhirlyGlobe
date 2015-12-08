//
//  ClusteredMarkersTestCase.swift
//  AutoTester
//
//  Created by jmnavarro on 2/11/15.
//  Copyright © 2015 mousebird consulting. All rights reserved.
//

import UIKit

class ClusteredMarkersTestCase: MaplyTestCase {

	override init() {
		super.init()

		self.name = "Clustered Markers"
		self.captureDelay = 3
	}

	override func setUpWithGlobe(globeVC: WhirlyGlobeViewController) -> Bool {
		let baseLayer = VectorsTestCase()
		baseLayer.setUpWithGlobe(globeVC)
		insertClusteredMarkers(baseLayer.compList! as! [MaplyVectorObject], theBaseView: globeVC)
		globeVC.animateToPosition(MaplyCoordinateMakeWithDegrees(151.211111, -33.859972), time: 1.0)
		return true
	}
	
	override func setUpWithMap(mapVC: MaplyViewController) -> Bool {
		let baseLayer = VectorsTestCase()
		baseLayer.setUpWithMap(mapVC)
		insertClusteredMarkers(baseLayer.compList!, theBaseView: mapVC)
		mapVC.animateToPosition(MaplyCoordinateMakeWithDegrees(151.211111, -33.859972), time: 1.0)

		return true
	}

	func insertClusteredMarkers(compB : NSArray, theBaseView: MaplyBaseViewController) {
		let size = CGSizeMake(32, 32)
		let image = UIImage (named: "alcohol-shop-24@2x")
		var markers = [MaplyScreenMarker]()
		for object in compB {
			let marker = MaplyScreenMarker()
			marker.image = image
			marker.loc = (object as! MaplyVectorObject).center()
			marker.size = size
			marker.userObject = object.userObject
			markers.append(marker)
		}
		theBaseView.addScreenMarkers(markers, desc: [kMaplyClusterGroup: 0], mode: MaplyThreadMode.Current)
	}

}
