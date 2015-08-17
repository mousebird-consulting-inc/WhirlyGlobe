//
//  ViewController.swift
//  HelloEarth
//
//  Created by jmnavarro on 24/07/15.
//  Copyright (c) 2015 Mousebird. All rights reserved.
//

import UIKit

class ViewController: UIViewController {

	private var theViewC: MaplyBaseViewController?
	private var globeViewC: WhirlyGlobeViewController?
	private var mapViewC: MaplyViewController?

	private let doGlobe = !true


	override func viewDidLoad() {
		super.viewDidLoad()

		if doGlobe {
			globeViewC = WhirlyGlobeViewController()
			theViewC = globeViewC
		}
		else {
			mapViewC = MaplyViewController()
			theViewC = mapViewC
		}

		self.view.addSubview(theViewC!.view)
		theViewC!.view.frame = self.view.bounds
		addChildViewController(theViewC!)

		// we want a black background for a globe, a white background for a map.
		theViewC!.clearColor = (globeViewC != nil) ? UIColor.blackColor() : UIColor.whiteColor()

		// and thirty fps if we can get it ­ change this to 3 if you find your app is struggling
		theViewC!.frameInterval = 2

		// set up the data source
		let tileSource = MaplyMBTileSource(MBTiles: "geography-class_medres")

		// set up the layer
		let layer = MaplyQuadImageTilesLayer(coordSystem: tileSource.coordSys, tileSource: tileSource)

		layer.handleEdges = (globeViewC != nil)
		layer.coverPoles = (globeViewC != nil)
		layer.requireElev = false
		layer.waitLoad = false
		layer.drawPriority = 0
		layer.singleLevelLoading = false
		theViewC!.addLayer(layer)

		// start up over Madrid, center of the old-world
		if let globeViewC = globeViewC {
			globeViewC.height = 0.8
			globeViewC.animateToPosition(MaplyCoordinateMakeWithDegrees(-3.6704803,40.5023056), time: 1.0)
		}
		else if let mapViewC = mapViewC {
			mapViewC.height = 1.0
			mapViewC.animateToPosition(MaplyCoordinateMakeWithDegrees(-3.6704803,40.5023056), time: 1.0)
		}
		
	}
	
}
