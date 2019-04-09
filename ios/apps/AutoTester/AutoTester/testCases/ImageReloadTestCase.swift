//
//  ImageReloadTestCase.swift
//  AutoTester
//
//  Created by Steve Gifford on 4/9/19.
//  Copyright © 2019 mousebird consulting. All rights reserved.
//

import UIKit

class ImageReloadTestCase: MaplyTestCase
{
    override init() {
        super.init()
        
        self.name = "Image Reload"
        self.implementations = [.globe, .map]
    }
    
    var imageLoader : MaplyQuadImageLoader? = nil
    let maxZoom : Int32 = 16

    // Set up a loader with one tile source
    func setupLoader(_ baseVC: MaplyBaseViewController) {
        // We're not caching so we can test refreshing while data might be loading
        let tileInfo = MaplyRemoteTileInfoNew(baseURL: "http://tile.stamen.com/watercolor/{z}/{x}/{y}.png",
                                              minZoom: 0,
                                              maxZoom: maxZoom)
        
        // Parameters describing how we want a globe broken down
        let sampleParams = MaplySamplingParams()
        sampleParams.coordSys = MaplySphericalMercator(webStandard: ())
        sampleParams.coverPoles = true
        sampleParams.edgeMatching = true
        sampleParams.minZoom = tileInfo.minZoom()
        sampleParams.maxZoom = tileInfo.maxZoom()
        sampleParams.singleLevel = true
        
        guard let imageLoader = MaplyQuadImageLoader(params: sampleParams, tileInfo: tileInfo, viewC: baseVC) else {
            return
        }
        self.imageLoader = imageLoader
        imageLoader.imageFormat = .imageUShort565
        //        imageLoader.debugMode = true
        
        // Let things settle and then change the source
        DispatchQueue.main.asyncAfter(deadline: .now() + 10.0) {
            if (self.imageLoader == nil) {
                return
            }
            
            // Now we change to a completely different tile source
            // This will also a trigger a reload
            let tileInfo = MaplyRemoteTileInfoNew(baseURL: "http://basemaps.cartocdn.com/rastertiles/voyager/{z}/{x}/{y}@2x.png",
                                                  minZoom: 0,
                                                  maxZoom: self.maxZoom)
            self.imageLoader?.changeTileInfo(tileInfo)
        }
    }
    
    override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
        setupLoader(globeVC)
        
        globeVC.keepNorthUp = true
        globeVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(-3.6704803, 40.5023056), time: 1.0)
    }
    
    override func setUpWithMap(_ mapVC: MaplyViewController) {
        setupLoader(mapVC)
        
        mapVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(-3.6704803, 40.5023056), height: 1.0, time: 1.0)
        mapVC.setZoomLimitsMin(0.01, max: 5.0)
    }
}
