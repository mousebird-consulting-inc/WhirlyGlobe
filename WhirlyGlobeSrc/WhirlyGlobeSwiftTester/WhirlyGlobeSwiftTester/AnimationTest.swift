//
//  AnimationTest.swift
//  WhirlyGlobeSwiftTester
//
//  Created by jmnavarro on 29/09/15.
//  Copyright (c) 2015 Mousebird. All rights reserved.
//

import Foundation

/** The animation test object runs a sphere around the globe
over a defined time period.
*/
class AnimatedSphere: MaplyActiveObject {

	private let start: NSTimeInterval
	private let period: Float
	private let radius: Float
	private let color: UIColor
	private var sphereObj: MaplyComponentObject?


	/// Initialize with period (amount of time for one orbit), radius and color of the sphere and a starting point
	init(period: Float, radius: Float, color: UIColor, viewC: MaplyBaseViewController) {
		self.period = period
		self.radius = radius
		self.color = color

		start = CFAbsoluteTimeGetCurrent()

		super.init(viewController: viewC)
	}

	var hasUpdate: Bool {
		return true
	}

	func updateForFrame(frameInfo: AnyObject?) {
		if let sphereObj = sphereObj {
			viewC?.removeObjects([sphereObj], mode: .Current)
			self.sphereObj = nil
		}

		let t = (CFAbsoluteTimeGetCurrent()-start)/Double(period)

		let center = MaplyCoordinateMakeWithDegrees(Float(-180+t*360.0), Float(0.0))

		let sphere = MaplyShapeSphere()
		sphere.radius = radius
		sphere.center = center

		// Here's the trick, we must use MaplyThreadCurrent to make this happen right now
		sphereObj = viewC!.addShapes([sphere], desc: [kMaplyColor: color], mode: .Current)
	}

	func shutdown() {
		if let sphereObj = sphereObj {
			viewC?.removeObjects([sphereObj], mode: .Current)
			self.sphereObj = nil
		}
	}

}
