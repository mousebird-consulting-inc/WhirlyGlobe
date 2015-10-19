//
//  TestViewController.swift
//  WhirlyGlobeSwiftTester
//
//  Created by jmnavarro on 14/09/15.
//  Copyright (c) 2015 Mousebird. All rights reserved.
//

import UIKit


enum MapType {
	case MaplyGlobe(elevation: Bool)
	case Maply3DMap
	case Maply2DMap

	static func numTypes() -> Int {
		return 4
	}

	var name: String {
		switch self {
		case .MaplyGlobe(let elevation):
			return elevation ? "Globe w/ Elevation (3D)" : "Globe (3D)"
		case .Maply3DMap:
			return "Map (3D)"
		case .Maply2DMap:
			return "Map (2D)"
		}
	}
}

func ==(a: MapType, b: MapType) -> Bool {
	switch (a, b) {
	case (.MaplyGlobe(let elevA), .MaplyGlobe(let elevB))
	where elevA == elevB:
		return true

	case (.Maply3DMap, .Maply3DMap):
		return true

	case (.Maply2DMap, .Maply2DMap):
		return true

	default:
		return false
	}
}

func !=(a: MapType, b: MapType) -> Bool {
	return !(a == b)
}



//typealias LocationInfo = (name: String, lat: Float, lon: Float)
struct LocationInfo {
	let name: String
	let lat: Float
	let lon: Float
}


let locations: [LocationInfo] = [
	LocationInfo(name: "Kansas City",     lat: 39.1,       lon: -94.58),
	LocationInfo(name: "Washington, DC",  lat: 38.895111,  lon: -77.036667),
	LocationInfo(name: "Manila",          lat: 14.583333,  lon: 120.966667),
	LocationInfo(name: "Moscow",          lat: 55.75,      lon: 37.616667),
	LocationInfo(name: "London",          lat: 51.507222,  lon: -0.1275),
	LocationInfo(name: "Caracas",         lat: 10.5,       lon: -66.916667),
	LocationInfo(name: "Lagos",           lat: 6.453056,   lon: 3.395833),
	LocationInfo(name: "Sydney",          lat: -33.859972, lon: 151.211111),
	LocationInfo(name: "Seattle",         lat: 47.609722,  lon: -122.333056),
	LocationInfo(name: "Tokyo",           lat: 35.689506,  lon: 139.6917),
	LocationInfo(name: "McMurdo Station", lat: -77.85,     lon: 166.666667),
	LocationInfo(name: "Tehran",          lat: 35.696111,  lon: 51.423056),
	LocationInfo(name: "Santiago",        lat: -33.45,     lon: -70.666667),
	LocationInfo(name: "Pretoria",        lat: -25.746111, lon: 28.188056),
	LocationInfo(name: "Perth",           lat: -31.952222, lon: 115.858889),
	LocationInfo(name: "Beijing",         lat: 39.913889,  lon: 116.391667),
	LocationInfo(name: "New Delhi",       lat: 28.613889,  lon: 77.208889),
	LocationInfo(name: "San Francisco",   lat: 37.7793,    lon: -122.4192),
	LocationInfo(name: "Pittsburgh",      lat: 40.441667,  lon: -80),
	LocationInfo(name: "Freetown",        lat: 8.484444,   lon: -13.234444),
	LocationInfo(name: "Windhoek",        lat: -22.57,     lon: 17.083611),
	LocationInfo(name: "Buenos Aires",    lat: -34.6,      lon: -58.383333),
	LocationInfo(name: "Zhengzhou",       lat: 34.766667,  lon: 113.65),
	LocationInfo(name: "Bergen",          lat: 60.389444,  lon: 5.33),
	LocationInfo(name: "Glasgow",         lat: 55.858,     lon: -4.259),
	LocationInfo(name: "Bogota",          lat: 4.598056,   lon: -74.075833),
	LocationInfo(name: "Haifa",           lat: 32.816667,  lon: 34.983333),
	LocationInfo(name: "Puerto Williams", lat: -54.933333, lon: -67.616667),
	LocationInfo(name: "Panama City",     lat: 8.983333,   lon: -79.516667),
	LocationInfo(name: "Niihau",          lat: 21.9,       lon: -160.166667)
]

// Countries we have geoJSON for
let countryArray = [
	"ABW", "AFG", "AGO", "AIA", "ALA", "ALB", "AND", "ARE", "ARG", "ARM", "ASM", "ATA",
	"ATF", "ATG", "AUS", "AUT", "AZE", "BDI", "BEL", "BEN", "BES", "BFA", "BGD", "BGR",
	"BHR", "BHS", "BIH", "BLM", "BLR", "BLZ", "BMU", "BOL", "BRA", "BRB", "BRN", "BTN",
	"BVT", "BWA", "CAF", "CAN", "CCK", "CHE", "CHL", "CHN", "CIV", "CMR", "COD", "COG",
	"COK", "COL", "COM", "CPV", "CRI", "CUB", "CUW", "CXR", "CYM", "CYP", "CZE", "DEU",
	"DJI", "DMA", "DNK", "DOM", "DZA", "ECU", "EGY", "ERI", "ESH", "ESP", "EST", "ETH",
	"FIN", "FJI", "FLK", "FRA", "FRO", "FSM", "GAB", "GBR", "GEO", "GGY", "GHA", "GIB",
	"GIN", "GLP", "GMB", "GNB", "GNQ", "GRC", "GRD", "GRL", "GTM", "GUF", "GUM", "GUY",
	"HKG", "HMD", "HND", "HRV", "HTI", "HUN", "IDN", "IMN", "IND", "IOT", "IRL", "IRN",
	"IRQ", "ISL", "ISR", "ITA", "JAM", "JEY", "JOR", "JPN", "KAZ", "KEN", "KGZ", "KHM",
	"KIR", "KNA", "KOR", "KWT", "LAO", "LBN", "LBR", "LBY", "LCA", "LIE", "LKA", "LSO",
	"LTU", "LUX", "LVA", "MAC", "MAF", "MAR", "MCO", "MDA", "MDG", "MDV", "MEX", "MHL",
	"MKD", "MLI", "MLT", "MMR", "MNE", "MNG", "MNP", "MOZ", "MRT", "MSR", "MTQ", "MUS",
	"MWI", "MYS", "MYT", "NAM", "NCL", "NER", "NFK", "NGA", "NIC", "NIU", "NLD", "NOR",
	"NPL", "NRU", "NZL", "OMN", "PAK", "PAN", "PCN", "PER", "PHL", "PLW", "PNG", "POL",
	"PRI", "PRK", "PRT", "PRY", "PSE", "PYF", "QAT", "REU", "ROU", "RUS", "RWA", "SAU",
	"SDN", "SEN", "SGP", "SGS", "SHN", "SJM", "SLB", "SLE", "SLV", "SMR", "SOM", "SPM",
	"SRB", "SSD", "STP", "SUR", "SVK", "SVN", "SWE", "SWZ", "SXM", "SYC", "SYR", "TCA",
	"TCD", "TGO", "THA", "TJK", "TKL", "TKM", "TLS", "TON", "TTO", "TUN", "TUR", "TUV",
	"TWN", "TZA", "UGA", "UKR", "UMI", "URY", "USA", "UZB", "VAT", "VCT", "VEN", "VGB",
	"VIR", "VNM", "VUT", "WLF", "WSM", "YEM", "ZAF", "ZMB", "ZWE"
]


let UseSunSphere = false
let UseMoonSphere = false
let EarthRadius = Double(6371000)

let CountryTextures = true

// Number of unique images to use for the mega markers
let NumMegaMarkerImages = 1000
// Number of markers to whip up for the large test case
let NumMegaMarkers = 15000


class TestViewController: UIViewController, UIPopoverControllerDelegate, WhirlyGlobeViewControllerDelegate, MaplyViewControllerDelegate {

	enum PerformanceMode {
		case High
		case Low

		var frameInterval: Int32 {
			return (self == .High)
				? 2	// 30fps
				: 3 // 20fps
		}

		var threadPerLayer: Bool {
			return (self == .High)
		}
	}

	let BaseEarthPriority = kMaplyImageLayerDrawPriorityDefault

	var mapType = MapType.Maply2DMap

	/// This is the base class shared between the MaplyViewController and the WhirlyGlobeViewController
	var baseViewC: MaplyBaseViewController?

	/// If we're displaying a globe, this is set
	var globeViewC: WhirlyGlobeViewController?

	/// If we're displaying a map, this is set
	var mapViewC: MaplyViewController?

	// Base layer
	var baseLayerName: String?
	var baseLayer: MaplyViewControllerLayer?

	// Overlay layers
	var ovlLayers = [String:MaplyViewControllerLayer]()

	// The configuration view comes up when the user taps outside the globe
	var configViewC: ConfigViewController?

	var popControl: UIPopoverController?

