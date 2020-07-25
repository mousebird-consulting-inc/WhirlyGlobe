//
//  OpenMapTilesTestCase.swift
//  AutoTester
//
//  Created by Stephen Gifford on 12/23/17.
//  Copyright © 2017 Saildrone. All rights reserved.
//

import Foundation

class OpenMapTilesHybridTestCase: MaplyTestCase {
    
    override init() {
        super.init()
        
        self.name = "OpenMapTiles Hybrid"
        self.implementations = [.map, .globe]
    }
    
    var imageLoader : MaplyQuadImageLoader? = nil

    // Set up an OpenMapTiles display layer
    func setupLoader(_ baseVC: MaplyBaseViewController) -> MaplyQuadImageLoader?
    {
        guard let path = Bundle.main.path(forResource: "SE_Basic", ofType: "json") else {
            return nil
        }
        guard let styleData = NSData.init(contentsOfFile: path) else {
            return nil
        }
        
        // Set up an offline renderer and a Mapbox vector style handler to render to it
        let imageSize = (width: 512.0, height: 512.0)
        guard let offlineRender = MaplyRenderController.init(size: CGSize.init(width: imageSize.width, height: imageSize.height), mode: baseVC.getRenderType()) else {
            return nil
        }
        let imageStyleSettings = MaplyVectorStyleSettings.init(scale: UIScreen.main.scale)
        imageStyleSettings.arealShaderName = kMaplyShaderDefaultTriNoLighting
        guard let imageStyleSet = MapboxVectorStyleSet(json: styleData as Data,
                                                            settings: imageStyleSettings,
                                                            viewC: offlineRender) else {
                return nil
        }
        // TODO: Need to put the filter back here
        //            { (styleAttrs) -> Bool in
        //                // We only want polygons for the image
        //                if let type = styleAttrs["type"] as? String {
        //                    if type == "background" || type == "fill" {
        //                        return true
        //                    }
        //                }
        //                return false
        //        })
        //            else {
        //                return nil
        //        }

        // Set up a style for just the vector data we want to overlay
        let vectorSettings = MaplyVectorStyleSettings.init(scale: UIScreen.main.scale)
        vectorSettings.baseDrawPriority = 100+1
        vectorSettings.drawPriorityPerLevel = 1000
        guard let vectorStyleSet = MapboxVectorStyleSet(json: styleData as Data,
                                                             settings: vectorSettings,
                                                             viewC: baseVC) else {
            return nil
        }
// TODO: Need to put the filter back here
//            { (styleAttrs) -> Bool in
//                // We want everything but the polygons
//                if let type = styleAttrs["type"] as? String {
//                    if type != "background" && type != "fill" {
//                        return true
//                    }
//                }
//                return false
//        })
//            else {
//                return nil
//        }
        
        // Set up the tile info (where the data is) and the tile source to interpet it
        let tileInfo = MaplyRemoteTileInfoNew(baseURL: "http://public-mobile-data-stage-saildrone-com.s3-us-west-1.amazonaws.com/openmaptiles/{z}/{x}/{y}.png",
                                              minZoom: 0,
                                              maxZoom: 14)
        let cacheDir = NSSearchPathForDirectoriesInDomains(.cachesDirectory, .userDomainMask, true)[0]
        tileInfo.cacheDir = "\(cacheDir)/openmaptiles_saildrone/"

        // Parameters describing how we want a globe broken down
        let sampleParams = MaplySamplingParams()
        sampleParams.coordSys = MaplySphericalMercator(webStandard: ())
        sampleParams.minImportance = 1024 * 1024
        sampleParams.singleLevel = true
        sampleParams.levelLoads = [-3];
        if baseVC is WhirlyGlobeViewController {
            sampleParams.coverPoles = true
            sampleParams.edgeMatching = true
        } else {
            sampleParams.coverPoles = false
            sampleParams.edgeMatching = false
        }
        sampleParams.minZoom = 0
        sampleParams.maxZoom = tileInfo.maxZoom()
        
        guard let imageLoader = MaplyQuadImageLoader(params: sampleParams, tileInfo: tileInfo, viewC: baseVC) else {
            return nil
        }
//        imageLoader.debugMode = true

        guard let mapboxInterp = MapboxVectorInterpreter(imageStyle: imageStyleSet,
                                                         offlineRender: offlineRender,
                                                         vectorStyle: vectorStyleSet,
                                                         viewC: baseVC) else {
            return nil
        }
        imageLoader.setInterpreter(mapboxInterp)
        
        return imageLoader
    }
    
    override func setUpWithMap(_ mapVC: MaplyViewController) {
//        mapVC.performanceOutput = true
        mapVC.setPosition(MaplyCoordinateMakeWithDegrees(-0.1275, 51.507222), height: 0.01)
        imageLoader = setupLoader(mapVC)
    }
    
    override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
//        globeVC.performanceOutput = true
        globeVC.setPosition(MaplyCoordinateMakeWithDegrees(-0.1275, 51.507222), height: 0.01)
        globeVC.heading = 0.0

        imageLoader = setupLoader(globeVC)
    }
}


