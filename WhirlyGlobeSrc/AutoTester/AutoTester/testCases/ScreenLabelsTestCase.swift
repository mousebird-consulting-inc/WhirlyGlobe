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
			if (object.userObject as AnyObject).description.characters.count > 0 {
				let label = MaplyScreenLabel()

				label.text = (object.userObject as AnyObject).description
				label.loc = object.center()
				label.selectable = true
				label.layoutImportance = 10
                label.userObject = label.text;
                label.layoutPlacement = kMaplyLayoutRight;
//                label.rotation = Float(M_PI/2.0);
//                label.offset = CGPointMake(0.0,100.0);

				if (i % 2 == 0) {
					// Some with text shadow
					if let comp = theViewC.addScreenLabels([label], desc: [
							kMaplyFont: UIFont.boldSystemFont(ofSize: 24.0),
							kMaplyShadowColor: UIColor.black,
							kMaplyShadowSize: 2.0,
                            kMaplySelectable: true,
							kMaplyColor: UIColor.white]) {
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
							kMaplyColor: UIColor.white]) {
						labelList.append(comp)
					}
				}
			}
		}
	}
}
