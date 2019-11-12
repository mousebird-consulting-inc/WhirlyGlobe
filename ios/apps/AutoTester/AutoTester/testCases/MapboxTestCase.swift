//
//  MapboxTestCase.swift
//  AutoTester
//
//  Created by Steve Gifford on 11/12/19.
//  Copyright © 2019 mousebird consulting. All rights reserved.
//

import UIKit

class MapboxTestCase: MaplyTestCase {
    
    override init() {
        super.init()
        
        self.name = "Mapbox Test Cases"
        self.captureDelay = 4
        self.implementations = [.map,.globe]
    }
    
    // Styles included in the bundle
    let styles : [(name: String, sheet: String)] =
        [("Satellite", "mapbox_satellite-v9"),
         ("Hybrid Satellite", "mapbox_satellite-streets-v9"),
//         ("Streets", "mapbox_streets")
    ]
    let MapTilerStyle = 0
    
    var mapboxMap : MapboxKindaMap? = nil
    
    // Start fetching the required pieces for a Mapbox style map
    func startMap(_ style: (name: String, sheet: String), viewC: MaplyBaseViewController, round: Bool) {
        guard let fileName = Bundle.main.url(forResource: style.sheet, withExtension: "json") else {
            print("Style sheet missing from bundle: \(style.sheet)")
            return
        }

        // Parse it and then let it start itself
        let mapboxMap = MapboxKindaMap(fileName, viewC: viewC)
        // We have to capture the mapbox: URLs and redirect them
        let token = "GetYerOwnToken"
        if token == "GetYerOwnToken" {
            print("----------\nYou need to insert your own Mapbox token.  You can't use mine.\n------------")
            return
        }
        
        let spriteURLstr = "https://api.mapbox.com/styles/v1/{filename}?access_token=" + token
//        let styleURLstr = "https://api.mapbox.com/styles/v1/mapbox/{filename}?access_token=" + token
        let tileURLstr = "https://api.mapbox.com/v4/{filename}.json?secure&access_token=" + token
        mapboxMap.fileOverride = {
            (url) in
            if url.isFileURL {
                return url
            }
            if let urlComp = URLComponents(url: url, resolvingAgainstBaseURL: false) {
                if urlComp.scheme == "mapbox" {
                    // These URLs a bit wonky.  The bit we want comes out in the host area
                    if let host = urlComp.host {
                        if host.contains("sprites") {
                            let fullStr = spriteURLstr.replacingOccurrences(of: "{filename}", with: urlComp.path).replacingOccurrences(of: "//", with: "/")
                            return URL(string: fullStr)!
                        } else {
                            // These are probably TileJSON files
                            let fullStr = tileURLstr.replacingOccurrences(of: "{filename}", with: host)
                            return URL(string: fullStr)!
                        }
                    }
                }
            }
            return url
        }
        mapboxMap.styleSettings.textScale = 1.2  // Note: Why does this work better than 2.0?
        mapboxMap.backgroundAllPolys = false  // Render all the polygons into an image for the globe
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

