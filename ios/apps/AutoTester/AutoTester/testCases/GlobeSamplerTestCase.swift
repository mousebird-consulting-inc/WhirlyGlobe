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
        self.implementations = [.globe]
    }
    
    // Put together a quad sampler layer
    func setupLayer(_ baseVC: MaplyBaseViewController) -> MaplyQuadSamplingLayer? {
        let coordSys = MaplySphericalMercator(webStandard: ())
        
        if let sampleLayer = MaplyQuadSamplingLayer.init(coordSystem: coordSys) {
            sampleLayer.setMinZoom(0, maxZoom: 10, importance: 256.0*256.0)

            return sampleLayer
        }

        return nil
    }
    
    override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
        if let layer = setupLayer(globeVC) {
            globeVC.add(layer)
        }
    }
}
