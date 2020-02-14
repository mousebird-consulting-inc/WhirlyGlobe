//
//  VectorMBTilesTestCase.swift
//  AutoTester
//
//  Created by Steve Gifford on 3/9/16.
//  Copyright © 2016-2017 mousebird consulting.
//

import Foundation

class VectorMBTilesTestCase: MaplyTestCase {
    
    var baseCase = GeographyClassTestCase()
    
    typealias VecTileWrap = (tileFetcher: MaplyMBTileFetcher, loader: MaplyQuadPagingLoader, vectorStyle: MaplyVectorStyleSimpleGenerator, interp: MapboxVectorInterpreter)
    var vecTiles : [VecTileWrap] = []
    
    override init() {
        super.init()
        
        self.name = "Vector MBTiles"
       self.implementations = [.map, .globe]
    }
    
    func addVectorTiles(_ name: String, baseVC: MaplyBaseViewController) -> VecTileWrap? {
        // Fetcher for MBTiles
        guard let tileFetcher = MaplyMBTileFetcher(mbTiles: name) else {
            return nil
        }

        // Parameters describing how we want the space broken down
        let sampleParams = MaplySamplingParams()
        sampleParams.coordSys = MaplySphericalMercator(webStandard: ())
//        sampleParams.minImportance = 1024 * 1024
        sampleParams.singleLevel = true
        sampleParams.coverPoles = false
        sampleParams.edgeMatching = false
        sampleParams.minZoom = 0
        sampleParams.maxZoom = tileFetcher.maxZoom()
            
        // This parses the actual vector data and turns it into geometry
        let vectorStyle = MaplyVectorStyleSimpleGenerator(viewC: baseVC)
        let interp = MapboxVectorInterpreter(vectorStyle: vectorStyle!, viewC: baseVC)

        // Loader does the actual loading
        let loader = MaplyQuadPagingLoader(params: sampleParams, tileInfo: tileFetcher.tileInfo(), loadInterp: interp!, viewC: baseVC)
        loader?.setTileFetcher(tileFetcher)
        
        return (tileFetcher, loader!, vectorStyle!, interp!)
    }
    
    func setupLoader(_ baseVC: MaplyBaseViewController) {
//        if let wrap = addVectorTiles("93f80180-7bed-4a2c-be77-ef437f1cb935", baseVC: baseVC) {
//            vecTiles.append(wrap)
//        }
//        if let wrap = addVectorTiles("states-countries", baseVC: baseVC) {
//            vecTiles.append(wrap)
//        }
//        if let wrap = addVectorTiles("out-3", baseVC: baseVC) {
//            vecTiles.append(wrap)
//        }
        if let wrap = addVectorTiles("France", baseVC: baseVC) {
            vecTiles.append(wrap)
        }
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

        for wrap in vecTiles {
            wrap.tileFetcher.shutdown()
            wrap.loader.shutdown()
        }
        vecTiles = []
    }
}
