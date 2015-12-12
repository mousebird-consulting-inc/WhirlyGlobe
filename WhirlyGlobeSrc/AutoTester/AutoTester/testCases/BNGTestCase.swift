//
//  TestBNG.swift
//  AutoTester
//
//  Created by jmnavarro on 10/12/15.
//  Copyright © 2015 mousebird consulting. All rights reserved.
//

import UIKit

class BNGTestCase: MaplyTestCase {
	
	override init(){
		super.init()
		self.name = "British National Grid"
		self.captureDelay = 3
	}
	
	override func setUpWithMap(mapVC: MaplyViewController) -> Bool {
		GeographyClassTestCase().setUpWithMap(mapVC)
		BNGCustomMapTestCase().createBritishNationalOverlayLocal(mapVC, maplyMap: true)

		return true
	}
	
	override func setUpWithGlobe(globeVC: WhirlyGlobeViewController) -> Bool {
		GeographyClassTestCase().setUpWithGlobe(globeVC)
		BNGCustomMapTestCase().createBritishNationalOverlayLocal(globeVC, maplyMap: false)

		return true
	}

}
