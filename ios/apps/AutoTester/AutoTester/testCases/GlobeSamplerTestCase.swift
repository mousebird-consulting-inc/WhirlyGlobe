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
    
    // Put together a quad sampler layer
    func setupLayer(_ baseVC: MaplyBaseViewController) -> MaplyQuadSamplingLayer? {
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

        let coordSys = MaplySphericalMercator(webStandard: ())
        guard let imageLoader = MaplyQuadImageLoader(tileSource: tileSource) else {
            return nil
        }
        imageLoader.numSimultaneousFetches = 8
        
        guard let sampleLayer = MaplyQuadSamplingLayer.init(coordSystem: coordSys, imageLoader: imageLoader) else {
            return nil
        }
        sampleLayer.setMinZoom(0, maxZoom: 22, importance: 256.0*256.0)
        sampleLayer.edgeMatching = true
        sampleLayer.coverPoles = true

        return sampleLayer
    }
    
    override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
        if let layer = setupLayer(globeVC) {
            globeVC.add(layer)
        }
    }
    
    override func setUpWithMap(_ mapVC: MaplyViewController) {
        if let layer = setupLayer(mapVC) {
            mapVC.add(layer)
        }
    }
}