	var screenLabelDesc = [String:AnyObject]()
	var labelDesc = [String:AnyObject]()
	var vectorDesc = [String:AnyObject]()

	// If we're in 3D mode, how far the elevation goes
	var zoomLimit = Int32(0)
	var requireElev = false
	var imageWaitLoad = false
	var maxLayerTiles = Int32(0)

	var perfMode = PerformanceMode.Low

	// These represent a group of objects we've added to the globe.
	// This is how we track them for removal
	var screenMarkersObj: MaplyComponentObject?
	var markersObj: MaplyComponentObject?
	var shapeCylObj: MaplyComponentObject?
	var shapeSphereObj: MaplyComponentObject?
	var greatCircleObj: MaplyComponentObject?
	var arrowsObj: MaplyComponentObject?
	var modelsObj: MaplyComponentObject?
	var screenLabelsObj: MaplyComponentObject?
	var labelsObj: MaplyComponentObject?
	var autoLabels: MaplyComponentObject?
	var stickersObj: MaplyComponentObject?
	var latLonObj: MaplyComponentObject?
	var sunObj: MaplyComponentObject?
	var moonObj: MaplyComponentObject?
	var stars: MaplyStarsModel?
	var atmosObj: MaplyAtmosphere?
	var animSphere: MaplyActiveObject?

	var megaMarkersObj: MaplyComponentObject?
	var megaMarkersImages = [MaplyTexture]()

	var loftPolyDict = NSMutableDictionary()
	var vecObjects: [MaplyComponentObject]?
	var sfRoadsObjArray: [MaplyComponentObject]?
	var arcGisObj: MaplyComponentObject?


	// Paging marker test
	var markerLayer: MaplyQuadPagingLayer?
	var markerDelegate: PagingTestDelegate?

	// Dashed lines used in wide vector test
	var dashedLineTex: MaplyTexture?
	var filledLineTex: MaplyTexture?


	override func viewDidLoad() {
		// What sort of hardware are we on?
		if UIScreen.mainScreen().scale > 1.0 {
			// Retina devices tend to be better, except for
			perfMode = .High
		}

		#if (arch(i386) || arch(x86_64)) && os(iOS)
			perfMode = .High;
		#endif

		configViewC = ConfigViewController(nibName: "ConfigViewController", bundle: nil)
		configViewC!.options = .All

		createMap()

		// Settings panel
		self.navigationItem.rightBarButtonItem = UIBarButtonItem(barButtonSystemItem: .Edit, target: self, action: "showConfig")
	}


	private func createMap() {
		// Create an empty globe or map controller
		zoomLimit = 0
		requireElev = false
		maxLayerTiles = 256

		cleanUp()

		switch mapType {
		case .MaplyGlobe(_):
			globeViewC = WhirlyGlobeViewController()
			globeViewC!.delegate = self
			baseViewC = globeViewC
			maxLayerTiles = 128

		case .Maply3DMap:
			mapViewC = MaplyViewController(mapType: .Type3D)
			mapViewC!.doubleTapZoomGesture = true
			mapViewC!.twoFingerTapGesture = true
			mapViewC!.viewWrap = true
			mapViewC!.delegate = self
			baseViewC = mapViewC;

		case .Maply2DMap:
			mapViewC = MaplyViewController(mapType: .TypeFlat)
			mapViewC!.viewWrap = true
			mapViewC!.doubleTapZoomGesture = true
			mapViewC!.twoFingerTapGesture = true
			mapViewC!.delegate = self
			baseViewC = mapViewC
			configViewC?.options = .Flat
		}

		self.view.addSubview(baseViewC!.view)
		baseViewC!.view.frame = self.view.bounds
		addChildViewController(baseViewC!)

		baseViewC!.frameInterval = perfMode.frameInterval
		baseViewC!.threadPerLayer = perfMode.threadPerLayer

		if let globeViewC = globeViewC {
			globeViewC.clearColor = UIColor(white: 0.8, alpha: 1.0)

			// Limit the zoom (for sun & stars)
			globeViewC.setZoomLimitsMin(globeViewC.getZoomLimitsMin(), max: 3.0)

			// Start up over San Francisco
			globeViewC.height = 0.8
			globeViewC.animateToPosition(MaplyCoordinateMakeWithDegrees(-122.4192, 37.7793), time: 1.0)
		}
		else if let mapViewC = mapViewC {
			mapViewC.clearColor = UIColor.whiteColor()

			mapViewC.height = 1.0
			mapViewC.animateToPosition(MaplyCoordinateMakeWithDegrees(-122.4192, 37.7793), time: 1.0)
		}
//TODO Unused?
//		let cacheDir = NSSearchPathForDirectoriesInDomains(.CachesDirectory, .UserDomainMask, true)[0] as! String

		// For elevation mode, we need to do some other stuff
		if isElevated() {
			// TODO
		}

		// Force the view to load so we can get the default switch values
		configViewC?.loadValues()

		// Bring up things based on what's turned on
		self.changeMapContents()
	}

	private func cleanUp() {
		baseViewC?.view.removeFromSuperview()
		baseViewC?.removeFromParentViewController()
		baseViewC = nil
		globeViewC = nil
		mapViewC = nil
	}

	private dynamic func showConfig() {
		if UI_USER_INTERFACE_IDIOM() == .Pad {
			popControl = UIPopoverController(contentViewController: configViewC!)
			popControl?.delegate = self
			popControl?.setPopoverContentSize(CGSizeMake(400, 4.0/5.0*self.view.bounds.size.height), animated: true)
			popControl?.presentPopoverFromRect(CGRectMake(0, 0, 10, 10), inView: self.view, permittedArrowDirections: .Up, animated: true)
		}
		else {
			configViewC!.navigationItem.hidesBackButton = true
			configViewC!.navigationItem.rightBarButtonItem = UIBarButtonItem(barButtonSystemItem: .Done, target: self, action: "editDone")
			self.navigationController?.pushViewController(configViewC!, animated: true)
		}
	}

	private dynamic func editDone() {
		self.navigationController?.popToViewController(self, animated: true)
		changeMapContents()
	}

	func popoverControllerDidDismissPopover(popoverController: UIPopoverController) {
		changeMapContents()
	}

