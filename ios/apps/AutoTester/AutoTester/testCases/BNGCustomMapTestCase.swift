//
//  CustomMapBNG.swift
//  AutoTester
//
//  Created by jmnavarro on 10/12/15.
//  Copyright © 2015-2017 mousebird consulting.
//

import UIKit


class BNGCustomMapTestCase: MaplyTestCase {
	
	override init() {
		super.init()
		self.name = "British National Grid (custom map)"
		self.implementations = [.map]
	}
	
	override func setUpWithMap(_ mapVC: MaplyViewController) {
        mapVC.clearColor = UIColor.red
        
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
    
    var imageLoader : MaplyQuadImageLoader? = nil
    var debugInterp : MaplyOvlDebugImageLoaderInterpreter? = nil
	
	func createBritishNationalOverlayLocal(_ baseViewC: MaplyBaseViewController, maplyMap: Bool) {
		let bngCoordSys = buildBritishNationalGrid(false)
        
        // Parameters describing how we want the tiles broken down
        let sampleParams = MaplySamplingParams()
        sampleParams.coordSys = bngCoordSys
        sampleParams.coverPoles = false
        sampleParams.edgeMatching = false
        sampleParams.minZoom = 0
        sampleParams.maxZoom = 22
        sampleParams.singleLevel = true

        imageLoader = MaplyQuadImageLoader(params: sampleParams, tileInfo: nil, viewC: baseViewC)
        
        if let imageLoader = imageLoader {
            debugInterp = MaplyOvlDebugImageLoaderInterpreter(viewC: baseViewC)
            imageLoader.setInterpreter(debugInterp!)
            imageLoader.baseDrawPriority = kMaplyImageLayerDrawPriorityDefault+1000
        }
	}

}
