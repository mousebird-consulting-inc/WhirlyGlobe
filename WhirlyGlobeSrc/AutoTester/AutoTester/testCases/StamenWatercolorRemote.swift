//
//  MapboxSatellite.swift
//  AutoTester
//
//  Created by jmWork on 13/10/15.
//  Copyright © 2015 mousebird consulting. All rights reserved.
//

import UIKit

class StamenWatercolorRemote: MaplyTestCase {

	override init() {
		super.init()

		self.name = "Stamen Watercolor Remote"
		self.captureDelay = 2
	}

	override func setUp() {
		self.globeViewController = WhirlyGlobeViewController()
		self.testView.addSubview(self.globeViewController.view)
		self.globeViewController.view.frame = self.testView.bounds

		self.globeViewController.clearColor = UIColor.blackColor()

		self.globeViewController.frameInterval = 2

		let cacheDir = NSSearchPathForDirectoriesInDomains(.CachesDirectory, .UserDomainMask, true)[0]

		let thisCacheDir = "\(cacheDir)/stamentiles/"
		let maxZoom = Int32(16)
		let tileSource = MaplyRemoteTileSource(
			baseURL: "http://tile.stamen.com/watercolor/",
			ext: "png", minZoom: Int32(0), maxZoom: Int32(maxZoom))
		tileSource!.cacheDir = thisCacheDir
		let layer = MaplyQuadImageTilesLayer(tileSource: tileSource!)
		layer!.handleEdges = true
		layer!.requireElev = false
		layer!.drawPriority = kMaplyImageLayerDrawPriorityDefault
//		layer!.waitLoad = imageWaitLoad
		layer!.singleLevelLoading = false

		self.globeViewController.heading = 0.0

		self.globeViewController.addLayer(layer!)
		self.globeViewController.animateToPosition(MaplyCoordinateMakeWithDegrees(-3.6704803, 40.5023056), time: 1.0)
	}

}
