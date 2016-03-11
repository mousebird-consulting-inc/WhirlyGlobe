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
		self.captureDelay = 20
	}
	
	override func setUpWithMap(mapVC: MaplyViewController) -> Bool {
        StamenWatercolorRemote().setUpWithMap(mapVC)
		BNGCustomMapTestCase().createBritishNationalOverlayLocal(mapVC, maplyMap: true)
        mapVC.setPosition(MaplyCoordinateMakeWithDegrees(-0.1275, 51.507222), height: 0.3)

		return true
	}
	
	override func setUpWithGlobe(globeVC: WhirlyGlobeViewController) -> Bool {
        StamenWatercolorRemote().setUpWithGlobe(globeVC)
		BNGCustomMapTestCase().createBritishNationalOverlayLocal(globeVC, maplyMap: false)
        globeVC.clearColor = UIColor.init(colorLiteralRed: 0.8, green: 0.8, blue: 0.8, alpha: 1.0)

		return true
	}

}
