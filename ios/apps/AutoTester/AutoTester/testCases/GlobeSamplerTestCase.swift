//
//  GlobeSamplerTestCase.swift
//  AutoTester
//
//  Created by Stephen Gifford on 3/27/18.
//  Copyright © 2018 mousebird consulting. All rights reserved.
//

import Foundation

class GlobeSamplerTestCase: MaplyTestCase {
    
    override init() {
        super.init()
        
        self.name = "GlobeSampler Test Case"
        self.captureDelay = 4
        self.implementations = [.globe,.map]
    }
    
    var imageLoader : MaplyQuadImageLoader? = nil
    
    // Put together a quad sampler layer
    func setupLoader(_ baseVC: MaplyBaseViewController) -> MaplyQuadImageLoader? {
        // Stamen tile source
        let cacheDir = NSSearchPathForDirectoriesInDomains(.cachesDirectory, .userDomainMask, true)[0]
        let thisCacheDir = "\(cacheDir)/stamentiles/"
        let maxZoom = Int32(16)
        guard let tileSource = MaplyRemoteTileSource(
            baseURL: "http://tile.stamen.com/watercolor/",
            ext: "jpg", minZoom: Int32(0), maxZoom: Int32(maxZoom)) else {
                return nil
        }
        tileSource.cacheDir = thisCacheDir
        
        // Parameters describing how we want a globe broken down
        let sampleParams = MaplySamplingParams()
        sampleParams.coordSys = tileSource.coordSys
        sampleParams.coverPoles = true
        sampleParams.edgeMatching = true
        sampleParams.minZoom = 0
        sampleParams.maxZoom = 16

        guard let imageLoader = MaplyQuadImageLoader(params: sampleParams, tileSource: tileSource, viewC: baseVC) else {
            return nil
        }
        imageLoader.numSimultaneousFetches = 8
        
        return imageLoader
    }
    
    override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
        imageLoader = setupLoader(globeVC)
    }
    
    override func setUpWithMap(_ mapVC: MaplyViewController) {
        imageLoader = setupLoader(mapVC)
    }
}
