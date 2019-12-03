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
        
        // Maptiler token
        // Go to maptiler.com, setup an account and get your own
        let token = "GetYerOwnToken"
        if token == "GetYerOwnToken" {
            let alertControl = UIAlertController(title: "Missing Token", message: "You need to add your own Maptiler token.\nYou can't use mine.", preferredStyle: .alert)
            alertControl.addAction(UIAlertAction(title: "Fine!", style: .cancel, handler: { _ in
                alertControl.dismiss(animated: true, completion: nil)
            }))
            viewC.present(alertControl, animated: true, completion: nil)
            return
        }

        // Parse it and then let it start itself
        let mapboxMap = MapboxKindaMap(fileName, viewC: viewC)
        mapboxMap.styleSettings.textScale = 1.2  // Note: Why does this work better than 2.0?
        mapboxMap.backgroundAllPolys = round     // Render all the polygons into an image for the globe
        mapboxMap.cacheDir = FileManager.default.urls(for: .cachesDirectory, in: .userDomainMask)[0].appendingPathComponent(name)
        // Replace the MapTilerKey in any URL with the actual token
        mapboxMap.fileOverride = {
            (url) in
            if url.isFileURL {
                return url
            }
            if url.absoluteString.contains("MapTilerKey") {
                return URL(string: url.absoluteString.replacingOccurrences(of: "MapTilerKey", with: token))!
            }
            
            return url
        }
        mapboxMap.start()
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

