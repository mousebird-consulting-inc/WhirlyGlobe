//
//  Issue721TestCase.swift
//  AutoTester
//
//  Created by jmnavarro on 24/10/16.
//  Copyright © 2016-2017 mousebird consulting.
//
import Foundation

class Issue721TestCase : MaplyTestCase{

	override init() {
        super.init(name: "Issue 721: Interaction lost bug", supporting: [.globe])
	}

	override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
		baseCase.setUpWithGlobe(globeVC)

		DispatchQueue.main.asyncAfter(deadline: .now() + 4.5) {
			print("Changing frame...")
			UIView.animate(withDuration: 2.0) {
				globeVC.view.frame = CGRect(
					origin: CGPoint.zero,
					size: CGSize(
						width: self.globeViewController!.view!.bounds.width/2,
						height: self.globeViewController!.view!.bounds.height/2))
			}
		}
	}

    override func stop() {
        baseCase.stop()
    }

    private let baseCase = StamenWatercolorRemote()
}
