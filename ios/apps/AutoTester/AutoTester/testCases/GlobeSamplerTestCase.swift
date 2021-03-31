//
//  GlobeSamplerTestCase.swift
//  AutoTester
//
//  Created by Stephen Gifford on 3/27/18.
//  Copyright © 2018 mousebird consulting.
//

import Foundation

class GlobeSamplerTestCase: MaplyTestCase {

    override init() {
        super.init()
        
        self.name = "GlobeSampler Test Case"
        self.implementations = [.globe,.map]
    }

    var imageLoader : MaplyQuadImageLoader? = nil

    // Put together a quad sampler layer
    func setupLoader(_ baseVC: MaplyBaseViewController) -> MaplyQuadImageLoader? {
        // Stamen tile source
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
        sampleParams.maxZoom = tileInfo.maxZoom()
        sampleParams.singleLevel = true
        
        guard let imageLoader = MaplyQuadImageLoader(params: sampleParams, tileInfo: tileInfo, viewC: baseVC) else {
            return nil
        }
        let interp = MaplyOvlDebugImageLoaderInterpreter(viewC: baseVC)
        imageLoader.setInterpreter(interp)
#if !targetEnvironment(simulator)
        imageLoader.imageFormat = .imageUShort565;
#endif
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
