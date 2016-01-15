//
//  CustomMapBNG.swift
//  AutoTester
//
//  Created by jmnavarro on 10/12/15.
//  Copyright © 2015 mousebird consulting. All rights reserved.
//

import UIKit


class BNGCustomMapTestCase: MaplyTestCase, MaplyViewControllerDelegate {
	
	override init() {
		super.init()
		self.name = "British National Grid (custom map)"
		self.captureDelay = 3
	}
	
	override func setUpWithMap(mapVC: MaplyViewController) -> Bool {
		GeographyClassTestCase().setUpWithMap(mapVC)
		mapVC.coordSys = self.buildBritishNationalGrid(true)
		createBritishNationalOverlayLocal(mapVC, maplyMap: true)

		return true
	}
	
	func buildBritishNationalGrid(display: Bool) -> (MaplyCoordinateSystem){
		let gsb = NSBundle.mainBundle().pathForResource("OSTN02_NTv2", ofType: "gsb")
		let proj4Str = "+proj=tmerc +lat_0=49 +lon_0=-2 +k=0.9996012717 +x_0=400000 +y_0=-100000 +ellps=airy +nadgrids=\(gsb!) +units=m +no_defs"
		let coordSys = MaplyProj4CoordSystem(string: proj4Str)
		var bbox = MaplyBoundingBox()
		bbox.ll.x = 1393.0196
		bbox.ll.y = 12494.9764
		bbox.ur.x = 671196.3657
		bbox.ur.y = 1230275.0454
		if display {
			let spanX = bbox.ur.x - bbox.ll.x
			let spanY = bbox.ur.y - bbox.ur.x
			let extra = Float(1.0)
			bbox.ll.x -= extra * spanX
			bbox.ur.x += extra * spanX
			bbox.ll.y -= extra * spanY
			bbox.ur.y += extra * spanY
		}
		coordSys.setBounds(bbox)

		return coordSys
	}
	
	func createBritishNationalOverlayLocal(baseView: MaplyBaseViewController, maplyMap: Bool) {
		let bngCoordSys = buildBritishNationalGrid(false)
		let tileSource = MaplyAnimationTestTileSource(
			coordSys: bngCoordSys,
			minZoom: 0,
			maxZoom: 22,
			depth: 1)
		tileSource.pixelsPerSide = 128
		tileSource.transparentMode = true
		let layer = MaplyQuadImageTilesLayer(
			coordSystem: tileSource.coordSys,
			tileSource: tileSource)
		layer?.maxTiles = 256
		layer?.handleEdges = false
		layer?.flipY = true
		layer?.coverPoles = false
		if maplyMap {
			layer?.useTargetZoomLevel = true
			layer?.singleLevelLoading = true
			layer?.multiLevelLoads = [-2]
		}
		baseView.addLayer(layer!)
		layer?.drawPriority = 100
	}

}
