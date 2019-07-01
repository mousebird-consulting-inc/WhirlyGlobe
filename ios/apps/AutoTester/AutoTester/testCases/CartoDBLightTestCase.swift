//
//  GlobeSamplerTestCase.swift
//  AutoTester
//
//  Created by Stephen Gifford on 3/27/18.
//  Copyright © 2018 mousebird consulting.
//

import UIKit

class CartoDBLightTestCase: MaplyTestCase {
    
    override init() {
        super.init()
        
        self.name = "CartoDB Light Test Case"
        self.implementations = [.globe,.map]
    }
    
    var imageLoader : MaplyQuadImageLoader? = nil
    
    // Put together a quad sampler layer
    func setupLoader(_ baseVC: MaplyBaseViewController) -> MaplyQuadImageLoader? {
        let cacheDir = NSSearchPathForDirectoriesInDomains(.cachesDirectory, .userDomainMask, true)[0]
        let thisCacheDir = "\(cacheDir)/cartodblight/"
        let maxZoom = Int32(16)
        let tileInfo = MaplyRemoteTileInfoNew(baseURL: "http://basemaps.cartocdn.com/rastertiles/voyager/{z}/{x}/{y}@2x.png",
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
        
        guard let imageLoader = MaplyQuadImageLoader(params: sampleParams, tileInfo: tileInfo, viewC: baseVC) else {
            return nil
        }
        imageLoader.imageFormat = .imageUShort565;
        //        imageLoader.debugMode = true
        
        return imageLoader
    }
    
    override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
        imageLoader = setupLoader(globeVC)
    }
    
    override func setUpWithMap(_ mapVC: MaplyViewController) {
        imageLoader = setupLoader(mapVC)
    }
}
