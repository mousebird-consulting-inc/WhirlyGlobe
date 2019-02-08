//
//  NASAGIBSTestCase.swift
//  AutoTester
//
//  Created by jmnavarro on 6/12/15.
//  Copyright © 2015-2017 mousebird consulting.
//

import UIKit

class NASAGIBSTestCase: MaplyTestCase {

	override init() {
		super.init()
		
		self.name = "NASA GIBS"
		self.implementations = [.globe, .map]
	}
    
    var baseImageLoader : MaplyQuadImageLoader? = nil
    var ovlImageLoader : MaplyQuadImageLoader? = nil

	override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
		baseImageLoader = setupBaseLayer(globeVC)
		ovlImageLoader = setupOverlaysLayer(globeVC)
		globeVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(-122.4192,37.7793), time: 1)
	}
	
	override func setUpWithMap(_ mapVC: MaplyViewController) {
		baseImageLoader = setupBaseLayer(mapVC)
		ovlImageLoader = setupOverlaysLayer(mapVC)
		mapVC.height = 0.5
		mapVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(-122.4192,37.7793), time: 1)
	}
	
	func setupBaseLayer (_ viewC: MaplyBaseViewController) -> MaplyQuadImageLoader? {
        let cacheDir = NSSearchPathForDirectoriesInDomains(.cachesDirectory, .userDomainMask, true)[0]
        let thisCacheDir = "\(cacheDir)/cartodbdark/"
        let maxZoom = Int32(16)
        let tileInfo = MaplyRemoteTileInfoNew(baseURL: "http://s.basemaps.cartocdn.com/dark_all/{z}/{x}/{y}@2x.png",
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
        
        guard let imageLoader = MaplyQuadImageLoader(params: sampleParams, tileInfo: tileInfo, viewC: viewC) else {
            return nil
        }
        imageLoader.baseDrawPriority = kMaplyImageLayerDrawPriorityDefault
        imageLoader.imageFormat = .imageUShort565;
        
        return imageLoader
	}
	
	func setupOverlaysLayer (_ viewC: MaplyBaseViewController) -> MaplyQuadImageLoader? {
        let cacheDir = NSSearchPathForDirectoriesInDomains(.cachesDirectory, .userDomainMask, true)[0]
        let thisCacheDir = "\(cacheDir)/nasa_lights/"
        let tileInfo = MaplyRemoteTileInfoNew(baseURL: "http://map1.vis.earthdata.nasa.gov/wmts-webmerc/VIIRS_CityLights_2012/default/2015-07-01/GoogleMapsCompatible_Level8/{z}/{y}/{x}.jpg",
                                              minZoom: 1,
                                              maxZoom: 8)
        tileInfo.cacheDir = thisCacheDir
        
        // Parameters describing how we want a globe broken down
        let sampleParams = MaplySamplingParams()
        sampleParams.coordSys = MaplySphericalMercator(webStandard: ())
        sampleParams.coverPoles = false
        sampleParams.edgeMatching = false
        sampleParams.minZoom = tileInfo.minZoom()
        sampleParams.maxZoom = tileInfo.maxZoom()
        sampleParams.singleLevel = true
        
        guard let imageLoader = MaplyQuadImageLoader(params: sampleParams, tileInfo: tileInfo, viewC: viewC) else {
            return nil
        }
        imageLoader.baseDrawPriority = kMaplyImageLayerDrawPriorityDefault+1000
        imageLoader.imageFormat = .imageUShort565;
        
        return imageLoader
	}

}
