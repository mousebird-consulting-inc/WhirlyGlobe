//
//  OfflineRenderTestCase.swift
//  AutoTester
//
//  Created by Steve Gifford on Oct 23 2019.
//  Copyright © 2015-2019 mousebird consulting.
//

import UIKit

class OfflineRenderTestCase: MaplyTestCase {

    override init() {
        super.init()

        self.name = "Offline Renderer"
        self.implementations = [.globe]
    }
    
    var imageLoader : MaplyQuadImageLoader? = nil
    
    func setupLoader(_ baseVC: WhirlyGlobeRenderController) -> MaplyQuadImageLoader? {
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
        
        return imageLoader
    }

    override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
        guard let renderControl = WhirlyGlobeRenderController(size: CGSize(width: 1024, height: 768)) else {
            return
        }
        renderControl.keepNorthUp = true
        imageLoader = setupLoader(renderControl)
        
        // Set to a starting position
        let viewState0 = WhirlyGlobeViewControllerAnimationState()
        viewState0.pos = MaplyCoordinateDMakeWithDegrees(-3.6704803, 40.5023056)
        viewState0.height = 1.0
        viewState0.heading = 0.0
        renderControl.setViewState(viewState0)
        
        DispatchQueue.main.asyncAfter(deadline: .now()+5.0) {
            // Force a single render
            if let image0 = renderControl.snapshot() {
                print("Got an image!  \(image0)")
            }
            
            // Move and try again
            let viewState1 = WhirlyGlobeViewControllerAnimationState()
            viewState1.pos = MaplyCoordinateDMakeWithDegrees(-3.6704803, 40.5023056)
            viewState1.height = 0.25
            viewState1.heading = 0.0
            renderControl.setViewState(viewState1)

            DispatchQueue.main.asyncAfter(deadline: .now()+5.0) {
                if let image1 = renderControl.snapshot() {
                    print("Got another image! \(image1)")
                }
            }
        }
    }

}
