//
//  VectorMBTilesTestCase.swift
//  AutoTester
//
//  Created by Steve Gifford on 3/9/16.
//  Copyright © 2016-2017 mousebird consulting.
//

import Foundation

class VectorMBTilesTestCase: MaplyTestCase {
    
    var baseCase = VectorsTestCase()
    
    var tileFetcher : MaplyMBTileFetcher? = nil
    var loader : MaplyQuadPagingLoader? = nil
    var vectorStyle : MaplyVectorStyleSimpleGenerator? = nil
    var interp : MapboxVectorInterpreter? = nil
    
    override init() {
        super.init()
        
        self.name = "Vector MBTiles"
       self.implementations = [.map, .globe]
    }
    
    func setupLoader(_ baseVC: MaplyBaseViewController) {
        // Fetcher for MBTiles
        guard let tileFetcher = MaplyMBTileFetcher(mbTiles: "France") else {
            return
        }
        self.tileFetcher = tileFetcher

        // Parameters describing how we want the space broken down
        let sampleParams = MaplySamplingParams()
        sampleParams.coordSys = MaplySphericalMercator(webStandard: ())
//        sampleParams.minImportance = 1024 * 1024
        sampleParams.singleLevel = true
        sampleParams.coverPoles = false
        sampleParams.edgeMatching = false
        sampleParams.minZoom = tileFetcher.minZoom()
        sampleParams.maxZoom = tileFetcher.maxZoom()
        
        // This parses the actual vector data and turns it into geometry
        vectorStyle = MaplyVectorStyleSimpleGenerator(viewC: baseVC)
        interp = MapboxVectorInterpreter(vectorStyle: vectorStyle!, viewC: baseVC)

        // Loader does the actual loading
        loader = MaplyQuadPagingLoader(params: sampleParams, tileInfo: tileFetcher.tileInfo(), loadInterp: interp!, viewC: baseVC)
        loader?.setTileFetcher(tileFetcher)
    }

    override func setUpWithMap(_ mapVC: MaplyViewController) {
        //        mapVC.performanceOutput = true
        baseCase.mapViewController = mapVC
        baseCase.baseViewController = mapVC
        baseCase.setUpWithMap(mapVC)
        setupLoader(mapVC)
    }
    
    override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
        //        globeVC.performanceOutput = true
        baseCase.globeViewController = globeVC
        baseCase.baseViewController = globeVC
        baseCase.setUpWithGlobe(globeVC)
        setupLoader(globeVC)
    }
    
    override func stop() {
        baseCase.stop()
        
        tileFetcher?.shutdown()
        tileFetcher = nil
        loader?.shutdown()
        loader = nil
        vectorStyle = nil
        interp = nil
    }
}
