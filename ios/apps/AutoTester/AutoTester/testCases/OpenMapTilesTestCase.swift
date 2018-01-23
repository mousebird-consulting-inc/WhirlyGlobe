//
//  OpenMapTilesTestCase.swift
//  AutoTester
//
//  Created by Stephen Gifford on 12/7/17.
//  Copyright © 2017 Saildrone. All rights reserved.
//

import Foundation

class OpenMapTilesTestCase: MaplyTestCase {
    
    override init() {
        super.init()
        
        self.name = "OpenMapTiles Test Case"
        self.captureDelay = 4
        self.implementations = [.map, .globe]
    }
    
    // Set up an OpenMapTiles display layer
    func setupLayer(_ baseVC: MaplyBaseViewController) -> MaplyQuadPagingLayer?
    {
        guard let path = Bundle.main.path(forResource: "SE_Basic", ofType: "json") else {
            return nil
        }
        guard let styleData = NSData.init(contentsOfFile: path) else {
            return nil
        }
        guard let styleSet = MaplyMapboxVectorStyleSet.init(json: styleData as Data!, viewC: baseVC) else {
            return nil
        }
        
        // Set up an offline renderer
//        let offlineRender = MaplyRenderController.init(size: CGSize.init(width: 512.0, height: 512.0))
//        offlineRender?.setClear(UIColor.red)
//        let testImage = offlineRender?.renderToImage()
        
        let tileInfo = MaplyRemoteTileInfo.init(baseURL: "http://public-mobile-data-stage-saildrone-com.s3-us-west-1.amazonaws.com/openmaptiles/{z}/{x}/{y}.png", ext: nil, minZoom: 0, maxZoom: 14)
//        let tileSource = MaplyRemoteTileSource(info: tileInfo)
//        let simpleStyle = MaplyVectorStyleSimpleGenerator(viewC: baseVC)
        
//        if let tileSource = tileSource {
//            let pageDelegate = MapboxVectorTiles(tileSource: tileSource, style: styleSet, viewC: baseVC)
//            pageDelegate.tileParser?.debugLabel = true
//            pageDelegate.tileParser?.debugOutline = true
//            if let pageLayer = MaplyQuadPagingLayer(coordSystem: MaplySphericalMercator(), delegate: pageDelegate) {
//                pageLayer.flipY = false
//                pageLayer.importance = 512*512;
//                pageLayer.singleLevelLoading = true
//
//                // Background layer supplies the background color
//                if let backLayer = styleSet.layersByName["background"] as? MapboxVectorLayerBackground? {
//                    baseVC.clearColor = backLayer?.paint.color
//                }
//
//                return pageLayer
//            }
//        }
        
        let tileSource = MapboxVectorTileImageSource.init(tileInfo: tileInfo, style: styleSet, viewC: baseVC)
        if let tileSource = tileSource {
            if let imageLayer = MaplyQuadImageTilesLayer.init(tileSource: tileSource) {
                imageLayer.flipY = true
                baseVC.add(imageLayer)
            }
        }
        
        return nil
    }
    
    override func setUpWithMap(_ mapVC: MaplyViewController) {
        mapVC.performanceOutput = true
        
        if let layer = setupLayer(mapVC) {
            mapVC.add(layer)
        }
    }
    
    override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
        globeVC.performanceOutput = true
        
        if let layer = setupLayer(globeVC) {
            globeVC.add(layer)
        }
    }
}

