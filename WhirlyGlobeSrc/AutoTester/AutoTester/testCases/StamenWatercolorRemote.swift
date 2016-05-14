//
//  MapboxSatellite.swift
//  AutoTester
//
//  Created by jmnavarro on 13/10/15.
//  Copyright © 2015 mousebird consulting. All rights reserved.
//

import UIKit

class StamenWatercolorRemote: MaplyTestCase {

	override init() {
		super.init()

		self.name = "Stamen Watercolor Remote"
		self.captureDelay = 4
	}
	
	func setupLayer(baseVC: MaplyBaseViewController) -> MaplyQuadImageTilesLayer {
		let cacheDir = NSSearchPathForDirectoriesInDomains(.CachesDirectory, .UserDomainMask, true)[0]
		
		let thisCacheDir = "\(cacheDir)/stamentiles/"
		let maxZoom = Int32(16)
		let tileSource = MaplyRemoteTileSource(
			baseURL: "http://tile.stamen.com/watercolor/",
			ext: "png", minZoom: Int32(0), maxZoom: Int32(maxZoom))
		tileSource!.cacheDir = thisCacheDir
		let layer = MaplyQuadImageTilesLayer(tileSource: tileSource!)
		layer!.handleEdges = true
		layer!.drawPriority = kMaplyImageLayerDrawPriorityDefault
		//		layer!.waitLoad = imageWaitLoad
		layer!.singleLevelLoading = false

		return layer!;
	}

	override func setUpWithGlobe(globeVC: WhirlyGlobeViewController) -> Bool {
		let layer = setupLayer(globeVC)
		
		globeVC.keepNorthUp = true
		globeVC.addLayer(layer)
		globeVC.animateToPosition(MaplyCoordinateMakeWithDegrees(-3.6704803, 40.5023056), time: 1.0)

		return true
	}

	override func setUpWithMap(mapVC: MaplyViewController) -> Bool {
		let layer = setupLayer(mapVC)

		mapVC.addLayer(layer)
		mapVC.animateToPosition(MaplyCoordinateMakeWithDegrees(-3.6704803, 40.5023056), height: 1.0, time: 1.0)
		mapVC.setZoomLimitsMin(0.01, max: 4.0)
		
		return true;
	}
	
	override func remoteResources() -> [AnyObject]? {
		return ["https://manuals.info.apple.com/en_US/macbook_retina_12_inch_early2016_essentials.pdf"]
	}
}
