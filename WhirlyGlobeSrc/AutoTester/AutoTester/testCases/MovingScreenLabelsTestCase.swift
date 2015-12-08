//
//  MovingScreenLabelsTestCase.swift
//  AutoTester
//
//  Created by jmnavarro on 2/11/15.
//  Copyright © 2015 mousebird consulting. All rights reserved.
//

import UIKit

class MovingScreenLabelsTestCase: MaplyTestCase {

	override init() {
		super.init()

		self.name = "Moving Screen Labels"
		self.captureDelay = 3
	}

	override func setUpWithGlobe(globeVC: WhirlyGlobeViewController) -> Bool {
		return true
	}

	override func setUpWithMap(mapVC: MaplyViewController) -> Bool {
		return true
	}

}
