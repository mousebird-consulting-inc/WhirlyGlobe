//
//  TestViewController.swift
//  WhirlyGlobeSwiftTester
//
//  Created by jmWork on 14/09/15.
//  Copyright (c) 2015 Liferay. All rights reserved.
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

class TestViewController: UIViewController, UIPopoverControllerDelegate {

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

	var mapType = MapType.Maply2DMap {
		didSet {
			createMap(mapType)
		}
	}

	/// This is the base class shared between the MaplyViewController and the WhirlyGlobeViewController
	var baseViewC: MaplyBaseViewController?

	/// If we're displaying a globe, this is set
	var globeViewC: WhirlyGlobeViewController?

	/// If we're displaying a map, this is set
	var mapViewC: MaplyViewController?

	// The configuration view comes up when the user taps outside the globe
	var configViewC: ConfigViewController?

	var popControl: UIPopoverController?

	// If we're in 3D mode, how far the elevation goes
	var zoomLimit = 0
	var requireElev = false
	var imageWaitLoad = false
	var maxLayerTiles = 0

	var perfMode = PerformanceMode.Low


	override func viewDidLoad() {
		// What sort of hardware are we on?
		if UIScreen.mainScreen().scale > 1.0 {
			// Retina devices tend to be better, except for
			perfMode = .High
		}

/*TODO
		#if TARGET_IPHONE_SIMULATOR
			perfMode = HighPerformance;
		#endif
*/

		configViewC = ConfigViewController(nibName: "ConfigViewController", bundle: nil)
		configViewC!.options = .All

	}


	private func createMap(type: MapType) {
		// Create an empty globe or map controller
		zoomLimit = 0
		requireElev = false
		maxLayerTiles = 256

		cleanUp()

		switch type {
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

			//TODO config
//			configViewC.configOptions = ConfigOptionsFlat;
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

		let cacheDir = NSSearchPathForDirectoriesInDomains(.CachesDirectory, .UserDomainMask, true)[0] as! String

		// For elevation mode, we need to do some other stuff
		switch mapType {
		case .MaplyGlobe(let elevation) where elevation:
			() // TODO
		default: ()
		}

		// Force the view to load so we can get the default switch values
		configViewC?.loadView()

		// Bring up things based on what's turned on
		self.changeMapContents()

		// Settings panel
		self.navigationItem.rightBarButtonItem = UIBarButtonItem(barButtonSystemItem: .Edit, target: self, action: "showConfig")

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
/*
		let imageWaitLoad = configViewC!.valueForSection(kMaplyTestCategoryInternal, row: kMaplyTestWaitLoad)

		setupBaseLayer(configViewC!.values[0].rows)
		if configViewC!.values.count > 1 {
			setupOverlays(configViewC!.values[1].rows)
		}
*/
	}


}
