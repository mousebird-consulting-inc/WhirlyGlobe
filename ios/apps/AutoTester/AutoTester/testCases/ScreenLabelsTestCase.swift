//
//  ScreenLabelsTestCase.swift
//  AutoTester
//
//  Created by jmnavarro on 30/10/15.
//  Copyright © 2015-2017 mousebird consulting.
//

import UIKit

class ScreenLabelsTestCase: MaplyTestCase {

	var labelList = [MaplyComponentObject]()
    var markerList = [MaplyComponentObject]()
    var baseCase = VectorsTestCase()

	override init() {
		super.init()
		
		self.name = "Screen Labels"
		self.implementations = [.globe, .map]
	}

	override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
        globeVC.keepNorthUp = true
		baseCase.setUpWithGlobe(globeVC)
		insertLabels(baseCase.vecList! as NSArray as! [MaplyVectorObject], theViewC: globeVC)
//        globeVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(151.211111, -33.859972), time: 1.0)
        globeVC.height = 1.5
        globeVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(45.0, -20.0), time: 1.0)
	}

	override func setUpWithMap(_ mapVC: MaplyViewController) {
		baseCase.setUpWithMap(mapVC)
		insertLabels(baseCase.vecList! as NSArray as! [MaplyVectorObject], theViewC: mapVC)
		mapVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(151.211111, -33.859972), time: 1.0)
	}

	fileprivate func insertLabels(_ arrayComp: [MaplyVectorObject], theViewC: MaplyBaseViewController) {
		
		for i in 0..<arrayComp.count {
			let object = arrayComp[i]
            let str = object.attributes?["title"] as? String
            if str != nil {
				let label = MaplyScreenLabel()

				label.text = str
				label.loc = object.center()
				label.selectable = true
                label.userObject = label.text;
                label.layoutPlacement = kMaplyLayoutCenter
//                label.rotation = Float(M_PI/2.0)
//                label.offset = CGPoint(x: 100.0, y: 0.0)

				if (i % 2 == 0) {
                    label.layoutImportance = 10
					// Some with text shadow
                    if let comp = theViewC.addScreenLabels([label], desc: [
                            kMaplyFont: UIFont.boldSystemFont(ofSize: 12.0),
                            kMaplyShadowColor: UIColor.black,
                            kMaplyShadowSize: 1.0,
                            kMaplySelectable: true,
                            kMaplyTextColor: UIColor.red,
                            kMaplyMinVis: 0.1,
                            kMaplyMaxVis: 0.5,
                            kMaplyBackgroundColor: UIColor.blue]) {
                        labelList.append(comp)
                    }
				}
				else {
                    label.layoutImportance = 20
                    label.layoutPlacement = kMaplyLayoutBelow
					//Some with text outline
                    if let comp = theViewC.addScreenLabels([label], desc: [
                            kMaplyFont: UIFont.boldSystemFont(ofSize: 24.0),
                            kMaplyTextOutlineColor: UIColor.black,
                            kMaplyTextOutlineSize: 2.0,
                            kMaplySelectable: true,
                            kMaplyTextColor: UIColor.green,
                            kMaplyBackgroundColor: UIColor.red]) {
                        labelList.append(comp)
                    }
				}
                
#if true
                // Marker for reference
                let marker = MaplyScreenMarker()
                marker.loc = object.center()
                marker.layoutImportance = MAXFLOAT
                marker.size = CGSize(width: 4.0, height: 4.0)
                if let comp = theViewC.addScreenMarkers([marker], desc: [kMaplyDrawPriority: 10000000])
                {
                    markerList.append(comp)
                }
#endif
			}
		}
        
        let layoutTest = kMaplyLayoutCenter
        
        // A multi-line test case
        let label = MaplyScreenLabel()
        label.text = "Multi-line labels\nare now\navailable in\nWhirlyGlobe-Maply."
        label.selectable = true
        label.layoutImportance = 30
        label.userObject = label.text
        label.layoutPlacement = layoutTest
        label.loc = MaplyCoordinateMakeWithDegrees(0.0, 5.0)
        
        theViewC.addScreenLabels([label], desc: [
            kMaplyFont: UIFont.boldSystemFont(ofSize: 24.0),
            kMaplyTextOutlineColor: UIColor.red,
            kMaplyTextOutlineSize: 2.0,
            kMaplySelectable: true,
//            kMaplyBackgroundColor: UIColor.purple,
            kMaplyTextColor: UIColor.lightGray])

        // Marker for reference
        let marker = MaplyScreenMarker()
        marker.loc = MaplyCoordinateMakeWithDegrees(0.0, 5.0)
        marker.layoutImportance = MAXFLOAT
        marker.size = CGSize(width: 8.0, height: 8.0)
        theViewC.addScreenMarkers([marker], desc: [kMaplyDrawPriority: 10000000, kMaplyColor: UIColor.blue])

        // A multi-line test case
        let label2 = MaplyScreenLabel()
        label2.text = "And now\nyou can\njustify text."
        label2.selectable = true
        label2.layoutImportance = 25
        label2.userObject = label.text
        label2.layoutPlacement = layoutTest
        label2.loc = MaplyCoordinateMakeWithDegrees(3.0, 5.0)
        
        theViewC.addScreenLabels([label2], desc: [
            kMaplyFont: UIFont.boldSystemFont(ofSize: 24.0),
            kMaplyTextOutlineColor: UIColor.blue,
            kMaplyTextOutlineSize: 2.0,
            kMaplySelectable: true,
//            kMaplyBackgroundColor: UIColor.yellow,
            kMaplyTextColor: UIColor.lightGray])
    
    // A multi-line test case
    let label3 = MaplyScreenLabel()
    label3.text = "Android too!"
    label3.selectable = true
    label3.layoutImportance = 20
    label3.userObject = label.text
    label3.layoutPlacement = layoutTest
    label3.loc = MaplyCoordinateMakeWithDegrees(1.5, 5.0-0.75)
    // Make this really tall and skinny as a test case
//    label3.layoutSize = CGSize(width: 1.0, height: 200.0)
    
    let marker2 = MaplyScreenMarker()
    marker2.loc = MaplyCoordinateMakeWithDegrees(1.5, 5.0-0.75)
    marker2.layoutImportance = MAXFLOAT
    marker2.size = CGSize(width: 8.0, height: 8.0)
    theViewC.addScreenMarkers([marker2], desc: [kMaplyDrawPriority: 10000000, kMaplyColor: UIColor.blue])

    theViewC.addScreenLabels([label3], desc: [
    kMaplyFont: UIFont.boldSystemFont(ofSize: 24.0),
    kMaplyTextOutlineColor: UIColor.black,
    kMaplyTextOutlineSize: 2.0,
    kMaplySelectable: true,
//    kMaplyBackgroundColor: UIColor.yellow,
    kMaplyTextColor: UIColor.lightGray])
}

}
