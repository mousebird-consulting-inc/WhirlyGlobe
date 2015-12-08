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
	}

	override func setUpWithGlobe(globeVC: WhirlyGlobeViewController) -> Bool {
		let vectorTestCase = VectorsTestCase()
		vectorTestCase.setUpWithGlobe(globeVC)
		insertLabels(vectorTestCase.compList! as! [MaplyVectorObject], theViewC: globeVC)
		globeVC.animateToPosition(MaplyCoordinateMakeWithDegrees(151.211111, -33.859972), time: 1.0)

		return true;
	}

	override func setUpWithMap(mapVC: MaplyViewController) -> Bool {
		let vectorTestCase = VectorsTestCase()
		vectorTestCase.setUpWithMap(mapVC)
		insertLabels(vectorTestCase.compList! as! [MaplyVectorObject], theViewC: mapVC)
		mapVC.animateToPosition(MaplyCoordinateMakeWithDegrees(151.211111, -33.859972), time: 1.0)
		return true;
	}

	private func insertLabels(arrayComp: [MaplyVectorObject], theViewC: MaplyBaseViewController) {
		
		for i in 0..<arrayComp.count {
			let object = arrayComp[i]
			if object.userObject?.description.characters.count > 0{
				let label = MaplyScreenLabel()

				label.text = object.userObject?.description
				label.loc = object.center()
				label.selectable = true
				label.layoutImportance = 10

				if (i % 2 == 0) {
					// Some with text shadow
					if let comp = theViewC.addScreenLabels([label], desc: [
							kMaplyFont: UIFont.boldSystemFontOfSize(24.0),
							kMaplyShadowColor: UIColor.blackColor(),
							kMaplyShadowSize: 2.0,
							kMaplyColor: UIColor.whiteColor()]) {
						labelList.append(comp)
					}
				}
				else {
					//Some with text outline
					if let comp = theViewC.addScreenLabels([label], desc: [
							kMaplyFont: UIFont.boldSystemFontOfSize(24.0),
							kMaplyTextOutlineColor: UIColor.blackColor(),
							kMaplyTextOutlineSize: 2.0,
							kMaplyColor: UIColor.whiteColor()]) {
						labelList.append(comp)
					}
				}
			}
		}
	}
}
