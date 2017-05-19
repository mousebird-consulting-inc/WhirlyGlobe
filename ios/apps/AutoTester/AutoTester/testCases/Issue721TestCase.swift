//
//  Issue721TestCase.swift
//  AutoTester
//
//  Created by jmnavarro on 24/10/16.
//  Copyright © 2016-2017 mousebird consulting. All rights reserved.
//
import Foundation

class Issue721TestCase : MaplyTestCase{

	override init() {
		super.init()

		self.name = "Issue 721: Interaction lost bug"
		self.captureDelay = 4
		self.implementations = [.globe]
	}

	override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
		let baseLayer = StamenWatercolorRemote()
		baseLayer.setUpWithGlobe(globeVC)

		DispatchQueue.main.asyncAfter(deadline: .now() + 4.5) {
			print("Changing frame...")
			UIView.animate(withDuration: 2.0) {
				globeVC.view.frame = CGRect(
					origin: CGPoint.zero,
					size: CGSize(
						width: self.testView!.bounds.width/2,
						height: self.testView!.bounds.height/2))
			}
		}
	}
}
