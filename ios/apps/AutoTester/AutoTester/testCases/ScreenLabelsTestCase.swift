//
//  ScreenLabelsTestCase.swift
//  AutoTester
//
//  Created by jmnavarro on 30/10/15.
//  Copyright © 2015 mousebird consulting. All rights reserved.
//

import UIKit

class ScreenLabelsTestCase: MaplyTestCase {

	var labelList = [MaplyComponentObject]()
    var markerList = [MaplyComponentObject]()

	override init() {
		super.init()
		
		self.name = "Screen Labels"
		self.captureDelay = 3
		self.implementations = [.globe, .map]
	}

	override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
        globeVC.keepNorthUp = true
		let vectorTestCase = VectorsTestCase()
		vectorTestCase.setUpWithGlobe(globeVC)
		insertLabels(vectorTestCase.compList! as NSArray as! [MaplyVectorObject], theViewC: globeVC)
		globeVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(151.211111, -33.859972), time: 1.0)
	}

	override func setUpWithMap(_ mapVC: MaplyViewController) {
		let vectorTestCase = VectorsTestCase()
		vectorTestCase.setUpWithMap(mapVC)
		insertLabels(vectorTestCase.compList! as NSArray as! [MaplyVectorObject], theViewC: mapVC)
		mapVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(151.211111, -33.859972), time: 1.0)
	}

	fileprivate func insertLabels(_ arrayComp: [MaplyVectorObject], theViewC: MaplyBaseViewController) {
		
		for i in 0..<arrayComp.count {
			let object = arrayComp[i]
            let str = object.userObject as? String
            if str != nil {
				let label = MaplyScreenLabel()

				label.text = str
				label.loc = object.center()
				label.selectable = true
				label.layoutImportance = 10
                label.userObject = label.text;
                label.layoutPlacement = kMaplyLayoutRight | kMaplyLayoutLeft | kMaplyLayoutAbove | kMaplyLayoutBelow
//                label.rotation = Float(M_PI/2.0)
//                label.offset = CGPoint(x: 100.0, y: 0.0)

				if (i % 2 == 0) {
					// Some with text shadow
					if let comp = theViewC.addScreenLabels([label], desc: [
							kMaplyFont: UIFont.boldSystemFont(ofSize: 24.0),
							kMaplyShadowColor: UIColor.black,
							kMaplyShadowSize: 1.0,
                            kMaplySelectable: true,
							kMaplyTextColor: UIColor.white]) {
						labelList.append(comp)
					}
				}
				else {
					//Some with text outline
					if let comp = theViewC.addScreenLabels([label], desc: [
							kMaplyFont: UIFont.boldSystemFont(ofSize: 24.0),
							kMaplyTextOutlineColor: UIColor.black,
							kMaplyTextOutlineSize: 2.0,
                            kMaplySelectable: true,
							kMaplyTextColor: UIColor.lightGray]) {
						labelList.append(comp)
					}
				}
                
#if false
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
        
        // A multi-line test case
        let label = MaplyScreenLabel()
        label.text = "abcdef\nghijklm\nnopqr\nstuvw\nxyzAB\nCDEFG\nHIJKL\nMNOPQR\nSTUVW\nXYZ"
        label.selectable = true
        label.layoutImportance = 20
        label.userObject = label.text
        label.layoutPlacement = kMaplyLayoutCenter
        label.loc = MaplyCoordinateMake(0.0, 0.0)
        
        theViewC.addScreenLabels([label], desc: [
            kMaplyFont: UIFont.boldSystemFont(ofSize: 24.0),
            kMaplyTextOutlineColor: UIColor.red,
            kMaplyTextOutlineSize: 2.0,
            kMaplyTextJustify: kMaplyTextJustifyCenter,
            kMaplySelectable: true,
            kMaplyBackgroundColor: UIColor.purple,
            kMaplyTextColor: UIColor.lightGray])

        // Marker for reference
        let marker = MaplyScreenMarker()
        marker.loc = MaplyCoordinateMake(0.0, 0.0)
        marker.layoutImportance = MAXFLOAT
        marker.size = CGSize(width: 8.0, height: 8.0)
        theViewC.addScreenMarkers([marker], desc: [kMaplyDrawPriority: 10000000, kMaplyColor: UIColor.blue])

        // A multi-line test case
        let label2 = MaplyScreenLabel()
        label2.text = "abcdef"
        label2.selectable = true
        label2.layoutImportance = 20
        label2.userObject = label.text
        label2.layoutPlacement = kMaplyLayoutCenter
        label2.loc = MaplyCoordinateMakeWithDegrees(1.0, 0.0)
        
        theViewC.addScreenLabels([label2], desc: [
            kMaplyFont: UIFont.boldSystemFont(ofSize: 24.0),
            kMaplyTextOutlineColor: UIColor.blue,
            kMaplyTextOutlineSize: 2.0,
            kMaplyTextJustify: kMaplyTextJustifyCenter,
            kMaplySelectable: true,
            kMaplyBackgroundColor: UIColor.yellow,
            kMaplyTextColor: UIColor.lightGray])
	}
}
