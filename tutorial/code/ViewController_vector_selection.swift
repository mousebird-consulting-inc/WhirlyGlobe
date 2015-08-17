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

	private var vectorDict: [String:AnyObject]?

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

		// add the capability to use the local tiles or remote tiles
		let useLocalTiles = false

		// we'll need this layer in a second
		let layer: MaplyQuadImageTilesLayer

		if useLocalTiles {
			let tileSource = MaplyMBTileSource(MBTiles: "geography-class_medres")
			layer = MaplyQuadImageTilesLayer(coordSystem: tileSource.coordSys, tileSource: tileSource)
		}
		else {
			// Because this is a remote tile set, we'll want a cache directory
			let baseCacheDir = NSSearchPathForDirectoriesInDomains(.CachesDirectory, .UserDomainMask, true)[0] as! String
			let aerialTilesCacheDir = "\(baseCacheDir)/osmtiles/"
			let maxZoom = Int32(18)

			// MapQuest Open Aerial Tiles, Courtesy Of Mapquest
			// Portions Courtesy NASA/JPL­Caltech and U.S. Depart. of Agriculture, Farm Service Agency

			let tileSource = MaplyRemoteTileSource(
				baseURL: "http://otile1.mqcdn.com/tiles/1.0.0/sat/",
				ext: "png",
				minZoom: 0, maxZoom: maxZoom)
			layer = MaplyQuadImageTilesLayer(coordSystem: tileSource.coordSys, tileSource: tileSource)
		}

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

		vectorDict = [
			kMaplyColor: UIColor.whiteColor(),
			kMaplySelectable: true,
			kMaplyVecWidth: 4.0]

		// add the countries
		addCountries()
	}


	private func addCountries() {
		// handle this in another thread
		let queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_BACKGROUND, 0)
		dispatch_async(queue) {
			let allOutlines = NSBundle.mainBundle().pathsForResourcesOfType("geojson", inDirectory: nil) as! [String]

			for outline in allOutlines {
				if let jsonData = NSData(contentsOfFile: outline) {
					let wgVecObj = MaplyVectorObject(fromGeoJSON: jsonData)
					// the admin tag from the country outline geojson has the country name ­ save
					if let attrs = wgVecObj.attributes,
						vecName = attrs.objectForKey("ADMIN") as? NSObject {

							wgVecObj.userObject = vecName

							if count(vecName.description) > 0 {
								let label = MaplyScreenLabel()
								label.text = vecName.description
								label.loc = wgVecObj.center()
								label.selectable = true
								self.theViewC?.addScreenLabels([label],
									desc: [
										kMaplyFont: UIFont.boldSystemFontOfSize(24.0),
										kMaplyTextOutlineColor: UIColor.blackColor(),
										kMaplyTextOutlineSize: 2.0,
										kMaplyColor: UIColor.whiteColor()])
							}
					}

					// add the outline to our view
					let compObj = self.theViewC?.addVectors([wgVecObj], desc: self.vectorDict)

					// If you ever intend to remove these, keep track of the MaplyComponentObjects above.

				}
			}
		}
	}

}
