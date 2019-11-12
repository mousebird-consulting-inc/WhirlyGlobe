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
//         ("Topo", "maptiler_topo")
    ]
    let MapTilerStyle = 2
    
    var mapboxMap : MapboxKindaMap? = nil
    
    // Start fetching the required pieces for a Mapbox style map
    func startMap(_ style: (name: String, sheet: String), viewC: MaplyBaseViewController, round: Bool) {
        guard let fileName = Bundle.main.url(forResource: style.sheet, withExtension: "json") else {
            print("Style sheet missing from bundle: \(style.sheet)")
            return
        }

        // Parse it and then let it start itself
        let mapboxMap = MapboxKindaMap(fileName, viewC: viewC)
        mapboxMap.styleSettings.textScale = 1.2  // Note: Why does this work better than 2.0?
        mapboxMap.backgroundAllPolys = round     // Render all the polygons into an image for the globe
        mapboxMap.start()
        mapboxMap.cacheDir = FileManager.default.urls(for: .cachesDirectory, in: .userDomainMask)[0].appendingPathComponent(name)
        self.mapboxMap = mapboxMap
    }
    
    override func setUpWithMap(_ mapVC: MaplyViewController) {
        mapVC.performanceOutput = true
        
        startMap(styles[MapTilerStyle], viewC: mapVC, round: false)
    }
    
    override func setUpWithGlobe(_ mapVC: WhirlyGlobeViewController) {
        mapVC.performanceOutput = true
        
        startMap(styles[MapTilerStyle], viewC: mapVC, round: true)
    }

}

