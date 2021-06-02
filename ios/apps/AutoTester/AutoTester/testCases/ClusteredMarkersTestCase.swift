//
//  ClusteredMarkersTestCase.swift
//  AutoTester
//
//  Created by jmnavarro on 2/11/15.
//  Copyright © 2015-2017 mousebird consulting.
//

import UIKit

class ClusteredMarkersTestCase: MaplyTestCase {

	override init() {
		super.init()

		self.name = "Clustered Markers"
		self.implementations = [.globe, .map]
	}

	override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
		baseCase.setUpWithGlobe(globeVC)
		insertClusteredMarkers(baseCase.vecList!, theBaseView: globeVC)
		globeVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(151.211111, -33.859972), time: 1.0)
        globeVC.keepNorthUp = false
	}
	
	override func setUpWithMap(_ mapVC: MaplyViewController) {
		baseCase.setUpWithMap(mapVC)
		insertClusteredMarkers(baseCase.vecList!, theBaseView: mapVC)
		mapVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(151.211111, -33.859972), time: 1.0)
        mapVC.rotateGesture = true
	}

	func insertClusteredMarkers(_ compB : NSArray, theBaseView: MaplyBaseViewController) {
		let size = CGSize(width: 48, height: 48)
		let image = UIImage(named: "marker-stroked-24@2x")
        let ptImg = UIImage(named: "circle-24@2x")
        var markers = [MaplyScreenMarker]()
        var points = [MaplyScreenMarker]()
		for object in compB {
            let c = (object as! MaplyVectorObject).center()
			let marker = MaplyScreenMarker()
			marker.image = image
			marker.loc = c
			marker.size = size
            marker.userObject = (object as! MaplyVectorObject).attributes?["title"]
            marker.selectable = true
            marker.offset = CGPoint(x: CGFloat(size.width) * CGFloat(c.x / Float.pi / 2),
                                    y: CGFloat(size.height) * CGFloat(c.y / Float.pi))
            marker.rotation = ((arc4random() % 100) < 50) ? 1.0e-8 : 0.0
			markers.append(marker)
            
            let pt = MaplyScreenMarker()
            pt.image = ptImg
            pt.loc = c
            pt.size = CGSize(width: 4, height: 4)
            pt.selectable = false
            pt.layoutImportance = Float.greatestFiniteMagnitude
            points.append(pt)
		}
        if let obj =  theBaseView.addScreenMarkers(markers, desc: [kMaplyClusterGroup: 0], mode: MaplyThreadMode.current) {
            objs.append(obj)
        }
        if let obj =  theBaseView.addScreenMarkers(points, desc: nil, mode: MaplyThreadMode.current) {
            objs.append(obj)
        }

        // Disable after a few seconds
//        DispatchQueue.main.asyncAfter(deadline:  .now() + 5.0) {
//            theBaseView.disableObjects([screenObj!], mode: MaplyThreadMode.current)
//        }
        
        // Re-enable after a few seconds
//        DispatchQueue.main.asyncAfter(deadline: .now() + 10.0) {
//            theBaseView.enable([screenObj!], mode: MaplyThreadMode.current)
//        }
	}

    override func stop() {
        baseViewController?.remove(objs)
        objs.removeAll()
        baseCase.stop()
        super.stop()
    }

    var objs = [MaplyComponentObject]()
    let baseCase = VectorsTestCase()
}
