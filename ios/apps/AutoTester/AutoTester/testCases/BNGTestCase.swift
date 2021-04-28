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
        baseCase.setUpWithMap(mapVC)
        bngCase.setUpWithMap(mapVC)
	}
	
	override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
        baseCase.setUpWithGlobe(globeVC)

        bngCase.createBritishNationalOverlayLocal(globeVC)

        let bound = bngCase.geoBound(BNGCustomMapTestCase.buildBritishNationalGrid(false))
        let middle = MaplyCoordinate(x: (bound.ll.x + bound.ur.x) / 2.0,
                                     y: (bound.ll.y + bound.ur.y) / 2.0)
        let h = globeVC.findHeight(toViewBounds: bound, pos: middle)
        globeVC.setPosition(middle, height: h/3)
        globeVC.animate(toPosition: middle, height: h, heading: 0, time: 1)
	}

    override func stop() {
        bngCase.stop()
        baseCase.stop()
        super.stop()
    }

    var baseCase = GeographyClassTestCase()
    var bngCase = BNGCustomMapTestCase()
}
