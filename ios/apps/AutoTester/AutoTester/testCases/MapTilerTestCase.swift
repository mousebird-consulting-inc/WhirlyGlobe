//
//  MapTiler.swift
//  AutoTester
//
//  Created by Steve Gifford on 11/8/19.
//  Copyright © 2019 mousebird consulting. All rights reserved.
//

import UIKit

/**
   Wrapper class for loading a Mapbox-style vector tiles-probably kinda map.
    You give it a style sheet and it figures out the rest.
    Set the various settings before it gets going to modify how it works
 */
class MapboxKindaMap {
    let styleURL: URL
    weak var viewC : MaplyBaseViewController? = nil
    var fetching = false
    var startWhenDoneFetching = false
    
    // TODO: Add a block for error handling
    init?(_ styleURL: URL, viewC: MaplyBaseViewController) {
        self.viewC = viewC
        if !styleURL.isFileURL {
            print("Remote URLs not yet supported")
            return nil
        }
        self.styleURL = styleURL

        // Load the style sheet itself
        guard let styleJson = try? Data(contentsOf: styleURL),
            let styleSheet = MapboxVectorStyleSet(json: styleJson,
                                                    settings: styleSettings,
                                                    viewC: viewC,
                                                    filter: nil) else {
                print("Failed to parse style sheet")
                return
        }
        self.styleSheet = styleSheet

        fetchStuff()
    }

    var styleSettings = MaplyVectorStyleSettings()
    var styleSheet : MapboxVectorStyleSet? = nil
    var spriteJSON : Data? = nil
    var spritePNG : UIImage? = nil
    
    // Information about the sources as we fetch them
    var outstandingFetches : [URLSessionDataTask?] = []

    // Fetch tile JSONs and sprite sheets currently
    func fetchStuff() {
        fetching = true
        guard let styleSheet = styleSheet else {
            return
        }

        // Fetch what we need to for the sources
        var success = true
        styleSheet.sources.forEach {
            let source = $0 as! MaplyMapboxVectorStyleSource
            if source.tileSpec == nil && success {
                guard let urlStr = source.url,
                    let url = URL(string: urlStr) else {
                    print("Expecting either URL or tile info for a source.  Giving up.")
                    success = false
                    return
                }
                
                // Go fetch the TileJSON
                let dataTask = URLSession.shared.dataTask(with: url) {
                    (data, resp, error) in
                    guard error == nil else {
                        print("Error trying to fetch tileJson from \(urlStr)")
                        self.stop()
                        return
                    }
                    
                    if let data = data,
                        let resp = try? JSONSerialization.jsonObject(with: data, options: []) as? [String: Any] {
                        source.tileSpec = resp
                        
                        self.checkFinished()
                    }
                }
                outstandingFetches.append(dataTask)
                dataTask.resume()
            }
        }
        
        // And for the sprite sheets
        if let spriteURLStr = styleSheet.spriteURL,
            let spriteJSONurl = URL(string: spriteURLStr)?.appendingPathComponent("sprite@2x.json"),
            let spritePNGurl = URL(string: spriteURLStr)?.appendingPathComponent("sprite@2x.png") {
                let dataTask1 = URLSession.shared.dataTask(with: spriteJSONurl) {
                    (data, resp, error) in
                    guard error == nil else {
                        print("Failed to fetch spriteJSON from \(spriteURLStr)")
                        self.stop()
                        return
                    }
                    self.spriteJSON = data

                    self.checkFinished()
                }
                dataTask1.resume()
                outstandingFetches.append(dataTask1)
                let dataTask2 = URLSession.shared.dataTask(with: spritePNGurl) {
                    (data, resp, error) in
                    guard error == nil else {
                        print("Failed to fetch spritePNG from \(spriteURLStr)")
                        self.stop()
                        return
                    }
                    if let data = data {
                        self.spritePNG = UIImage(data: data)
                    }
                    
                    self.checkFinished()
                }
                dataTask2.resume()
                outstandingFetches.append(dataTask2)
            }
        
        if !success {
            stop()
        }
    }
    
    // Check if we've finished loading stuff
    private func checkFinished() {
        var stillGoing = false
        outstandingFetches.forEach {
            if $0?.state == .running {
                stillGoing = true
            }
        }
        
        // If we're done, kick this off
        if !stillGoing && startWhenDoneFetching {
            fetching = false
            start()
        }
    }
    
    // TODO: Replace with a loader
    var pageLayer : MaplyQuadPagingLayer? = nil
    var pageDelegate : MapboxVectorTilesPagingDelegate? = nil
    
