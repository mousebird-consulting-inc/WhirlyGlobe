//
//  ImageSingleLevelTestCase.swift
//  AutoTester
//
//  Created by Steve Gifford on 10/6/16.
//  Copyright © 2016-2017 mousebird consulting. All rights reserved.
//

import Foundation

class ImageSingleLevelTestCase: MaplyTestCase {
    
    override init() {
        super.init()
        
        self.name = "Image Single Level Test Case"
        self.captureDelay = 4
        self.implementations = [.globe, .map]
    }
    
    func setupLayer(baseVC: MaplyBaseViewController) -> MaplyQuadImageTilesLayer {
        let sphericalMercator = MaplySphericalMercator()
        let tileSource = MaplyAnimationTestTileSource(
            coordSys: sphericalMercator,
            minZoom: 0,
            maxZoom: 16,
            depth: 1)
        tileSource.useDelay = false
        let layer = MaplyQuadImageTilesLayer(tileSource: tileSource)
        layer!.handleEdges = false
        layer!.singleLevelLoading = true
        layer!.drawPriority = kMaplyImageLayerDrawPriorityDefault+100
        layer!.color = UIColor.init(white: 0.8, alpha: 0.8)
        
        return layer!;
    }
    
    override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
        let baseLayer = StamenWatercolorRemote()
        baseLayer.setUpWithGlobe(globeVC)
        
        globeVC.add(setupLayer(baseVC: globeVC))
        
        globeVC.keepNorthUp = true
        globeVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(-3.6704803, 40.5023056), time: 1.0)
    }
    
    override func setUpWithMap(_ mapVC: MaplyViewController) {
        let baseLayer = StamenWatercolorRemote()
        baseLayer.setUpWithMap(mapVC)

        mapVC.add(setupLayer(baseVC: mapVC))
        
        mapVC.animate(toPosition:MaplyCoordinateMakeWithDegrees(-3.6704803, 40.5023056), height: 1.0, time: 1.0)
        mapVC.setZoomLimitsMin(0.01, max: 4.0)
    }
    
//    override func remoteResources() -> [AnyObject]? {
//        return nil;
        /*
         return ["https://manuals.info.apple.com/en_US/macbook_retina_12_inch_early2016_essentials.pdf"]
         */
//    }
}
