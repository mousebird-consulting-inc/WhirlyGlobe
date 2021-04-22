//
//  CustomMapBNG.swift
//  AutoTester
//
//  Created by jmnavarro on 10/12/15.
//  Copyright © 2015-2021 mousebird consulting.
//

import UIKit


class BNGCustomMapTestCase: MaplyTestCase {
	
	override init() {
		super.init()
		self.name = "British National Grid Custom Map"
		self.implementations = [.map]
	}
	
	override func setUpWithMap(_ mapVC: MaplyViewController) {
        baseCase.setUpWithMap(mapVC)
		createBritishNationalOverlayLocal(mapVC)

        let bound = geoBound(Self.buildBritishNationalGrid(false))
        let middle = MaplyCoordinate(x: (bound.ll.x + bound.ur.x) / 2.0,
                                     y: (bound.ll.y + bound.ur.y) / 2.0)
        let h = mapVC.findHeight(toViewBounds: bound, pos: middle)
        mapVC.setPosition(middle, height: h/3)
        mapVC.animate(toPosition: middle, height: h, heading: 0, time: 1)
	}

    public func geoBound(_ coordSys: MaplyCoordinateSystem) -> MaplyBoundingBox {
        var ll = MaplyCoordinate()
        var ur = MaplyCoordinate()
        coordSys.getBoundsLL(&ll, ur: &ur)
        return MaplyBoundingBox(ll: ll, ur: ur)
    }

	override func customCoordSystem() -> MaplyCoordinateSystem? {
		return Self.buildBritishNationalGrid(true)
	}

    override func stop() {
        debugInterp = nil
        imageLoader?.shutdown()
        imageLoader = nil
        imageFetcher?.shutdown()
        imageFetcher = nil
        baseCase.stop()
        super.stop()
    }

	static func buildBritishNationalGrid(_ display: Bool) -> MaplyCoordinateSystem {
		let gsb = Bundle.main.path(forResource: "OSTN02_NTv2", ofType: "gsb")
		let proj4Str = "+proj=tmerc +lat_0=49 +lon_0=-2 +k=0.9996012717 +x_0=400000 +y_0=-100000 +ellps=airy +units=m +no_defs +nadgrids=\(gsb!)"
		let coordSys = MaplyProj4CoordSystem(string: proj4Str)
		var bbox = MaplyBoundingBox()
		bbox.ll.x = 1393.0196
		bbox.ll.y = 13494.9764
		bbox.ur.x = 671196.3657
		bbox.ur.y = 1230275.0454
		if display {
            let extra = Float(0.5)
			let extraX = extra * (bbox.ur.x - bbox.ll.x)
            let extraY = extra * (bbox.ur.y - bbox.ll.y)
			bbox.ll.x -= extraX
			bbox.ur.x += extraX
			bbox.ll.y -= extraY
			bbox.ur.y += extraY
		}
		coordSys.setBounds(bbox)

		return coordSys
	}


	func createBritishNationalOverlayLocal(_ baseViewC: MaplyBaseViewController) {
        let bngCoordSys = Self.buildBritishNationalGrid(false)

        // Parameters describing how we want the tiles broken down
        let sampleParams = MaplySamplingParams()
        sampleParams.coordSys = bngCoordSys
        sampleParams.coverPoles = false
        sampleParams.edgeMatching = false
        sampleParams.maxZoom = 10
        sampleParams.minZoom = 0
        sampleParams.singleLevel = true

        let tileInfo = TestTileImageFetcher.TestTileInfo(minZoom: sampleParams.minZoom,
                                                         maxZoom: sampleParams.maxZoom)

        let fetcher = TestTileImageFetcher(control: baseViewC, name: "Test")
        fetcher.alpha = 96
        imageFetcher = fetcher

        guard let loader = MaplyQuadImageLoader(params: sampleParams,
                                                tileInfo: tileInfo,
                                                viewC: baseViewC) else { return }
        loader.baseDrawPriority = kMaplyImageLayerDrawPriorityDefault+1000
        loader.setTileFetcher(fetcher)
        loader.debugMode = true
        imageLoader = loader

        //    debugInterp = MaplyOvlDebugImageLoaderInterpreter(viewC: baseViewC)
        //    imageLoader.setInterpreter(debugInterp!)
        //    imageLoader.baseDrawPriority = kMaplyImageLayerDrawPriorityDefault+1000
	}

    var baseCase = GeographyClassTestCase()
    var imageLoader: MaplyQuadImageLoader?
    var imageFetcher: MaplyTileFetcher?
    var debugInterp: MaplyOvlDebugImageLoaderInterpreter?
}
