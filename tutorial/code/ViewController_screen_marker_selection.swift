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
//		addCountries()
		// add bars
		addBars()
		// ad spheres
//		addSpheres()
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

	private func addAnnotationWithTitle(title: String, subtitle: String, loc:MaplyCoordinate) {
		theViewC?.clearAnnotations()

		let a = MaplyAnnotation()
		a.title = title
		a.subTitle = subtitle

		theViewC?.addAnnotation(a, forPoint: loc, offset: CGPointZero)
	}

	private func addBars() {
		let capitals = [
			MaplyCoordinateMakeWithDegrees(-122.4192,37.7793),
			MaplyCoordinateMakeWithDegrees(-77.036667, 38.895111),
			MaplyCoordinateMakeWithDegrees(120.966667, 14.583333),
			MaplyCoordinateMakeWithDegrees(55.75, 37.616667),
			MaplyCoordinateMakeWithDegrees(-0.1275, 51.507222),
			MaplyCoordinateMakeWithDegrees(-66.916667, 10.5),
			MaplyCoordinateMakeWithDegrees(139.6917, 35.689506),
			MaplyCoordinateMakeWithDegrees(166.666667, -77.85),
			MaplyCoordinateMakeWithDegrees(-58.383333, -34.6),
			MaplyCoordinateMakeWithDegrees(-74.075833, 4.598056),
			MaplyCoordinateMakeWithDegrees(-79.516667, 8.983333)
		]

		let icon = UIImage(named: "alcohol-shop-24")

		let markers = capitals.map { cap -> MaplyScreenMarker in
			let marker = MaplyScreenMarker()
			marker.image = icon
			marker.loc = cap
			marker.size = CGSizeMake(140, 140)
			return marker
		}

		theViewC?.addScreenMarkers(markers, desc: nil)
	}

	private func addSpheres() {
		let capitals = [MaplyCoordinateMakeWithDegrees(-77.036667, 38.895111),
			MaplyCoordinateMakeWithDegrees(120.966667, 14.583333),
			MaplyCoordinateMakeWithDegrees(55.75, 37.616667),
			MaplyCoordinateMakeWithDegrees(-0.1275, 51.507222),
			MaplyCoordinateMakeWithDegrees(-66.916667, 10.5),
			MaplyCoordinateMakeWithDegrees(139.6917, 35.689506),
			MaplyCoordinateMakeWithDegrees(166.666667, -77.85),
			MaplyCoordinateMakeWithDegrees(-58.383333, -34.6),
			MaplyCoordinateMakeWithDegrees(-74.075833, 4.598056),
			MaplyCoordinateMakeWithDegrees(-79.516667, 8.983333)]

		// work through the spheres
		let spheres = capitals.map { capital -> MaplyShapeSphere in
			let sphere = MaplyShapeSphere()
			sphere.center = capital
			sphere.radius = 0.01
			return sphere
		}

		self.theViewC?.addShapes(spheres, desc: [
			kMaplyColor: UIColor(red: 0.75, green: 0.0, blue: 0.0, alpha: 0.75)])
	}

	private func handleSelection(selectedObject: NSObject) {
		if let selectedObject = selectedObject as? MaplyVectorObject {
			var loc = UnsafeMutablePointer<MaplyCoordinate>.alloc(1)
			if selectedObject.centroid(loc) {
				addAnnotationWithTitle("selected", subtitle: selectedObject.userObject as! String, loc: loc.memory)
			}
			loc.dealloc(1)
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
