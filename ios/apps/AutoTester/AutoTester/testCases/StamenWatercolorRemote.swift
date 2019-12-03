//
//  MapboxSatellite.swift
//  AutoTester
//
//  Created by jmnavarro on 13/10/15.
//  Copyright © 2015-2017 mousebird consulting.
//

import UIKit

class StamenWatercolorRemote: MaplyTestCase {

	override init() {
		super.init()

		self.name = "Stamen Watercolor Remote"
		self.implementations = [.globe, .map]
	}
    
    var imageLoader : MaplyQuadImageLoader? = nil
	
	func setupLoader(_ baseVC: MaplyBaseViewController) -> MaplyQuadImageLoader? {
        let cacheDir = NSSearchPathForDirectoriesInDomains(.cachesDirectory, .userDomainMask, true)[0]
        let thisCacheDir = "\(cacheDir)/stamentiles/"
        let maxZoom = Int32(16)
        let tileInfo = MaplyRemoteTileInfoNew(baseURL: "http://tile.stamen.com/watercolor/{z}/{x}/{y}.png",
                                              minZoom: Int32(0),
                                              maxZoom: Int32(maxZoom))
        tileInfo.cacheDir = thisCacheDir
        
        // Parameters describing how we want a globe broken down
        let sampleParams = MaplySamplingParams()
        sampleParams.coordSys = MaplySphericalMercator(webStandard: ())
        sampleParams.coverPoles = true
        sampleParams.edgeMatching = true
        sampleParams.minZoom = tileInfo.minZoom()
        sampleParams.maxZoom = tileInfo.maxZoom()
        sampleParams.singleLevel = true
//        sampleParams.minImportance = 1024.0 * 1024.0
        
        guard let imageLoader = MaplyQuadImageLoader(params: sampleParams, tileInfo: tileInfo, viewC: baseVC) else {
            return nil
        }
        // TODO: Get this working
//        imageLoader.imageFormat = .imageUShort565
        //        imageLoader.debugMode = true
        
        return imageLoader
	}

	override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
		imageLoader = setupLoader(globeVC)
        		
		globeVC.keepNorthUp = true
		globeVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(-3.6704803, 40.5023056), time: 1.0)
        
//        globeVC.globeCenter = CGPoint(x: globeVC.view.center.x, y: globeVC.view.center.y + 0.33*globeVC.view.frame.size.height/2.0)
	}

	override func setUpWithMap(_ mapVC: MaplyViewController) {
		imageLoader = setupLoader(mapVC)

		mapVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(-3.6704803, 40.5023056), height: 1.0, time: 1.0)
		mapVC.setZoomLimitsMin(0.01, max: 5.0)
	}
    
    override func stop() {
        imageLoader?.shutdown()
        imageLoader = nil
    }
}
