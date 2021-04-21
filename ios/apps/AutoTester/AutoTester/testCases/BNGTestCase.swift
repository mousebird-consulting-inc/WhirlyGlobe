//
//  TestBNG.swift
//  AutoTester
//
//  Created by jmnavarro on 10/12/15.
//  Copyright © 2015-2021 mousebird consulting.
//

import UIKit

class BNGTestCase: MaplyTestCase {
	
	override init(){
		super.init()
		self.name = "British National Grid Tile Source"
		self.implementations = [.globe, .map]
	}

	override func setUpWithMap(_ mapVC: MaplyViewController) {
        baseCase = GeographyClassTestCase()
        bngCase = BNGCustomMapTestCase()
		baseCase?.setUpWithMap(mapVC)
		bngCase?.createBritishNationalOverlayLocal(mapVC, maplyMap: true)
		mapVC.setPosition(MaplyCoordinateMakeWithDegrees(-0.1275, 51.507222), height: 0.3)
	}
	
	override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
        baseCase = GeographyClassTestCase()
        bngCase = BNGCustomMapTestCase()
		baseCase?.setUpWithGlobe(globeVC)
		bngCase?.createBritishNationalOverlayLocal(globeVC, maplyMap: false)
        globeVC.setPosition(MaplyCoordinateMakeWithDegrees(-0.1275, 51.507222), height: 0.3)
        //globeVC.clearColor = UIColor(red: 0.8, green: 0.8, blue: 0.8, alpha: 1.0)
	}

    override func stop() {
        bngCase?.stop()
        bngCase = nil
        baseCase?.stop()
        baseCase = nil
        super.stop()
    }

    var baseCase: MaplyTestCase?
    var bngCase: BNGCustomMapTestCase?
}
