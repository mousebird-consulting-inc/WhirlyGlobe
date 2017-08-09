//
//  ClusteredMarkersTestCase.swift
//  AutoTester
//
//  Created by jmnavarro on 2/11/15.
//  Copyright © 2015-2017 mousebird consulting. All rights reserved.
//

import UIKit

class ClusteredMarkersTestCase: MaplyTestCase {

	override init() {
		super.init()

		self.name = "Clustered Markers"
		self.captureDelay = 3
		self.implementations = [.globe, .map]
	}

	override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
		let baseLayer = VectorsTestCase()
		baseLayer.setUpWithGlobe(globeVC)
		insertClusteredMarkers(baseLayer.compList!, theBaseView: globeVC)
		globeVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(151.211111, -33.859972), time: 1.0)
	}
	
	override func setUpWithMap(_ mapVC: MaplyViewController) {
		let baseLayer = VectorsTestCase()
		baseLayer.setUpWithMap(mapVC)
		insertClusteredMarkers(baseLayer.compList!, theBaseView: mapVC)
		mapVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(151.211111, -33.859972), time: 1.0)
	}

	func insertClusteredMarkers(_ compB : NSArray, theBaseView: MaplyBaseViewController) {
		let size = CGSize(width: 32, height: 32)
		let image = UIImage (named: "alcohol-shop-24@2x")
		var markers = [MaplyScreenMarker]()
		for object in compB {
			let marker = MaplyScreenMarker()
			marker.image = image
			marker.loc = (object as! MaplyVectorObject).center()
			marker.size = size
			marker.userObject = (object as AnyObject).userObject!
			markers.append(marker)
		}
		let screenObj =  theBaseView.addScreenMarkers(markers, desc: [kMaplyClusterGroup: 0], mode: MaplyThreadMode.current)

        // Disable after a few seconds
        DispatchQueue.main.asyncAfter(deadline:  .now() + 5.0) {
            theBaseView.disableObjects([screenObj!], mode: MaplyThreadMode.current)
        }
        
        // Re-enable after a few seconds
        DispatchQueue.main.asyncAfter(deadline: .now() + 10.0) {
            theBaseView.enable([screenObj!], mode: MaplyThreadMode.current)
        }
        
	}

}
