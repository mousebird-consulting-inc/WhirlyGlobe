//
//  MapTiler.swift
//  AutoTester
//
//  Created by Steve Gifford on 11/8/19.
//  Copyright © 2019 mousebird consulting. All rights reserved.
//

import UIKit

class MapTilerTestCase: MaplyTestCase {
    
    override init() {
        super.init()
        
        self.name = "MapTiler Test Cases"
        self.captureDelay = 4
        self.implementations = [.map,.globe]
    }
    
    // Styles included in the bundle
    let styles : [(name: String, sheet: String)] =
        [("Basic", "maptiler_basic"),
         ("Hybrid Satellite", "maptiler_hybrid_satellite"),
         ("Streets", "maptiler_streets"),
         ("Topo", "maptiler_topo")]
    let MapTilerStyle = 0
    
    var mapboxMap : MapboxKindaMap? = nil
    
    // Start fetching the required pieces for a Mapbox style map
    func startMap(_ style: (name: String, sheet: String), viewC: MaplyBaseViewController) {
        guard let fileName = Bundle.main.url(forResource: style.sheet, withExtension: "json") else {
            print("Style sheet missing from bundle: \(style.sheet)")
            return
        }

        // Parse it and then let it start itself
        mapboxMap = MapboxKindaMap(fileName, viewC: viewC)
        mapboxMap?.backgroundAllPolys = false
        mapboxMap?.start()
    }
    
    override func setUpWithMap(_ mapVC: MaplyViewController) {
        mapVC.performanceOutput = true
        
        startMap(styles[MapTilerStyle], viewC: mapVC)
    }
    
    override func setUpWithGlobe(_ mapVC: WhirlyGlobeViewController) {
        mapVC.performanceOutput = true
        
        startMap(styles[MapTilerStyle], viewC: mapVC)
    }

}

