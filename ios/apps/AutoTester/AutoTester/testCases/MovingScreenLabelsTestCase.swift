//
//  MovingScreenLabelsTestCase.swift
//  AutoTester
//
//  Created by jmnavarro on 2/11/15.
//  Copyright © 2015-2017 mousebird consulting. All rights reserved.
//

import UIKit

class MovingScreenLabelsTestCase: MaplyTestCase {

	override init() {
		super.init()

		self.name = "Moving Screen Labels"
		self.captureDelay = 3
		self.implementations = []
	}

	override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
	}

	override func setUpWithMap(_ mapVC: MaplyViewController) {
	}

}