    // Done messing with settings?  Then fire this puppy up
    // Will shut down the loader(s) it started
    func start() {
        if fetching {
            startWhenDoneFetching = true
            return
        }
        
        guard let viewC = viewC,
            let styleSheet = styleSheet else {
            return
        }
        
        // Build the TileInfoNew objects while we're out it
        // We want the full min/max of all the tile sources
//        var tileSources : [MaplyRemoteTileInfoNew] = []
        var tileSources : [MaplyRemoteTileInfo] = []
        var zoom : (min: Int32, max: Int32) = (10000, -1)
        styleSheet.sources.forEach {
            guard let source = $0 as? MaplyMapboxVectorStyleSource else {
                print("Bad format in tileInfo for style sheet")
                return
            }
            if let minZoom = source.tileSpec?["minzoom"] as? Int32,
                let maxZoom = source.tileSpec?["maxzoom"] as? Int32,
                let tiles = source.tileSpec?["tiles"] as? [String] {
                zoom.min = min(minZoom,zoom.min)
                zoom.max = max(maxZoom,zoom.max)
                
//                let tileSource = MaplyRemoteTileInfoNew(baseURL: tiles[0], minZoom: minZoom, maxZoom: maxZoom)
                let tileSource = MaplyRemoteTileInfo(baseURL: tiles[0], ext: nil, minZoom: minZoom, maxZoom: maxZoom)
                tileSources.append(tileSource)
            }
        }
        
        // Parameters describing how we want a globe broken down
        let sampleParams = MaplySamplingParams()
        sampleParams.coordSys = MaplySphericalMercator(webStandard: ())
        sampleParams.minImportance = 1024 * 1024
        sampleParams.singleLevel = true
        if viewC is WhirlyGlobeViewController {
            sampleParams.coverPoles = true
            sampleParams.edgeMatching = true
        } else {
            sampleParams.coverPoles = false
            sampleParams.edgeMatching = false
        }
        sampleParams.minZoom = zoom.min
        sampleParams.maxZoom = zoom.max

        // One tile source per tile spec
        // TODO: Switch this over to the hybrid approach
        let tileSource = MaplyRemoteTileSource(info: tileSources[0])!
        let pageDelegate = MapboxVectorTilesPagingDelegate(tileSource: tileSource, style: styleSheet, viewC: viewC)
        //            pageDelegate.tileParser?.debugLabel = true
        //            pageDelegate.tileParser?.debugOutline = true
        if let pageLayer = MaplyQuadPagingLayer(coordSystem: MaplySphericalMercator(), delegate: pageDelegate) {
            pageLayer.flipY = false
            pageLayer.importance = 512*512;
            pageLayer.singleLevelLoading = true
            
            // Background layer supplies the background color
            if let backLayer = styleSheet.layersByName!["background"] as? MapboxVectorLayerBackground? {
                viewC.clearColor = backLayer?.paint.color
            }
            self.pageLayer = pageLayer
            viewC.add(pageLayer)
        }
        self.pageDelegate = pageDelegate
    }
    
    func stop() {
        // If we're still fetching config data, cancel that
        outstandingFetches.forEach {
            $0?.cancel()
        }
        outstandingFetches = []

        if let pageLayer = pageLayer {
            viewC?.remove(pageLayer)
        }
        pageLayer = nil
        pageDelegate = nil
    }
}

class MapTileTestCase: MaplyTestCase {
    
    override init() {
        super.init()
        
        self.name = "MapTiler Test Cases"
        self.captureDelay = 4
        self.implementations = [.map]
    }
    
    // Styles included in the bundle
    let styles : [(name: String, sheet: String)] =
        [("Basic", "maptiler_basic"),
         ("Hybrid Satellite", "maptiler_hybrid_satellite"),
         ("Streets", "maptiler_streets"),
         ("Topo", "maptiler_topo")]
    
    var mapboxMap : MapboxKindaMap? = nil
    
    // Start fetching the required pieces for a Mapbox style map
    func startMap(_ style: (name: String, sheet: String), viewC: MaplyBaseViewController) {
        guard let fileName = Bundle.main.url(forResource: style.sheet, withExtension: "json") else {
            print("Style sheet missing from bundle: \(style.sheet)")
            return
        }

        // Parse it and then let it start itself
        mapboxMap = MapboxKindaMap(fileName, viewC: viewC)
        if mapboxMap == nil {
            return
        }
        mapboxMap?.start()
    }
    
    override func setUpWithMap(_ mapVC: MaplyViewController) {
        mapVC.performanceOutput = true
        
        startMap(styles[0], viewC: mapVC)
    }
}

