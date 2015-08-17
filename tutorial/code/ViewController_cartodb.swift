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

	private var theViewC: MaplyBaseViewController?
	private var globeViewC: WhirlyGlobeViewController?
	private var mapViewC: MaplyViewController?

	private var vectorDict: [String:AnyObject]?

	private let doGlobe = !true


	override func viewDidLoad() {
		super.viewDidLoad()

		if doGlobe {
			globeViewC = WhirlyGlobeViewController()
			globeViewC!.delegate = self
			theViewC = globeViewC
		}
		else {
			mapViewC = MaplyViewController()
			mapViewC!.delegate = self
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

		// start up over New York
		if let globeViewC = globeViewC {
			globeViewC.height = 0.0002
			globeViewC.animateToPosition(MaplyCoordinateMakeWithDegrees(-73.99, 40.75), time: 1.0)
		}
		else if let mapViewC = mapViewC {
			mapViewC.height = 0.0002
			mapViewC.animateToPosition(MaplyCoordinateMakeWithDegrees(-73.99, 40.75), time: 1.0)
		}


		vectorDict = [
			kMaplyColor: UIColor.whiteColor(),
			kMaplySelectable: true,
			kMaplyVecWidth: 4.0]

		// add buildings
		addBuildings()
	}


	private func addAnnotationWithTitle(title: String, subtitle: String, loc:MaplyCoordinate) {
		theViewC?.clearAnnotations()

		let a = MaplyAnnotation()
		a.title = title
		a.subTitle = subtitle

		theViewC?.addAnnotation(a, forPoint: loc, offset: CGPointZero)
	}

	private func addBuildings() {
		let search = "SELECT the_geom,address,ownername,numfloors FROM mn_mappluto_13v1 WHERE the_geom && ST_SetSRID(ST_MakeBox2D(ST_Point(%f, %f), ST_Point(%f, %f)), 4326) LIMIT 2000;"

		let cartoLayer = CartoDBLayer(search: search)
		cartoLayer.setMinZoom(15);
		cartoLayer.setMaxZoom(15);
		let coordSys = MaplySphericalMercator(webStandard: ())
		let quadLayer = MaplyQuadPagingLayer(coordSystem: coordSys, delegate: cartoLayer)
		theViewC?.addLayer(quadLayer)
	}

	private func handleSelection(selectedObject: NSObject) {
		if let selectedObject = selectedObject as? MaplyVectorObject {
			var loc = UnsafeMutablePointer<MaplyCoordinate>.alloc(1)
			if selectedObject.centroid(loc) {
				addAnnotationWithTitle("selected", subtitle: selectedObject.userObject as! String, loc: loc.memory)
			}
			loc.dealloc(1)
		}
		else if let selectedObject = selectedObject as? MaplyScreenMarker {
			addAnnotationWithTitle("selected", subtitle: "marker", loc: selectedObject.loc)
		}
	}


	// These are the versions for the map
	func maplyViewController(viewC: MaplyViewController!, didTapAt coord: MaplyCoordinate) {
		let subtitle = NSString(format: "(%.2fN, %.2fE)", coord.y*57.296,coord.x*57.296) as! String
		addAnnotationWithTitle("Tap!", subtitle: subtitle, loc: coord)
	}

	func maplyViewController(viewC: MaplyViewController!, didSelect selectedObj: NSObject!) {
		handleSelection(selectedObj)
	}

	// These are the versions for the globe
	func globeViewController(viewC: WhirlyGlobeViewController!, didTapAt coord: MaplyCoordinate) {
		let subtitle = NSString(format: "(%.2fN, %.2fE)", coord.y*57.296,coord.x*57.296) as! String
		addAnnotationWithTitle("Tap!", subtitle: subtitle, loc: coord)
	}

	func globeViewController(viewC: WhirlyGlobeViewController!, didSelect selectedObj: NSObject!) {
		handleSelection(selectedObj)
	}
	
}
