//
//  MapboxTestCase.swift
//  AutoTester
//
//  Created by Steve Gifford on 11/12/19.
//  Copyright 2019-2022 mousebird consulting. All rights reserved.
//

import UIKit
import WhirlyGlobe

class MapboxTestCase: MaplyTestCase {
    
    override init() {
        super.init()

        self.name = "Mapbox Variations"
        self.implementations = [.map,.globe]
    }
    
    // Styles included in the bundle
    let styles : [(name: String, sheet: String)] =
        [("Satellite", "mapbox_satellite-v9"),
         ("Hybrid Satellite", "mapbox_satellite-streets-v9")
//         ("Streets", "mapbox_streets")
    ]
    let MapboxStyle = 1
    
    var mapboxMap : MapboxKindaMap? = nil
    
    // Start fetching the required pieces for a Mapbox style map
    func startMap(_ style: (name: String, sheet: String), viewC: MaplyBaseViewController, round: Bool) {
        guard let fileName = Bundle.main.url(forResource: style.sheet, withExtension: "json") else {
            print("Style sheet missing from bundle: \(style.sheet)")
            return
        }

        // Parse it and then let it start itself
        let mapboxMap = MapboxKindaMap(fileName, viewC: viewC)
        // We have to capture the mapbox: URLs and redirect them because Mapbox does some funky things with their URLs

        // Put your Mapbox token here, or in the application environment variables.
        // To get a Mapbox token, go sign up on mapbox.com
        // Go to Edit Scheme, select Run, Arguments, and add an "MAPBOX_TOKEN" entry to Environment Variables.
        let myToken = "GetYerOwnToken"
        let key = "MAPBOX_TOKEN"
        var token = ProcessInfo.processInfo.environment[key] ?? myToken
        if token.isEmpty || token == "GetYerOwnToken" {
            if let def = UserDefaults.standard.string(forKey: key) {
                token = def
            }
        }

        if token.isEmpty || token == "GetYerOwnToken" {
            let alertControl = UIAlertController(title: "Missing Token", message: "You need to add your own Mapbox token.\nYou can't use mine.", preferredStyle: .alert)
            alertControl.addAction(UIAlertAction(title: "Fine!", style: .cancel, handler: { _ in
                alertControl.dismiss(animated: true, completion: nil)
            }))
            viewC.present(alertControl, animated: true, completion: nil)
            return
        }

        UserDefaults.standard.setValue(token, forKey: key)

        let spriteURLstr = "https://api.mapbox.com/styles/v1/{filename}?access_token=" + token
        let tileURLstr = "https://api.mapbox.com/v4/{filename}.json?secure&access_token=" + token
        let slashPat = try! NSRegularExpression(pattern: "(?<!:)//+")   // multiple slashes *not* preceded by a colon
        mapboxMap.fileOverride = {
            (url) in
            if url.isFileURL {
                return url
            }
            if let urlComp = URLComponents(url: url, resolvingAgainstBaseURL: false) {
                if urlComp.scheme == "mapbox" {
                    // These URLs a bit wonky.  The bit we want comes out in the host area
                    if let host = urlComp.host {
                        var fullStr: String
                        if host.contains("sprites") {
                            fullStr = spriteURLstr.replacingOccurrences(of: "{filename}", with: urlComp.path)
                        } else {
                            // These are probably TileJSON files
                            fullStr = tileURLstr.replacingOccurrences(of: "{filename}", with: host)
                        }
                        fullStr = slashPat.stringByReplacingMatches(in: fullStr, range: NSMakeRange(0,fullStr.count), withTemplate: "/")
                        return URL(string: fullStr)!
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
        
        startMap(styles[MapboxStyle], viewC: mapVC, round: false)
    }
    
    override func setUpWithGlobe(_ mapVC: WhirlyGlobeViewController) {
        mapVC.performanceOutput = true
        
        startMap(styles[MapboxStyle], viewC: mapVC, round: true)
    }

}

