//
//  ESRIRemote.swift
//  AutoTester
//
//  Created by Jess Taylor on 6/14/21.
//  Copyright 2021-2022 mousebird consulting. All rights reserved.
//

import UIKit
import WhirlyGlobe

/**
    This ESRI Bathymetry tile suite consists of two layers, a base layer and a labels layer. We create an
    individual image loader for each layer.
    
    If you use the ESRI suite publically be sure to go to their site and read the instructions for how to give
    them proper credidation (which means saying "maps from ESRI" or similar). But other than that, they are
    a free tile source.
 */
class ESRIRemoteTestCase: MaplyTestCase {

    override init() {
        super.init()
        self.name = "ESRI Bathymetry Remote"
        self.implementations = [.globe, .map]
    }
    
    var imageLoaderBase : MaplyQuadImageLoader? = nil
    var imageLoaderLabels : MaplyQuadImageLoader? = nil

    func setupLoaderBase(_ baseVC: MaplyBaseViewController) -> MaplyQuadImageLoader? {
        let cacheDir = NSSearchPathForDirectoriesInDomains(.cachesDirectory, .userDomainMask, true)[0]
        let thisCacheDir = "\(cacheDir)/esribasetiles/"
        let maxZoom = Int32(16)
        let tileInfo = MaplyRemoteTileInfoNew(baseURL: "https://services.arcgisonline.com/ArcGIS/rest/services/Ocean/World_Ocean_Base/MapServer/tile/{z}/{y}/{x}.png",
                                              minZoom: Int32(0),
                                              maxZoom: maxZoom)
        tileInfo.cacheDir = thisCacheDir
        
        // Parameters describing how we want a globe broken down
        let sampleParams = MaplySamplingParams()
        sampleParams.coordSys = MaplySphericalMercator(webStandard: ())
        sampleParams.coverPoles = true
        sampleParams.edgeMatching = true
        sampleParams.maxZoom = tileInfo.maxZoom()
        sampleParams.singleLevel = true
        sampleParams.minImportance = 1024.0 * 1024.0 / 2.0
        
        guard let imageLoader = MaplyQuadImageLoader(params: sampleParams, tileInfo: tileInfo, viewC: baseVC) else {
            return nil
        }
        return imageLoader
    }

    func setupLoaderLabels(_ baseVC: MaplyBaseViewController) -> MaplyQuadImageLoader? {
        let cacheDir = NSSearchPathForDirectoriesInDomains(.cachesDirectory, .userDomainMask, true)[0]
        let thisCacheDir = "\(cacheDir)/esrilabelstiles/"
        let maxZoom = Int32(16)
        let tileInfo = MaplyRemoteTileInfoNew(baseURL: "https://services.arcgisonline.com/ArcGIS/rest/services/Ocean/World_Ocean_Reference/MapServer/tile/{z}/{y}/{x}.png",
                                              minZoom: Int32(0),
                                              maxZoom: maxZoom)
        tileInfo.cacheDir = thisCacheDir
        
        // Parameters describing how we want a globe broken down
        let sampleParams = MaplySamplingParams()
        sampleParams.coordSys = MaplySphericalMercator(webStandard: ())
        sampleParams.coverPoles = true
        sampleParams.edgeMatching = true
        sampleParams.maxZoom = tileInfo.maxZoom()
        sampleParams.singleLevel = true
        sampleParams.minImportance = 1024.0 * 1024.0 / 2.0
        
        guard let imageLoader = MaplyQuadImageLoader(params: sampleParams, tileInfo: tileInfo, viewC: baseVC) else {
            return nil
        }
        return imageLoader
    }

    override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
        imageLoaderBase = setupLoaderBase(globeVC)
        imageLoaderLabels = setupLoaderLabels(globeVC)
        globeVC.keepNorthUp = true
        globeVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(-3.6704803, 40.5023056), time: 1.0)
    }

    override func setUpWithMap(_ mapVC: MaplyViewController) {
        imageLoaderBase = setupLoaderBase(mapVC)
        imageLoaderLabels = setupLoaderLabels(mapVC)
        mapVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(-3.6704803, 40.5023056), height: 1.0, time: 1.0)
        mapVC.setZoomLimitsMin(0.01, max: 5.0)
    }
    
    override func stop() {
        imageLoaderBase?.shutdown()
        imageLoaderBase = nil
        imageLoaderLabels?.shutdown()
        imageLoaderLabels = nil
        super.stop()
    }
}
