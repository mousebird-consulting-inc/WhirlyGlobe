//
//  TestBNG.swift
//  AutoTester
//
//  Created by jmnavarro on 10/12/15.
//  Copyright © 2015-2017 mousebird consulting.
//

import UIKit

class BNGTestCase: MaplyTestCase {
	
	override init(){
		super.init()
		self.name = "British National Grid"
		self.implementations = [.globe, .map]
	}
    
    let baseCase = StamenWatercolorRemote()
    let bngCase = BNGCustomMapTestCase()
	
	override func setUpWithMap(_ mapVC: MaplyViewController) {
		baseCase.setUpWithMap(mapVC)
		bngCase.createBritishNationalOverlayLocal(mapVC, maplyMap: true)
		mapVC.setPosition(MaplyCoordinateMakeWithDegrees(-0.1275, 51.507222), height: 0.3)
	}
	
	override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
		baseCase.setUpWithGlobe(globeVC)
		bngCase.createBritishNationalOverlayLocal(globeVC, maplyMap: false)
        globeVC.clearColor = UIColor(red: 0.8, green: 0.8, blue: 0.8, alpha: 1.0)
	}

}