	private func changeMapContents() {
		imageWaitLoad = configViewC!.valueForSection(kMaplyTestCategoryInternal, row: kMaplyTestWaitLoad)

		setupBaseLayer(configViewC!.values[0].rows)
		if configViewC!.values.count > 1 {
			setupOverlays(configViewC!.values[1].rows)
		}

		setupObjects(kMaplyTestLabel2D, component: &screenLabelsObj, method: addScreenLabels, 4, 0)
		setupObjects(kMaplyTestLabel3D, component: &labelsObj, method: addLabels, 4, 1)
		setupObjects(kMaplyTestMarker2D, component: &screenMarkersObj, method: addScreenMarkers, 4, 2)
		setupObjects(kMaplyTestMarker3D, component: &markersObj, method: addMarkers, 4, 3)
		setupObjects(kMaplyTestSticker, component: &stickersObj, method: addStickers, 4, 2)
		setupObjects(kMaplyTestShapeCylinder, component: &shapeCylObj, method: addShapeCylinders, 4, 0)
		setupObjects(kMaplyTestShapeSphere, component: &shapeSphereObj, method: addShapeSpheres, 4, 1)
		setupObjects(kMaplyTestShapeGreatCircle, component: &greatCircleObj, method: addGreatCircles, 4, 2)
		setupObjects(kMaplyTestShapeArrows, component: &arrowsObj, method: addArrows, 4, 2)
		setupObjects(kMaplyTestModels, component: &modelsObj, method: addModels, 4, 2)
		setupObjects(kMaplyTestLatLon, component: &latLonObj, method: addLinesLon, 20, 10)

		setupObjects(kMaplyTestRoads, components: &sfRoadsObjArray) {
			self.addShapeFile("tl_2013_06075_roads")
		}
		setupObjects(kMaplyTestArcGIS, component: &arcGisObj) {
			self.addArcGISQuery("http://services.arcgis.com/OfH668nDRN7tbJh0/arcgis/rest/services/SandyNYCEvacMap/FeatureServer/0/query?WHERE=Neighbrhd=%27Rockaways%27&f=pgeojson&outSR=4326")
		}

		if configViewC!.valueForSection(kMaplyTestCategoryObjects, row: kMaplyTestStarsAndSun) {
			if stars == nil {
				self.addStars("starcatalog_orig")
				self.addSun()
			}
		}
		else {
			if let stars = stars, sunObj = sunObj, moonObj = moonObj, atmosObj = atmosObj {
				stars.removeFromViewC()
				baseViewC?.removeObject(sunObj)
				baseViewC?.removeObject(moonObj)
				atmosObj.removeFromViewC()
				self.sunObj = nil
				self.moonObj = nil
				self.stars = nil
				self.atmosObj = nil
			}
		}

		if configViewC!.valueForSection(kMaplyTestCategoryObjects, row: kMaplyTestCountry) {
			if vecObjects == nil {
				self.addCountries(countryArray, stride:1)
			}
		}
		else {
			if let vecObjects = vecObjects {
				baseViewC?.removeObjects(vecObjects)
				self.vecObjects = nil
			}
			if let autoLabels = autoLabels {
				baseViewC?.removeObject(autoLabels)
				self.autoLabels = nil
			}
		}

		if configViewC!.valueForSection(kMaplyTestCategoryObjects, row: kMaplyTestLoftedPoly) {
		}
		else {
			if loftPolyDict.count > 0 {
				baseViewC?.removeObjects(loftPolyDict.allValues)
				loftPolyDict = NSMutableDictionary()
			}
		}

		if configViewC!.valueForSection(kMaplyTestCategoryObjects, row: kMaplyTestMegaMarkers) {
			if megaMarkersObj == nil {
				self.addMegaMarkers()
			}
		}
		else {
			if let megaMarkersObj = megaMarkersObj {
				baseViewC?.removeObject(megaMarkersObj)
				baseViewC?.removeTextures(megaMarkersImages, mode: .Any)
				self.megaMarkersObj = nil
			}
		}

		if configViewC!.valueForSection(kMaplyTestCategoryObjects, row: kMaplyTestQuadMarkers) {
			if markerLayer == nil {
				self.addMarkerPagingTest()
			}
		}
		else {
			if let markerLayer = markerLayer {
				baseViewC?.removeLayer(markerLayer)
				self.markerLayer = nil
				self.markerDelegate = nil
			}
		}

		if configViewC!.valueForSection(kMaplyTestCategoryObjects, row: kMaplyTestAnimateSphere) {
			if animSphere == nil {
				self.addAnimatedSphere()
			}
		}
		else {
			if let animSphere = animSphere {
				baseViewC?.removeActiveObject(animSphere)
				self.animSphere = nil
			}
		}


		baseViewC?.performanceOutput = configViewC!.valueForSection(kMaplyTestCategoryInternal, row: kMaplyTestPerf)

		if let globeViewC = globeViewC {
			globeViewC.keepNorthUp = configViewC!.valueForSection(kMaplyTestCategoryGestures, row: kMaplyTestNorthUp)
			globeViewC.pinchGesture = configViewC!.valueForSection(kMaplyTestCategoryGestures, row: kMaplyTestPinch)
			globeViewC.rotateGesture = configViewC!.valueForSection(kMaplyTestCategoryGestures, row: kMaplyTestRotate)
		}
		else {
			if configViewC!.valueForSection(kMaplyTestCategoryGestures, row: kMaplyTestNorthUp) {
				mapViewC!.heading = 0.0
			}
			mapViewC!.pinchGesture = configViewC!.valueForSection(kMaplyTestCategoryGestures, row: kMaplyTestPinch)
			mapViewC!.rotateGesture = configViewC!.valueForSection(kMaplyTestCategoryGestures, row: kMaplyTestRotate)
		}

		// Update rendering hints
		var hintDict = [NSObject:AnyObject]()
		hintDict[kMaplyRenderHintCulling] = NSNumber(bool: configViewC!.valueForSection(kMaplyTestCategoryInternal, row: kMaplyTestCulling))
		baseViewC!.setHints(hintDict)
	}

	private func setupObjects(
		rowName: String,
		inout component: MaplyComponentObject?,
		method: (Int,Int) -> (),
		_ stride: Int,
		_ offset: Int) {

		if configViewC!.valueForSection(kMaplyTestCategoryObjects, row: rowName) {
			if component == nil {
				method(stride, offset)
			}
		}
		else {
			if component != nil {
				baseViewC?.removeObject(component!)
				component = nil
			}
		}
	}

	private func setupObjects(
		rowName: String,
		inout component: MaplyComponentObject?,
		method: () -> ()) {

		if configViewC!.valueForSection(kMaplyTestCategoryObjects, row: rowName) {
			if component == nil {
				method()
			}
		}
		else {
			if component != nil {
				baseViewC?.removeObject(component!)
				component = nil
			}
		}
	}

	private func setupObjects(
		rowName: String,
		inout components: [MaplyComponentObject]?,
		method: () -> ()) {

		if configViewC!.valueForSection(kMaplyTestCategoryObjects, row: rowName) {
			if components == nil {
				method()
			}
		}
		else {
			if components != nil {
				baseViewC?.removeObjects(components!)
				components = nil
			}
		}
	}


	// Add screen (2D) labels
	private func addScreenLabels(stride stride: Int, offset: Int) {
		var labels = [MaplyScreenLabel]()

		for var i = offset; i < locations.count; i += stride {
			let label = MaplyScreenLabel()
			label.loc = MaplyCoordinateMakeWithDegrees(locations[i].lon, locations[i].lat)
			label.text = locations[i].name
			label.layoutImportance = 2.0
			label.userObject = locations[i].name
			labels.append(label)
		}

		screenLabelsObj = baseViewC?.addScreenLabels(labels, desc: screenLabelDesc)
	}

	// Add 3D labels
	private func addLabels(stride stride: Int, offset: Int) {
		let size = CGSizeMake(0, 0.05)
		var labels = [MaplyLabel]()

		for var i = offset; i < locations.count; i += stride {
			let label = MaplyLabel()
			label.loc = MaplyCoordinateMakeWithDegrees(locations[i].lon, locations[i].lat)
			label.size = size
			label.text = locations[i].name
			label.userObject = locations[i].name
			labels.append(label)
		}

		labelsObj = baseViewC?.addLabels(labels, desc: labelDesc)
	}

	// Add screen (2D) markers at all our locations
	private func addScreenMarkers(stride: Int, offset: Int) {
		let size = CGSizeMake(40, 40)
		let pinImage = UIImage(named: "map_pin")!

		var markers = [MaplyScreenMarker]()

		for var i = offset; i < locations.count; i += stride {
			let marker = MaplyScreenMarker()
			marker.image = pinImage
			marker.loc = MaplyCoordinateMake(locations[i].lon, locations[i].lat)
			marker.size = size
			marker.userObject = locations[i].name
			marker.layoutImportance = MAXFLOAT
			markers.append(marker)
		}

		screenMarkersObj = baseViewC?.addScreenMarkers(markers,
			desc: [
				kMaplyMinVis: NSNumber(double:0.0),
				kMaplyMaxVis: NSNumber(double:1.0),
				kMaplyFade: NSNumber(double:1.0),
				kMaplyDrawPriority: NSNumber(integer:100)])
	}

	// Add 3D markers
	private func addMarkers(stride: Int, offset: Int) {
		let size = CGSizeMake(0.5, 0.5)
		let starImage = UIImage(named: "Star")!

		var markers = [MaplyMarker]()

		for var i = offset; i < locations.count; i += stride {
			let marker = MaplyMarker()
			marker.image = starImage
			marker.loc = MaplyCoordinateMake(locations[i].lon, locations[i].lat)
			marker.size = size
			marker.userObject = locations[i].name
			markers.append(marker)
		}

		markersObj = baseViewC?.addMarkers(markers, desc: nil)
	}

	private func addStickers(stride: Int, offset: Int) {
		let image = UIImage(named: "Smiley_Face_Avatar_by_PixelTwist")!

		var stickers = [MaplySticker]()

		for var i = offset; i < locations.count; i += stride {
			let sticker = MaplySticker()
			sticker.image = image
			// Stickers are sized in geographic (because they're for KML ground overlays).  Bleah.
			sticker.ll = MaplyCoordinateMakeWithDegrees(locations[i].lon, locations[i].lat)
			sticker.ur = MaplyCoordinateMakeWithDegrees(locations[i].lon + 10, locations[i].lat + 10)
			// And a random rotation
			//        sticker.rotation = 2*M_PI * drand48()
			stickers.append(sticker)
		}

		markersObj = baseViewC?.addStickers(stickers, desc: [kMaplyFade: NSNumber(double: 1.0)])
	}

	// Add cylinders
	private func addShapeCylinders(stride: Int, offset: Int) {
		var cyls = [MaplyShapeCylinder]()

		for var i = offset; i < locations.count; i += stride {
			let cyl = MaplyShapeCylinder()
			cyl.baseCenter = MaplyCoordinateMakeWithDegrees(locations[i].lon, locations[i].lat)
			cyl.radius = 0.01
			cyl.height = 0.06
			cyl.selectable = true
			cyls.append(cyl)
		}

		shapeCylObj = baseViewC?.addShapes(cyls,
			desc: [
				kMaplyColor : UIColor(red: 0.0, green: 0.0, blue: 1.0, alpha: 0.8),
				kMaplyFade: NSNumber(double: 1.0),
				kMaplyDrawPriority: NSNumber(integer: 1000)])
	}

