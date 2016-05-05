//
//  ViewController.swift
//  HelloEarth
//
//  Created by jmnavarro on 24/07/15.
//  Copyright (c) 2015 Mousebird. All rights reserved.
//

import UIKit

class ViewController: UIViewController,
	WhirlyGlobeViewControllerDelegate,
MaplyViewControllerDelegate {

	private var theViewC: WhirlyGlobeViewController?
	private var vectorDict: [String:AnyObject]?

	private let DoOverlay = true


	override func viewDidLoad() {
		super.viewDidLoad()

		theViewC = WhirlyGlobeViewController()
		self.view.addSubview(theViewC!.view)
		theViewC!.view.frame = self.view.bounds
		addChildViewController(theViewC!)

		// we want a black background for a globe, a white background for a map.
		theViewC!.clearColor = UIColor.blackColor()

		// and thirty fps if we can get it ­ change this to 3 if you find your app is struggling
		theViewC!.frameInterval = 3

		// add the capability to use the local tiles or remote tiles
		let useLocalTiles = false

		// we'll need this layer in a second
		let layer: MaplyQuadImageTilesLayer

		if useLocalTiles {
			if let tileSource = MaplyMBTileSource(MBTiles: "geography-class_medres") {
				layer = MaplyQuadImageTilesLayer(tileSource: tileSource)
			}
		}
		else {
			// Because this is a remote tile set, we'll want a cache directory
			let baseCacheDir = NSSearchPathForDirectoriesInDomains(.CachesDirectory, .UserDomainMask, true)[0]
			let aerialTilesCacheDir = "\(baseCacheDir)/osmtiles/"

			// A set of various base layers to select from. Remember to adjust the maxZoom factor appropriately
			// http://otile1.mqcdn.com/tiles/1.0.0/sat/
			// http://map1.vis.earthdata.nasa.gov/wmts-webmerc/VIIRS_CityLights_2012/default/2015-05-07/GoogleMapsCompatible_Level8/{z}/{y}/{x} - jpg
			// http://map1.vis.earthdata.nasa.gov/wmts-webmerc/MODIS_Terra_CorrectedReflectance_TrueColor/default/2015-06-07/GoogleMapsCompatible_Level9/{z}/{y}/{x}  - jpg

			let maxZoom = Int32(18)

			// MapQuest Open Aerial Tiles, Courtesy Of Mapquest
			// Portions Courtesy NASA/JPL­Caltech and U.S. Depart. of Agriculture, Farm Service Agency

			if let tileSource = MaplyRemoteTileSource(
					baseURL: "http://otile1.mqcdn.com/tiles/1.0.0/sat/",
					ext: "jpg",
					minZoom: 0,
					maxZoom: maxZoom) {
				tileSource.cacheDir = aerialTilesCacheDir
				layer = MaplyQuadImageTilesLayer(tileSource: tileSource)
			}
		}


		layer.handleEdges = true
		layer.coverPoles = true
		layer.requireElev = false
		layer.waitLoad = false
		layer.drawPriority = 0
		layer.singleLevelLoading = false
		theViewC!.addLayer(layer)

		// start up over Santa Cruz, center of the universe's beach
		theViewC!.height = 0.06
		theViewC!.heading = 0.15
		theViewC!.tilt = 0.0         // PI/2 radians = horizon??
		theViewC!.animateToPosition(MaplyCoordinateMakeWithDegrees(-122.4192,37.7793), time: 1.0)

		// Setup a remote overlay from NASA GIBS
		if DoOverlay {
			// For network paging layers, where we'll store temp files
			let cacheDir = NSSearchPathForDirectoriesInDomains(.CachesDirectory, .UserDomainMask, true)[0]

			if let tileSource = MaplyRemoteTileSource(baseURL: "http://map1.vis.earthdata.nasa.gov/wmts-webmerc/Sea_Surface_Temp_Blended/default/2013-06-07/GoogleMapsCompatible_Level7/",
					ext: "png",
					minZoom: 1,
					maxZoom: 7) {
				tileSource.cacheDir = "\(cacheDir)/sea_temperature/"
				tileSource.tileInfo.cachedFileLifetime = 3 // invalidate OWM data after 3 secs
				if let temperatureLayer = MaplyQuadImageTilesLayer(tileSource: tileSource) {
					temperatureLayer.coverPoles = false
					temperatureLayer.handleEdges = false
					theViewC!.addLayer(temperatureLayer)
				}
			}
		}

		vectorDict = [
			kMaplyColor: UIColor.whiteColor(),
			kMaplySelectable: true,
			kMaplyVecWidth: 4.0
		]

		// add the countries
		addCountries()
	}


	private func addCountries() {
		// handle this in another thread
		let queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_BACKGROUND, 0)
		dispatch_async(queue) {
			let allOutlines = NSBundle.mainBundle().pathsForResourcesOfType("geojson", inDirectory: nil)

			for outline in allOutlines {
				if let jsonData = NSData(contentsOfFile: outline),
						wgVecObj = MaplyVectorObject(fromGeoJSON: jsonData) {
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
									kMaplyColor: UIColor.whiteColor()
								])
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
