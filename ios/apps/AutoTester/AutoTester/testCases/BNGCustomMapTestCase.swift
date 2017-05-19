//
//  CustomMapBNG.swift
//  AutoTester
//
//  Created by jmnavarro on 10/12/15.
//  Copyright © 2015-2017 mousebird consulting. All rights reserved.
//

import UIKit


class BNGCustomMapTestCase: MaplyTestCase {
	
	override init() {
		super.init()
		self.name = "British National Grid (custom map)"
		self.captureDelay = 20
		self.implementations = [.map]
	}
	
	override func setUpWithMap(_ mapVC: MaplyViewController) {
		StamenWatercolorRemote().setUpWithMap(mapVC)
		createBritishNationalOverlayLocal(mapVC, maplyMap: true)
		mapVC.setPosition(MaplyCoordinateMakeWithDegrees(-0.1275, 51.507222), height: 0.3)
		mapVC.setZoomLimitsMin(0.1, max: 4.0)
	}

	override func customCoordSystem() -> MaplyCoordinateSystem? {
		return self.buildBritishNationalGrid(true)
	}
	
	func buildBritishNationalGrid(_ display: Bool) -> (MaplyCoordinateSystem){
		let gsb = Bundle.main.path(forResource: "OSTN02_NTv2", ofType: "gsb")
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
	
	func createBritishNationalOverlayLocal(_ baseView: MaplyBaseViewController, maplyMap: Bool) {
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
		baseView.add(layer!)
		layer?.importanceScale = 4.0
		layer?.drawPriority = 10000
	}

}