	// Add spheres
	private func addShapeSpheres(stride: Int, offset: Int) {
		var spheres = [MaplyShapeSphere]()

		for var i = offset; i < locations.count; i += stride {
			let sphere = MaplyShapeSphere()
			sphere.center = MaplyCoordinateMakeWithDegrees(locations[i].lon, locations[i].lat)
			sphere.radius = 0.04
			sphere.selectable = true
			spheres.append(sphere)
		}

		shapeSphereObj = baseViewC?.addShapes(spheres,
			desc: [
				kMaplyColor : UIColor(red: 1.0, green: 0.0, blue: 0.0, alpha: 0.8),
				kMaplyFade: NSNumber(double: 1.0),
				kMaplyDrawPriority: NSNumber(integer: 1000)])
	}

	// Add great circles
	private func addGreatCircles(stride: Int, offset: Int) {
		var circles = [MaplyShapeGreatCircle]()

		for var i = offset; i < locations.count; i += stride {
			let loc0 = locations[i]
			let loc1 = locations[(i + 1) % locations.count]
			let greatCircle = MaplyShapeGreatCircle()
			greatCircle.startPt = MaplyCoordinateMakeWithDegrees(loc0.lon, loc0.lat)
			greatCircle.endPt = MaplyCoordinateMakeWithDegrees(loc1.lon, loc1.lat)
			greatCircle.lineWidth = 6.0
			greatCircle.selectable = true
			// This limits the height based on the length of the great circle
			let angle = greatCircle.calcAngleBetween()
			greatCircle.height = 0.3 * angle / Float(M_PI)
			circles.append(greatCircle)
		}

		greatCircleObj = baseViewC?.addShapes(circles,
			desc: [
				kMaplyColor : UIColor(red: 1.0, green: 0.1, blue: 0.0, alpha: 1.0),
				kMaplyFade: NSNumber(double: 1.0),
				kMaplyDrawPriority: NSNumber(integer: 1000)])
	}

	// Add arrows
	private func addArrows(stride: Int, offset: Int) {
		// Start out the arrow at 1m
		let size = Double(1)
		let doubleCoords = [
			-0.25 * size, -0.75 * size,
			-0.25 * size,  0.25 * size,
			-0.5  * size,  0.25 * size,
			 0.0  * size,  1.0  * size,
			 0.5  * size,  0.25 * size,
			 0.25 * size,  0.25 * size,
			 0.25 * size, -0.75 * size
		]
		let exShape = MaplyShapeExtruded(outline: doubleCoords)
		exShape.thickness = size * 1.0
		exShape.height = 0.0
		exShape.color = UIColor(red: 0.8, green: 0.25, blue: 0.25, alpha: 1.0)
		// Each shape is about 10km
		//    exShape.transform = [[MaplyMatrix alloc] initWithScale:10000*1/EarthRadius];
		exShape.scale = 1.0
		let shapeModel = MaplyGeomModel(shape: exShape)

		var arrows = [MaplyGeomModelInstance]()

		for var i = offset; i < locations.count; i += stride {
			let geomInst = MaplyGeomModelInstance()
			let coord = MaplyCoordinateMakeWithDegrees(locations[i].lon, locations[i].lat)
			geomInst.center = MaplyCoordinate3dMake(coord.x, coord.y, 10000)
			let orientMat = MaplyMatrix(yaw: 0.0, pitch: 0.0, roll: 45.0/180.0*M_PI)
			geomInst.transform = MaplyMatrix(scale: 10000*1/EarthRadius).multiplyWith(orientMat)
			geomInst.selectable = true
			geomInst.model = shapeModel
			arrows.append(geomInst)
		}

		arrowsObj = baseViewC?.addModelInstances(arrows,
			desc: [
				kMaplyColor : UIColor(red: 1.0, green: 0.1, blue: 0.0, alpha: 1.0),
				kMaplyFade: NSNumber(double: 1.0)],
			mode: .Any)
	}

	// Add models
	private func addModels(stride: Int, offset: Int) {
		// Load the model
		let fullPath = NSBundle.mainBundle().pathForResource("cessna", ofType: "obj")
		if fullPath == nil {
			return
		}
		let model = MaplyGeomModel(obj: fullPath!)
		if model == nil {
			return
		}

		var modelInstances = [MaplyMovingGeomModelInstance]()
		// We need to scale the models down to display space.  They start out in meters.
		// Note: Changes this to 1000.0/6371000.0 if you can't find the models
		let scaleMat = MaplyMatrix(scale: 1000.0/6371000.0)
		// Then we need to rotate around the X axis to get the model pointed up
		let rotMat = MaplyMatrix(angle: M_PI/2.0, axisX: 1.0, axisY: 0.0, axisZ: 0.0)
		// Combine the scale and rotation
		let localMat = rotMat.multiplyWith(scaleMat)

		for var i = offset; i < locations.count; i += stride {
			let mInst = MaplyMovingGeomModelInstance()
			mInst.model = model
			mInst.transform = localMat
			let loc2d = MaplyCoordinateMakeWithDegrees(locations[i].lon, locations[i].lat)
			// Put it 1km above the earth
			mInst.center = MaplyCoordinate3dMake(loc2d.x, loc2d.y, 10000)
			mInst.endCenter = MaplyCoordinate3dMake(loc2d.x + 0.1, loc2d.y + 0.1, 10000)
			mInst.duration = 100.0
			mInst.selectable = true
			modelInstances.append(mInst)
		}

		modelsObj = baseViewC?.addModelInstances(modelInstances,
			desc: [:],
			mode: MaplyThreadMode.Current)
	}

	private func addLinesLon(lonDelta: Int, latDelta: Int) {
		var vectors = [MaplyVectorObject]()
		let desc = [
			kMaplyColor: UIColor.blueColor(),
			kMaplySubdivType: kMaplySubdivSimple,
			kMaplySubdivEpsilon: NSNumber(double: 0.001),
			kMaplyVecWidth: NSNumber(double: 4.0)
		]

		// Longitude lines
		for var lon = Float(-180); lon < 180; lon += Float(lonDelta) {
			let coords = [
				lon, -90,
				lon, 0,
				lon, +90
			]

			vectors.append(MaplyVectorObject(lineString: coords, attributes: nil))
		}

		// Latitude lines
		for var lat = Float(-90); lat < 90; lat += Float(latDelta) {
			let coords = [
				-180, lat,
				-90, lat,
				0, lat,
				90, lat,
				180, lat,
			]

			vectors.append(MaplyVectorObject(lineString: coords, attributes: nil))
		}

		latLonObj = baseViewC?.addVectors(vectors, desc: desc)
	}

	private func addShapeFile(shapeFileName: String) {
		// Make the dashed line if it isn't already there
		if dashedLineTex == nil {
			let lineTexBuilder = MaplyLinearTextureBuilder(size: CGSizeMake(4,8))
			lineTexBuilder.setPattern([4, 4])
			lineTexBuilder.opacityFunc = .OpacitySin2
			let dashedLineImage = lineTexBuilder.makeImage()
			dashedLineTex = baseViewC?.addTexture(dashedLineImage,
				imageFormat: .ImageIntRGBA,
				wrapFlags: MaplyImageWrapY,
				mode: .Any)
		}

		if filledLineTex == nil {
			let lineTexBuilder = MaplyLinearTextureBuilder(size: CGSizeMake(3,32))
			lineTexBuilder.setPattern([32])
			lineTexBuilder.opacityFunc = .OpacitySin2
			let lineImage = lineTexBuilder.makeImage()
			filledLineTex = baseViewC?.addTexture(lineImage,
				imageFormat: .ImageIntRGBA,
				wrapFlags: MaplyImageWrapY,
				mode: .Any)
		}

		dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0)) {
			// Add the vectors at three different levels
			let vecDb = MaplyVectorDatabase(shape: shapeFileName)
			if let vecObj = vecDb.fetchAllVectors() {
				self.sfRoadsObjArray = self.addWideVectors(vecObj)
			}
		}
	}

	private func addWideVectors(vecObj: MaplyVectorObject) -> [MaplyComponentObject] {
		let color = UIColor.blueColor()
		let fade = 0.25

		let lines = baseViewC?.addVectors([vecObj],
			desc: [
				kMaplyColor: color,
				kMaplyVecWidth: NSNumber(double: 4.0),
				kMaplyFade: NSNumber(double: fade),
				kMaplyVecCentered: NSNumber(bool: true),
				kMaplyMaxVis: NSNumber(double: 10.0),
				kMaplyMinVis: NSNumber(double: 0.00032424763776361942)
			])


		let screenLines = baseViewC?.addWideVectors([vecObj],
			desc: [
				kMaplyColor: UIColor(red: 0.5, green: 0.0, blue: 0.0, alpha: 0.5),
				kMaplyFade: NSNumber(double: fade),
				kMaplyVecWidth: NSNumber(double: 3.0),
				kMaplyVecTexture: filledLineTex!,
				kMaplyWideVecCoordType: kMaplyWideVecCoordTypeScreen,
				kMaplyWideVecJoinType: kMaplyWideVecMiterJoin,
				kMaplyWideVecMiterLimit: NSNumber(double: 1.01),
				kMaplyWideVecTexRepeatLen: NSNumber(double: 8),
				kMaplyMaxVis: NSNumber(double: 0.00032424763776361942),
				kMaplyMinVis: NSNumber(double: 0.00011049506429117173)
			])

		let realLines = baseViewC?.addWideVectors([vecObj],
			desc: [
				kMaplyColor: color,
				kMaplyFade: NSNumber(double: fade),
				kMaplyVecTexture: dashedLineTex!,
				// 8m in display coordinates
				kMaplyVecWidth: NSNumber(double: 10.0/6371000.0),
				kMaplyWideVecCoordType: kMaplyWideVecCoordTypeReal,
				kMaplyWideVecJoinType: kMaplyWideVecMiterJoin,
				kMaplyWideVecMiterLimit: NSNumber(double: 1.01),
				// Repeat every 10m
				kMaplyWideVecTexRepeatLen: NSNumber(double: 10.0/6371000.0),
				kMaplyMaxVis: NSNumber(double: 0.00011049506429117173),
				kMaplyMinVis: NSNumber(double: 0.0)
			])

		// Look for some labels
		var labels = [MaplyScreenLabel]()
		for road in vecObj.splitVectors() as! [MaplyVectorObject] {
			// Note: We should get this from the view controller
			let coordSys = MaplySphericalMercator()
			let middle = road.linearMiddle(coordSys)
			let rot = road.linearMiddleRotation(coordSys)
			let attrs = road.attributes

			if let name = attrs?["FULLNAME"] as? String {
				let label = MaplyScreenLabel()
				label.loc = middle
				label.text = name
				label.layoutImportance = 1.0
				label.rotation = Float(rot + M_PI/2.0)
				label.keepUpright = true
				label.layoutImportance = Float(kMaplyLayoutBelow)
				labels.append(label)
			}
		}

		let labelObj = baseViewC?.addScreenLabels(labels,
			desc: [
				kMaplyTextOutlineSize: NSNumber(double: 1.0),
				kMaplyTextOutlineColor: UIColor.blackColor(),
				kMaplyFont: UIFont.systemFontOfSize(18.0),
				kMaplyDrawPriority: NSNumber(integer: 200)
			])

		return [lines!, screenLines!, realLines!, labelObj!]
	}

	private func addArcGISQuery(url: String) {
		let operation = AFHTTPRequestOperation(request: NSURLRequest(URL: NSURL(string: url)!))
		operation.responseSerializer = AFHTTPResponseSerializer()
		operation.setCompletionBlockWithSuccess({
			(operation, responseObject) -> Void in

			if let vecObj = MaplyVectorObject(geoJSON: responseObject as! NSData) {
				self.arcGisObj = self.baseViewC?.addVectors([vecObj],
					desc: [kMaplyColor: UIColor.redColor()])
			}

		}, failure: {
			(operation, error) -> Void in

			print("Unable to fetch ArcGIS layer: \(error)\n")

		})

		operation.start()
	}

	private func addStars(inFile: String) {
		if globeViewC == nil {
			return;
		}

		// Load the stars
		if let fileName = NSBundle.mainBundle().pathForResource(inFile, ofType: "txt") {
			stars = MaplyStarsModel(fileName: fileName)
			stars?.setImage(UIImage(named: "star_background")!)
			stars?.addToViewC(globeViewC!, date: NSDate(), desc: nil, mode: .Current)
		}
	}

	private func addSun() {
		if globeViewC == nil {
			return;
		}

		globeViewC?.clearColor = UIColor.blackColor()

		// Lighting for the sun
		let sun = MaplySun(date: NSDate())
		let sunLight = sun.makeLight()
		baseViewC?.clearLights()
		baseViewC?.addLight(sunLight)

		// And a model, because why not
		if UseSunSphere {
			let sphere = MaplyShapeSphere()
			sphere.center = sun.asPosition()
			sphere.radius = 0.2
			sphere.height = 4.0
			sunObj = globeViewC?.addShapes([sphere],
				desc: [
					kMaplyColor: UIColor.yellowColor(),
					kMaplyShader: kMaplyShaderDefaultTriNoLighting])
		}
		else {
			let bill = MaplyBillboard()
			let centerGeo = sun.asPosition()
			bill.center = MaplyCoordinate3dMake(centerGeo.x, centerGeo.y, Float(5.4*EarthRadius))
			bill.selectable = false
			bill.screenObj = MaplyScreenObject()
			let globeImage = UIImage(named: "SunImage")
			bill.screenObj?.addImage(globeImage, color: UIColor.whiteColor(), size: CGSizeMake(0.9, 0.9))
			sunObj = globeViewC?.addBillboards([bill],
				desc: [
					kMaplyBillboardOrient: kMaplyBillboardOrientEye,
					kMaplyDrawPriority: NSNumber(int: kMaplySunDrawPriorityDefault)],
				mode: .Any)
		}

		let moon = MaplyMoon(date: NSDate())
		if UseMoonSphere {
			let sphere = MaplyShapeSphere()
			//WARN moon.asPosition returns a MaplyCoordinate3d
			let coord3d = moon.asPosition()
			sphere.center = MaplyCoordinate(x: coord3d.x, y: coord3d.y)
			sphere.radius = 0.2
			sphere.height = 4.0
			moonObj = globeViewC?.addShapes([sphere],
				desc: [
					kMaplyColor: UIColor.grayColor(),
					kMaplyShader: kMaplyShaderDefaultTriNoLighting])
		}
		else {
			let bill = MaplyBillboard()
			let centerGeo = moon.asPosition()
			bill.center = MaplyCoordinate3dMake(centerGeo.x, centerGeo.y, Float(5.4*EarthRadius))
			bill.selectable = false
			bill.screenObj = MaplyScreenObject()
			let moonImage = UIImage(named: "moon")
			bill.screenObj?.addImage(moonImage,
				color: UIColor(white: CGFloat(moon.illuminatedFraction), alpha: 1.0),
				size: CGSizeMake(0.75, 0.75))
			moonObj = globeViewC?.addBillboards([bill],
				desc: [
					kMaplyBillboardOrient: kMaplyBillboardOrientEye,
					kMaplyDrawPriority: NSNumber(int: kMaplyMoonDrawPriorityDefault)],
				mode: .Any)
		}

		// And some atmosphere, because the iDevice fill rate is just too fast
		atmosObj = MaplyAtmosphere(viewC: globeViewC!)
		atmosObj?.setSunPosition(sun.getDirection())
	}

	// Add country outlines.  Pass in the names of the geoJSON files
	private func addCountries(names: [String], stride: Int) {
		let smileImage: UIImage?
		let smileTex: MaplyTexture?

		if CountryTextures {
			smileImage = UIImage(named: "Smiley_Face_Avatar_by_PixelTwist")

			smileTex = baseViewC?.addTexture(smileImage!,
				imageFormat: .ImageUShort5551,
				wrapFlags: MaplyImageWrapX, mode: .Current)
		}
		else {
			smileImage = nil
			smileTex = nil
		}

		// Parsing the JSON can take a while, so let's hand that over to another queue
		dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0)) {
			var locVecObjects = [MaplyComponentObject]()
			var locAutoLabels = [MaplyScreenLabel]()

			for (i,name) in names.enumerate() {
				if i % stride == 0 {
					let fileName = NSBundle.mainBundle().pathForResource(name, ofType: "geojson")
					if let fileName = fileName {
						let jsonData = try! NSData(contentsOfFile: fileName, options: [])
						let wgVecObj = MaplyVectorObject(fromGeoJSON: jsonData)!
						let vecName = wgVecObj.attributes?["ADMIN"] as? String
						wgVecObj.userObject = vecName
						var desc = [String:AnyObject]()
						desc[kMaplyFilled] = true
						desc[kMaplySelectable] = true

						if CountryTextures {
							desc[kMaplyVecTexture] = smileTex
							desc[kMaplyVecTextureProjection] = kMaplyProjectionScreen
							desc[kMaplyVecTexScaleX] = NSNumber(double: Double(1.0/smileImage!.size.width))
							desc[kMaplyVecTexScaleY] = NSNumber(double: Double(1.0/smileImage!.size.height))
						}

						let compObj = self.baseViewC?.addVectors([wgVecObj], desc: desc)
						let screenLabel = MaplyScreenLabel()
						// Add a label right in the middle
						let center = wgVecObj.centroid()
						if center.x != kMaplyNullCoordinate.x {
							screenLabel.loc = center
							screenLabel.layoutImportance = 1.0
							screenLabel.text = vecName
							screenLabel.userObject = screenLabel.text
							screenLabel.layoutPlacement = kMaplyLayoutRight | kMaplyLayoutAbove | kMaplyLayoutLeft | kMaplyLayoutBelow
							screenLabel.selectable = true;
							if screenLabel.text != nil {
								locAutoLabels.append(screenLabel)
							}
						}

						if let compObj = compObj {
							locVecObjects.append(compObj)
						}
					}
				}
			}

			// Keep track of the created objects
			// Note: You could lose track of the objects if you turn the countries on/off quickly
			dispatch_async(dispatch_get_main_queue()) {
				// Toss in all the labels at once, more efficient
				// Note: Debugging
				// MaplyComponentObject *autoLabelObj = [baseViewC addScreenLabels:locAutoLabels desc:
				//         @{kMaplyTextColor: [UIColor colorWithRed:0.85 green:0.85 blue:0.85 alpha:1.0],
				//           kMaplyFont: [UIFont systemFontOfSize:24.0],
				//           kMaplyTextOutlineColor: [UIColor blackColor],
				//           kMaplyTextOutlineSize: @(1.0),
				////         kMaplyShadowSize: @(1.0)
				//          } mode:MaplyThreadAny];

				self.vecObjects = locVecObjects
				// autoLabels = autoLabelObj;
			}

		}

	}

	// Make up a large number of markers and add them
	private func addMegaMarkers() {

		func randomImage() -> UIImage {
			let size = CGSizeMake(16, 16)
			UIGraphicsBeginImageContext(size)
			let ctx = UIGraphicsGetCurrentContext()

			let rect = CGRectMake(1, 1, size.width-2, size.height-2)
			CGContextAddEllipseInRect(ctx, rect)
			UIColor.whiteColor().setStroke()
			CGContextStrokePath(ctx)

			UIColor(red: CGFloat(drand48()), green: CGFloat(drand48()), blue: CGFloat(drand48()), alpha: CGFloat(1.0)).setFill()
			CGContextFillEllipseInRect(ctx, rect)

			let image = UIGraphicsGetImageFromCurrentImageContext()
			UIGraphicsEndImageContext()

			return image
		}

		dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0)) {
			// Make up a few markers
			var markerImages = [MaplyTexture]()
			for i in 0..<NumMegaMarkerImages {
				let image = randomImage()
				let tex = self.baseViewC?.addTextureToAtlas(image, mode: .Current)
				markerImages.append(tex!)
			}

			var markers = [MaplyScreenMarker]()
			for i in 0..<NumMegaMarkers {
				let marker = MaplyScreenMarker()
				marker.image = markerImages[random() % NumMegaMarkerImages]
				marker.size = CGSizeMake(16,16)
				marker.loc = MaplyCoordinateMakeWithDegrees(Float(drand48()*360-180), Float(drand48()*140-70))
				marker.layoutImportance = MAXFLOAT
				markers.append(marker)
			}

			self.megaMarkersObj = self.baseViewC?.addScreenMarkers(markers, desc: nil, mode: .Current)
			self.megaMarkersImages = markerImages
		}
	}

	private func addMarkerPagingTest() {
		markerDelegate = PagingTestDelegate()
		markerLayer = MaplyQuadPagingLayer(coordSystem: markerDelegate!.coordSys!, delegate: markerDelegate!)
		baseViewC?.addLayer(markerLayer!)
	}

	private func markerSpamRefresh() {
		markerLayer?.reload()

		if let markerLayer = markerLayer {
			dispatch_after(dispatch_time(DISPATCH_TIME_NOW, Int64(4*NSEC_PER_SEC)),
				dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0)) {
				self.markerSpamRefresh()
			}
		}
	}

	// Create an animated sphere
	private func addAnimatedSphere() {
		animSphere = AnimatedSphere(period: 20.0, radius: 0.01, color: UIColor.orangeColor(), viewC: baseViewC!)
		baseViewC!.addActiveObject(animSphere!)
	}


	// Set up the base layer depending on what they've picked.
	// Also tear down an old one
	private func setupBaseLayer(baseSettings: [String:Bool]) {
		// No fancy base layers for globe elevation
		if isElevated() {
			return
		}

		// Figure out which one we're supposed to display
		let newBaseLayerName = ConfigSection.firstSelected(baseSettings)

		// Didn't change
		if newBaseLayerName == baseLayerName {
			return
		}

		// Tear down the old layer
		if let baseLayer = baseLayer {
			baseLayerName = nil
			baseViewC?.removeLayer(baseLayer)
			self.baseLayer = nil
		}

		baseLayerName = newBaseLayerName

		// For network paging layers, where we'll store temp files
		let cacheDir = NSSearchPathForDirectoriesInDomains(.CachesDirectory, .UserDomainMask, true)[0] as! String

		// We'll pick default colors for the labels
		var screenLabelColor = UIColor.whiteColor()
		var screenLabelBackColor = UIColor.clearColor()
		var labelColor = UIColor.whiteColor()
		var labelBackColor = UIColor.clearColor()
		// And for the vectors to stand out
		var vecColor = UIColor.whiteColor()
		var vecWidth = 4.0

		var jsonTileSpec: String?
		var thisCacheDir: String?

		switch baseLayerName ?? "" {
		case kMaplyTestBlank:
			() // Nothing to see here

		case kMaplyTestGeographyClass:
			self.title = kMaplyTestGeographyClass
			// This is the Geography Class MBTiles data set from MapBox
			let tileSource = MaplyMBTileSource(MBTiles: "geography-class_medres")
			let layer = MaplyQuadImageTilesLayer(tileSource: tileSource!)
			layer?.handleEdges = (globeViewC != nil)
			layer?.coverPoles = (globeViewC != nil)
			layer?.requireElev = requireElev
			layer?.waitLoad = imageWaitLoad
			layer?.drawPriority = BaseEarthPriority

			baseLayer = layer
			baseViewC?.addLayer(layer!)

			labelColor = UIColor.blackColor()
			labelBackColor = UIColor.whiteColor()
			vecColor = UIColor(red: 0.4, green: 0.4, blue: 0.4, alpha: 1.0)

		case kMaplyTestBlueMarble:
			self.title = kMaplyTestBlueMarble
			if let globeViewC = self.globeViewC {
				let layer = globeViewC.addSphericalEarthLayerWithImageSet("lowres_wtb_info")
				baseLayer = layer
				baseLayer?.drawPriority = BaseEarthPriority
				screenLabelColor = UIColor.whiteColor()
				screenLabelBackColor = UIColor.blackColor()
				labelColor = UIColor.blackColor()
				labelBackColor = UIColor.whiteColor()
				vecColor = UIColor.whiteColor()
				vecWidth = 4.0
			}

		case kMaplyTestStamenWatercolor:
			self.title = kMaplyTestStamenWatercolor

			// These are the Stamen Watercolor tiles.
			thisCacheDir = "\(cacheDir)/stamentiles/"
			let maxZoom = (zoomLimit != 0 && zoomLimit < 16) ? zoomLimit : 16
			let tileSource = MaplyRemoteTileSource(
				baseURL: "http://tile.stamen.com/watercolor/",
				ext: "png", minZoom: Int32(0), maxZoom: Int32(maxZoom))
			tileSource!.cacheDir = thisCacheDir
			let layer = MaplyQuadImageTilesLayer(tileSource: tileSource!)
			layer!.handleEdges = true
			layer!.requireElev = requireElev
			baseViewC?.addLayer(layer!)
			layer!.drawPriority = BaseEarthPriority
			layer!.waitLoad = imageWaitLoad
			layer!.singleLevelLoading = (mapType == .Maply2DMap)
			baseLayer = layer
			screenLabelColor = UIColor.whiteColor()
			screenLabelBackColor = UIColor.whiteColor()
			labelColor = UIColor.blackColor()
			labelBackColor = UIColor.blackColor()
			vecColor = UIColor.grayColor()
			vecWidth = 4.0

		case kMaplyTestOSM:
			self.title = kMaplyTestOSM

			// This points to the OpenStreetMap tile set hosted by MapQuest (I think)
			thisCacheDir = "\(cacheDir)/osmtiles/"
			let maxZoom = (zoomLimit != 0 && zoomLimit < 18) ? zoomLimit : 18
			let tileSource = MaplyRemoteTileSource(
				baseURL: "http://otile1.mqcdn.com/tiles/1.0.0/osm/",
				ext: "png",
				minZoom: Int32(0),
				maxZoom: Int32(maxZoom))
			tileSource!.cacheDir = thisCacheDir
			let layer = MaplyQuadImageTilesLayer(tileSource: tileSource!)
			layer!.drawPriority = BaseEarthPriority
			layer!.handleEdges = true
			layer!.requireElev = requireElev
			layer!.waitLoad = imageWaitLoad
			layer!.maxTiles = maxLayerTiles
			layer!.singleLevelLoading = (mapType == .Maply2DMap)
			baseViewC?.addLayer(layer!)
			layer!.drawPriority = BaseEarthPriority
			baseLayer = layer
			screenLabelColor = UIColor.whiteColor()
			screenLabelBackColor = UIColor.whiteColor()
			labelColor = UIColor.blackColor()
			labelBackColor = UIColor.whiteColor()
			vecColor = UIColor.blackColor()
			vecWidth = 4.0

		case kMaplyTestMapBoxSat:
			self.title = kMaplyTestMapBoxSat

			jsonTileSpec = "http://a.tiles.mapbox.com/v3/examples.map-zyt2v9k2.json"
			thisCacheDir = "\(cacheDir)/mbtilessat1/"
			screenLabelColor = UIColor.whiteColor()
			screenLabelBackColor = UIColor.whiteColor()
			labelColor = UIColor.blackColor()
			labelBackColor = UIColor.whiteColor()
			vecColor = UIColor.whiteColor()
			vecWidth = 4.0

		case kMaplyTestMapBoxTerrain:
			self.title = kMaplyTestMapBoxTerrain

			jsonTileSpec = "http://a.tiles.mapbox.com/v3/examples.map-zq0f1vuc.json"
			thisCacheDir = "\(cacheDir)/mbtilesterrain1/"
			screenLabelColor = UIColor.whiteColor()
			screenLabelBackColor = UIColor.whiteColor()
			labelColor = UIColor.blackColor()
			labelBackColor = UIColor.whiteColor()
			vecColor = UIColor.blackColor()
			vecWidth = 4.0

		case kMaplyTestMapBoxRegular:
			self.title = kMaplyTestMapBoxRegular

			jsonTileSpec = "http://a.tiles.mapbox.com/v3/examples.map-zswgei2n.json"
			thisCacheDir = "\(cacheDir)/mbtilesregular1/"
			screenLabelColor = UIColor.whiteColor()
			screenLabelBackColor = UIColor.whiteColor()
			labelColor = UIColor.blackColor()
			labelBackColor = UIColor.whiteColor()
			vecColor = UIColor.blackColor()
			vecWidth = 4.0

		case kMaplyTestNightAndDay:
			self.title = kMaplyTestNightAndDay

			let minZoom = 1
			let maxZoom = 8
			let tileSource1 = MaplyRemoteTileInfo(
				baseURL: "http://map1.vis.earthdata.nasa.gov/wmts-webmerc/MODIS_Terra_CorrectedReflectance_TrueColor/default/2015-05-07/GoogleMapsCompatible_Level9/{z}/{y}/{x}",
				ext: "jpg",
				minZoom: Int32(minZoom),
				maxZoom: Int32(maxZoom))
			tileSource1.cacheDir = "\(cacheDir)/daytexture-2015-05-07/"
			let tileSource2 = MaplyRemoteTileInfo(
				baseURL: "http://map1.vis.earthdata.nasa.gov/wmts-webmerc/VIIRS_CityLights_2012/default/2015-05-07/GoogleMapsCompatible_Level8/{z}/{y}/{x}",
				ext: "jpg",
				minZoom: Int32(minZoom),
				maxZoom: Int32(maxZoom))
			tileSource2.cacheDir = "\(cacheDir)/nighttexture-2015-05-07/"

			let tileSource = MaplyMultiplexTileSource(sources: [tileSource1, tileSource2])

			let layer = MaplyQuadImageTilesLayer(
				coordSystem: tileSource1.coordSys!,
				tileSource: tileSource!)

			layer!.drawPriority = BaseEarthPriority
			layer!.handleEdges = true
			layer!.requireElev = requireElev
			layer!.waitLoad = imageWaitLoad
			layer!.maxTiles = maxLayerTiles
			layer!.imageDepth = 2
			layer!.allowFrameLoading = false
			layer!.currentImage = 0.5
			layer!.singleLevelLoading = (mapType == .Maply2DMap)
			layer!.shaderProgramName = kMaplyShaderDefaultTriNightDay
			baseViewC?.addLayer(layer!)
			layer!.drawPriority = BaseEarthPriority
			baseLayer = layer

			screenLabelColor = UIColor.whiteColor()
			screenLabelBackColor = UIColor.whiteColor()
			labelColor = UIColor.blackColor()
			labelBackColor = UIColor.whiteColor()
			vecColor = UIColor.blackColor()
			vecWidth = 4.0

		case kMaplyTestQuadTest:
			self.title = kMaplyTestQuadTest

			screenLabelColor = UIColor.whiteColor()
			screenLabelBackColor = UIColor.whiteColor()
			labelColor = UIColor.blackColor()
			labelBackColor = UIColor.whiteColor()
			vecColor = UIColor.blackColor()
			vecWidth = 4.0

			let tileSource = MaplyAnimationTestTileSource(
				coordSys: MaplySphericalMercator(),
				minZoom: Int32(0),
				maxZoom: Int32(21),
				depth: Int32(1))

			tileSource.pixelsPerSide = 256
			tileSource.transparentMode = true

			let layer = MaplyQuadImageTilesLayer(tileSource: tileSource)
			layer!.waitLoad = imageWaitLoad
			layer!.requireElev = requireElev
			layer!.maxTiles = 512
			layer!.handleEdges = true
			//        layer.color = [UIColor colorWithWhite:0.5 alpha:0.5];
			if mapType == .Maply2DMap {
				// Note: Debugging
				layer!.useTargetZoomLevel = true
				layer!.singleLevelLoading = true
				layer!.multiLevelLoads = [NSNumber(integer: -2)]
			}
			baseViewC?.addLayer(layer!)
			layer!.drawPriority = BaseEarthPriority
			baseLayer = layer
			//        [self zoomTest];

		case kMaplyTestQuadVectorTest:
			self.title = kMaplyTestQuadVectorTest

			screenLabelColor = UIColor.whiteColor()
			screenLabelBackColor = UIColor.whiteColor()
			labelColor = UIColor.blackColor()
			labelBackColor = UIColor.whiteColor()
			vecColor = UIColor.blackColor()
			vecWidth = 4.0

			let tileSource = MaplyPagingVectorTestTileSource(
				coordSys: MaplySphericalMercator(),
				minZoom: Int32(0),
				maxZoom: Int32(10))

			let layer = MaplyQuadPagingLayer(
				coordSystem: tileSource.coordSys,
				delegate: tileSource)
			layer!.importance = 128*128
			layer!.singleLevelLoading = (mapType == .Maply2DMap)
			baseViewC?.addLayer(layer!)
			layer!.drawPriority = BaseEarthPriority
			baseLayer = layer

		case kMaplyTestElevation:
			self.title = kMaplyTestElevation

			let maxZoom = (zoomLimit != 0 && zoomLimit < 16) ? zoomLimit : 16
			let elevSource = MaplyRemoteTileElevationCesiumSource(
				baseURL: "http://cesiumjs.org/stk-terrain/tilesets/world/tiles/",
				ext: "terrain",
				minZoom: Int32(0),
				maxZoom: Int32(maxZoom))

			baseViewC?.elevDelegate = elevSource

			screenLabelColor = UIColor.whiteColor()
			screenLabelBackColor = UIColor.whiteColor()
			labelColor = UIColor.blackColor()
			labelBackColor = UIColor.whiteColor()
			vecColor = UIColor.blackColor()
			vecWidth = 4.0

			let tileSource = MaplyPagingElevationTestTileSource(
				coordSys: MaplySphericalMercator(),
				minZoom: Int32(0),
				maxZoom: Int32(10),
				elevSource: elevSource)

			let layer = MaplyQuadPagingLayer(coordSystem: tileSource.coordSys, delegate: tileSource)

			layer!.importance = 128*128
			layer!.singleLevelLoading = (mapType == .Maply2DMap)
			baseViewC?.addLayer(layer!)
			layer!.drawPriority = 0
			baseLayer = layer!

		case kMaplyTestQuadTestAnimate:
			self.title = kMaplyTestQuadTestAnimate

			screenLabelColor = UIColor.whiteColor()
			screenLabelBackColor = UIColor.whiteColor()
			labelColor = UIColor.blackColor()
			labelBackColor = UIColor.whiteColor()
			vecColor = UIColor.blackColor()
			vecWidth = 4.0

			let tileSource = MaplyAnimationTestTileSource(
				coordSys: MaplySphericalMercator(),
				minZoom: Int32(0),
				maxZoom: Int32(17),
				depth: Int32(4))

			tileSource.transparentMode = true
			tileSource.pixelsPerSide = 128

			let layer = MaplyQuadImageTilesLayer(tileSource: tileSource)
			layer!.waitLoad = imageWaitLoad
			layer!.requireElev = requireElev
			layer!.imageDepth = 4
			layer!.handleEdges = (mapType != .Maply2DMap)
			layer!.maxTiles = 512
			// We'll cycle through at 1/2s per layer
			layer!.animationPeriod = 2.0
			layer!.allowFrameLoading = false
			layer!.useTargetZoomLevel = true
			layer!.singleLevelLoading = true
			layer!.multiLevelLoads = [NSNumber(integer: -3)]
			baseViewC?.addLayer(layer!)
			layer!.drawPriority = BaseEarthPriority
			baseLayer = layer

		default: ()

		}

		// If we're fetching one of the JSON tile specs, kick that off
		if let jsonTileSpec = jsonTileSpec {
			let request = NSURLRequest(URL: NSURL(string: jsonTileSpec)!)
			let operation = AFHTTPRequestOperation(request: request)
			operation.setCompletionBlockWithSuccess({
				(operation, responseObject) -> Void in

				// Add a quad earth paging layer based on the tile spec we just fetched
				let tileSource = MaplyRemoteTileSource(tilespec: responseObject as! [NSObject : AnyObject])
				tileSource!.cacheDir = thisCacheDir
				if self.zoomLimit != 0 && self.zoomLimit < tileSource!.maxZoom() {
					tileSource!.tileInfo.maxZoom = self.zoomLimit
				}
				let layer = MaplyQuadImageTilesLayer(tileSource: tileSource!)
				layer!.handleEdges = true
				layer!.waitLoad = self.imageWaitLoad
				layer!.requireElev = self.requireElev
				layer!.maxTiles = self.maxLayerTiles
				if self.mapType == .Maply2DMap {
					layer!.singleLevelLoading = true
					layer!.multiLevelLoads = [NSNumber(integer: -4), NSNumber(integer: -2)]
				}
				self.baseViewC?.addLayer(layer!)
				layer!.drawPriority = self.BaseEarthPriority
				self.baseLayer = layer
			},
			failure: { (operation, error) -> Void in
			})
		}

		// Set up some defaults for display
		screenLabelDesc = [
			kMaplyTextColor: screenLabelColor,
			kMaplyFade: 1.0,
			kMaplyTextOutlineSize: 1.5,
			kMaplyTextOutlineColor: UIColor.blackColor(),
		]
		labelDesc = [
			kMaplyTextColor: labelColor,
			kMaplyBackgroundColor: labelBackColor,
			kMaplyFade: 1.0
		]
		vectorDesc = [
			kMaplyColor: vecColor,
			kMaplyVecWidth: vecWidth,
			kMaplyFade: 1.0,
			kMaplySelectable: true
		]
	}

	private func setupOverlays(baseSettings: [String:Bool]) {
		// For network paging layers, where we'll store temp files
		let cacheDir = NSSearchPathForDirectoriesInDomains(.CachesDirectory, .UserDomainMask, true)[0] as! String
		var thisCacheDir: String?

		for (layerName,selected) in baseSettings {
			let layer = ovlLayers[layerName]

			// Need to create the layer
			if selected && layer == nil {
				thisCacheDir = createLayer(cacheDir, name: layerName)
			}
			else if let layer = layer where !selected {
				// Get rid of the layer
				baseViewC?.removeLayer(layer)
				ovlLayers.removeValueForKey(layerName)
			}
		}

		// Fill out the cache dir if there is one
		if let thisCacheDir = thisCacheDir {
			try! NSFileManager.defaultManager().createDirectoryAtPath(thisCacheDir,
				withIntermediateDirectories: true,
				attributes: nil)
		}
	}

	private func createLayer(cacheDir: String, name layerName: String) -> String? {
		var thisCacheDir: String?

		switch layerName {
		case kMaplyTestUSGSOrtho:
			thisCacheDir = "\(cacheDir)/usgs_naip/"
			let url = "http://raster.nationalmap.gov/ArcGIS/services/Orthoimagery/USGS_EDC_Ortho_NAIP/ImageServer/WMSServer"
			fetchWMSLayer(url,
				layer: "0",
				style: nil,
				cacheDir: cacheDir,
				ovlName: layerName)

		case kMaplyTestOWM:
			let tileSource = MaplyRemoteTileSource(
				baseURL: "http://tile.openweathermap.org/map/precipitation/",
				ext: "png",
				minZoom: 0,
				maxZoom: 6)
			thisCacheDir = "\(cacheDir)/openweathermap_precipitation/"
			tileSource?.cacheDir = thisCacheDir
			tileSource?.tileInfo.cachedFileLifetime = 3 * 60 * 60 // invalidate OWM data after three hours
			let weatherLayer = MaplyQuadImageTilesLayer(tileSource: tileSource!)
			weatherLayer?.coverPoles = false
			weatherLayer?.handleEdges = false

			baseViewC?.addLayer(weatherLayer!)
			ovlLayers[layerName] = weatherLayer

		case kMaplyTestForecastIO:
			// Collect up the various precipitation sources
			var tileSources = [MaplyRemoteTileInfo]()
			for i in 0...4 {
				let precipTileSource = MaplyRemoteTileInfo(
					baseURL: "http://a.tiles.mapbox.com/v3/mousebird.precip-example-layer\(i)/",
					ext: "png",
					minZoom: 0,
					maxZoom: 6)
				precipTileSource.cacheDir = "\(cacheDir)/forecast_io_weather_layer\(i)/"
				tileSources.append(precipTileSource)
			}

			let precipTileSource = MaplyMultiplexTileSource(sources: tileSources)
			// Create a precipitation layer that animates
			let precipLayer = MaplyQuadImageTilesLayer(tileSource: precipTileSource!)
			precipLayer?.imageDepth = UInt32(tileSources.count)
			precipLayer?.animationPeriod = 6.0
			precipLayer?.imageFormat = MaplyQuadImageFormat.ImageUByteRed
			precipLayer?.numSimultaneousFetches = 4
			precipLayer?.handleEdges = false
			precipLayer?.coverPoles = false
			precipLayer?.shaderProgramName = WeatherShader.setupWeatherShader(baseViewC!)
			precipLayer?.fade = 0.5

			baseViewC?.addLayer(precipLayer!)
			ovlLayers[layerName] = precipLayer

		default: ()
		}

		return thisCacheDir
	}

	// Try to fetch the given WMS layer
	private func fetchWMSLayer(baseURL: String,
		layer: String,
		style: String?,
		cacheDir: String,
		ovlName: String) {

		let capabilitiesURL = MaplyWMSCapabilities.CapabilitiesURLFor(baseURL)
		let request = NSURLRequest(URL: NSURL(string: capabilitiesURL)!)
		let operation = AFHTTPRequestOperation(request: request)
		operation.responseSerializer = AFXMLParserResponseSerializer()
		operation.setCompletionBlockWithSuccess({ op, responseObject in
			self.startWMSLayerBaseURL(baseURL,
				xml: responseObject as! DDXMLDocument,
				layerName: layer,
				styleName: style,
				cacheDir: cacheDir,
				ovlName: ovlName)
		},
		failure: { op, err in

		})
		operation.start()
	}

	private func startWMSLayerBaseURL(
		baseURL: String,
		xml: DDXMLDocument,
		layerName: String,
		styleName: String?,
		cacheDir: String,
		ovlName: String?) -> Bool {

		let cap = MaplyWMSCapabilities(XML: xml)
		let layer = cap?.findLayer(layerName)
		let coordSys = layer?.buildCoordSystem()
		let style = layer?.findStyle(styleName ?? "")

		if layer == nil {
			print("Couldn't find layer \(layerName) in WMS response.")
			return false
		}
		else if coordSys == nil {
			print("No coordinate system we recognize in WMS response.")
			return false
		}
		else if styleName != nil && style == nil {
			print("No style named \(styleName) in WMS response.")
			return false
		}

		let tileSource = MaplyWMSTileSource(
			baseURL: baseURL,
			capabilities: cap,
			layer: layer!,
			style: style!,
			coordSys: coordSys!,
			minZoom: 0,
			maxZoom: 16,
			tileSize: 256)
		tileSource?.cacheDir = cacheDir
		tileSource?.transparent = true

		let imageLayer = MaplyQuadImageTilesLayer(
			coordSystem: coordSys!,
			tileSource: tileSource!)
		imageLayer?.coverPoles = false
		imageLayer?.handleEdges = true
		imageLayer?.requireElev = requireElev
		imageLayer?.waitLoad = imageWaitLoad

		baseViewC?.addLayer(imageLayer!)

		if let ovlName = ovlName {
			ovlLayers[ovlName] = imageLayer
		}

		return true
	}


	private func isElevated() -> Bool {
		switch mapType {
		case .MaplyGlobe(let elevation) where elevation:
			return true
		default: ()
			return false
		}
	}


}
