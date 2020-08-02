//
//  MapboxKindaMap.swift
//  WhirlyGlobeMaplyComponent
//
//  Created by Steve Gifford on 11/11/19.
//  Copyright © 2019 mousebird consulting. All rights reserved.
//

import UIKit

/**
    Convenience class for loading a Mapbox-style vector tiles-probably kinda map.
    You give it a style sheet and it figures out the rest.
    Set the various settings before it gets going to modify how it works.
    Callbacks control various pieces that might need to be intercepted.
 */
public class MapboxKindaMap {
    public var styleURL: URL?
    public weak var viewC: MaplyBaseViewController?

    // If set, we build an image/vector hybrid where the polygons go into
    //  the image layer and the linears and points are represented as vectors
    // Otherwise, it's all put in a PagingLayer as vectors.  This is better for an overlay.
    public var imageVectorHybrid = true
    
    // If set, we'll sort all polygons into the background
    // Works well zoomed out, less enticing zoomed in
    public var backgroundAllPolys = true
    
    // If set, a top level directory where we'll cache everything
    public var cacheDir: URL?
    
    // If set, you can override the file loaded for a particular purpose.
    // This includes: the TileJSON files, sprite sheets, and the style sheet itself
    // For example, if you want to load from the bundle, but not have to change
    //  anything in the style sheet, just do this
    public var fileOverride : (_ file: URL) -> URL = { return $0 }
    
    /**
         If you want to build the URL Requests yourself, maybe add some headers, change the timeout, whatever,
            just provide this function and you can do what you like.  Otherwise we just build simple URL Requests
            from the URL.
     */
    public var makeURLRequest : (_ file: URL) -> URLRequest = { (url) in return URLRequest(url: url) }
    
    // If set, we'll consult this on the font to use for a given
    //  font name in the style.  Font names in the style often don't map
    //  directly to local font names.
    public var fontOverride : (_ name: String) -> UIFontDescriptor? = { _ in return nil }
    
    // If set, this will be called right after everything is set up
    // This is after all the configuration files are fetched so
    //  you can make any final tweaks to loading objects here
    public var postSetup : (_ map: MapboxKindaMap) -> Void = { _ in }
    
    // This is the importance value used in the sampler for loading
    // It's roughly the maximum number of pixels you want a tile to be on the screen
    //  before you load its children.  1024 is good for vector tiles, 256 good for image tiles
    public var minImportance = 1024.0 * 1024.0
    
    // If set, we'll fetch and use the sources from the style sheet
    // If not set, the sources have to be provided externally
    public var fetchSources = true
    
    public init() {
    }
    
    // Initialize with a URL where the style sheet lives
    public init(_ styleURL: URL, viewC: MaplyBaseViewController) {
        self.viewC = viewC
        self.styleURL = styleURL
        styleSettings.baseDrawPriority = kMaplyImageLayerDrawPriorityDefault+1000
        styleSettings.drawPriorityPerLevel = 1
    }
    
    // Initialize with the style sheet tiself and a pointer to the MBTiles file
    public init(_ styleSheet: String, localMBTiles: String, viewC: MaplyBaseViewController) {
        self.viewC = viewC
        self.styleSheetData = styleSheet.data(using: .utf8)
        self.localMBTiles.append(localMBTiles)
    }
    
    // Initialize with a style sheet that's already been parsed
    public init(_ styleSheet: MapboxVectorStyleSet, localMBTiles: String, viewC: MaplyBaseViewController) {
        self.viewC = viewC
        self.styleSheet = styleSheet
        self.localMBTiles.append(localMBTiles)
    }
    
    public var styleSettings = MaplyVectorStyleSettings()
    public var styleSheet: MapboxVectorStyleSet?
    public var styleSheetImage: MapboxVectorStyleSet?
    public var styleSheetVector: MapboxVectorStyleSet?
    public var styleSheetData: Data?
    public var spriteJSON: Data?
    public var spritePNG: UIImage?
    public var localMBTiles = [String]()
    
    // Information about the sources as we fetch them
    public var outstandingFetches: [URLSessionDataTask?] = []
    private var finished = false
    
    // Check if we've finished loading stuff
    private func checkFinished() {
        if finished {
            return
        }

        DispatchQueue.main.async {
            if self.finished {
                return
            }
            
            var done = true
            
            // If any of the oustanding fetches are running, don't start
            self.outstandingFetches.forEach {
                if $0?.state == .running {
                    done = false
                }
            }
            
            // All done, so start
            if done {
                self.finished = true
                self.startLoader()
            }
        }
    }
    
    public var mapboxInterp: MapboxVectorInterpreter?
    public var loader: MaplyQuadImageLoader?
    public var pagingLoader: MaplyQuadPagingLoader?
    public var offlineRender: MaplyRenderController?
    
    // If we're using a cache dir, look for the file there
    private func cacheResolve(_ url: URL) -> URL {
        let fileName = cacheName(url)
        if !fileName.isFileURL || !FileManager.default.fileExists(atPath: fileName.path) {
            return url
        }
        
        return fileName
    }
    
    // Generate a workable cache file path
    private func cacheName(_ url: URL) -> URL {
        guard let cacheDir = cacheDir else {
            return url
        }
        
        // Make sure the cache dir exists
        if !FileManager.default.fileExists(atPath: cacheDir.path) {
            try? FileManager.default.createDirectory(at: cacheDir, withIntermediateDirectories: true, attributes: nil)
        }
        
        // It's already local
        if url.isFileURL {
            return url
        }
        
        // Make up a cache name from the URL
        let cacheName = url.absoluteString.replacingOccurrences(of: "/", with: "_").replacingOccurrences(of: ":", with: "_")
        let fileURL = cacheDir.appendingPathComponent(cacheName)
        
        return fileURL
    }
    
    // Write a file to cache if appropriate
    private func cacheFile(_ url: URL, data: Data) {
        // If there's no cache dir or the file is local, don't cache
        if cacheDir == nil || url.isFileURL {
            return
        }
        
        let theCacheName = cacheName(url)
        try? data.write(to: theCacheName)
    }

        // Style sheet has parsed, so get the rest of the junk
    private func styleSheetKickoff() {
        guard let styleSheet = styleSheet else {
            return
        }
        
        var success = true
        if self.fetchSources {
            // Fetch what we need to for the sources
            styleSheet.sources.forEach {
                let source = $0 as! MaplyMapboxVectorStyleSource
                if source.tileSpec == nil && success {
                    guard let urlStr = source.url,
                        let origURL = URL(string: urlStr) else {
                        print("Expecting either URL or tile info for a source.  Giving up.")
                        success = false
                        self.stop()
                        return
                    }
                    let url = self.cacheResolve(self.fileOverride(origURL))
                    
                    // Go fetch the TileJSON
                    let dataTask = URLSession.shared.dataTask(with: self.makeURLRequest(url)) { (data, resp, error) in
                        guard error == nil else {
                            print("Error trying to fetch tileJson from \(urlStr)")
                            self.stop()
                            return
                        }
                        
                        if let data = data,
                            let resp = try? JSONSerialization.jsonObject(with: data, options: []) as? [String: Any] {
                            source.tileSpec = resp
                            self.cacheFile(origURL, data: data)

                            DispatchQueue.main.async {
                                self.checkFinished()
                            }
                        }
                    }
                    self.outstandingFetches.append(dataTask)
                    dataTask.resume()
                }
            }
        }
        
        // And for the sprite sheets
        if let spriteURLStr = styleSheet.spriteURL,
            var spriteJSONurl = URL(string: spriteURLStr.appending("@2x.json")),
            var spritePNGurl = URL(string: spriteURLStr.appending("@2x.png")) {
                            spriteJSONurl = self.fileOverride(spriteJSONurl)
                            spritePNGurl = self.fileOverride(spritePNGurl)
                let dataTask1 = URLSession.shared.dataTask(with: self.makeURLRequest(self.cacheResolve(self.fileOverride(spriteJSONurl)))) { (data, _, error) in
                    guard error == nil else {
                        print("Failed to fetch spriteJSON from \(spriteURLStr)")
                        self.stop()
                        return
                    }
                    
                    if let data = data {
                        self.spriteJSON = data

                        self.cacheFile(spriteJSONurl, data: data)
                    }

                    DispatchQueue.main.async {
                        self.checkFinished()
                    }
                }
                self.outstandingFetches.append(dataTask1)
                dataTask1.resume()
            let dataTask2 = URLSession.shared.dataTask(with: self.makeURLRequest(self.cacheResolve(self.fileOverride(spritePNGurl)))) { (data, _, error) in
                    guard error == nil else {
                        print("Failed to fetch spritePNG from \(spriteURLStr)")
                        self.stop()
                        return
                    }
                    if let data = data {
                        self.spritePNG = UIImage(data: data)
                        
                        self.cacheFile(spritePNGurl, data: data)
                    }

                    DispatchQueue.main.async {
                        self.checkFinished()
                    }
                }
                self.outstandingFetches.append(dataTask2)
                dataTask2.resume()
            }
        
        if !success {
            self.stop()
        }
    }
        
    // Done messing with settings?  Then fire this puppy up
    // Will shut down the loader(s) it started
    public func start() {
        guard let viewC = viewC else {
            return
        }
        
        if let styleSheetData = styleSheetData {
            guard let styleSheet = MapboxVectorStyleSet(json: styleSheetData,
                                                        settings: self.styleSettings,
                                                        viewC: viewC) else {
                print("Failed to parse style sheet")
                self.stop()
                return
            }
            self.styleSheet = styleSheet

            self.checkFinished()

        } else if styleSheet != nil {
            // User handed it in, so move on to the next step
            styleSheetKickoff()
        } else if var styleURL = styleURL {
            // Dev might be overriding the source
            styleURL = fileOverride(styleURL)
            styleURL = cacheResolve(styleURL)
            
            // Go get the style sheet (this will also handle local
            let dataTask = URLSession.shared.dataTask(with: self.makeURLRequest(styleURL)) { (data, _, error) in
                guard error == nil, let data = data else {
                    print("Error fetching style sheet:\n\(String(describing: error))")
                    
                    self.stop()
                    return
                }
                
                DispatchQueue.main.async {
                    guard let styleSheet = MapboxVectorStyleSet(json: data,
                                                          settings: self.styleSettings,
                                                            viewC: viewC) else {
                        print("Failed to parse style sheet")
                        self.stop()
                        return
                    }
                    self.styleSheetData = data
                    self.styleSheet = styleSheet
                    self.cacheFile(self.styleURL!, data: data)
                    
                    self.styleSheetKickoff()
                }
            }
            outstandingFetches.append(dataTask)
            dataTask.resume()
        } else {
            print("Need to set styleURL or styleSheetData")
            stop()
        }
    }
    
    // Everything has been fetched, so fire up the loader
    private func startLoader() {
        guard let styleSheet = styleSheet,
            let viewC = viewC else {
            return
        }
        
        // Figure out overall min/max zoom
        var zoom : (min: Int32, max: Int32) = (10000, -1)
        
        if fetchSources {
            styleSheet.sources.forEach {
                guard let source = $0 as? MaplyMapboxVectorStyleSource else {
                    print("Bad format in tileInfo for style sheet")
                    return
                }
                if let minZoom = source.tileSpec?["minzoom"] as? Int32,
                    let maxZoom = source.tileSpec?["maxzoom"] as? Int32 {
                    zoom.min = min(minZoom, zoom.min)
                    zoom.max = max(maxZoom, zoom.max)
                }
            }
            
            // Sources probably weren't set up
            if zoom.min > zoom.max {
                print("Sources missing.  Bad zoom min/max.")
                return
            }
        }

            // Put together the tileInfoNew objects
            var tileInfos: [MaplyTileInfoNew] = []
            var localFetchers: [MaplyMBTileFetcher] = []
            if fetchSources {
                styleSheet.sources.forEach {
                    guard let source = $0 as? MaplyMapboxVectorStyleSource else {
                        print("Bad format in tileInfo for style sheet")
                        return
                    }
                    if let minZoom = source.tileSpec?["minzoom"] as? Int32,
                        let maxZoom = source.tileSpec?["maxzoom"] as? Int32,
                        let tiles = source.tileSpec?["tiles"] as? [String] {
                        let tileSource = MaplyRemoteTileInfoNew(baseURL: tiles[0], minZoom: minZoom, maxZoom: maxZoom)
                        if let cacheDir = self.cacheDir {
                            tileSource.cacheDir = cacheDir.appendingPathComponent(tiles[0].replacingOccurrences(of: "/", with: "_").replacingOccurrences(of: ":", with: "_")).absoluteString
                        }
                        tileInfos.append(tileSource)
                    }
                }
            } else {
                // Must be local files
                localMBTiles.forEach {
                    if let fetcher = MaplyMBTileFetcher(mbTiles: $0),
                        let tileInfo = fetcher.tileInfo() {
                        localFetchers.append(fetcher)
                        tileInfos.append(tileInfo)
                        zoom.min = min(fetcher.minZoom(), zoom.min)
                        zoom.max = max(fetcher.maxZoom(), zoom.max)
                    }
                }
            }
            
            // Parameters describing how we want a globe broken down
            let sampleParams = MaplySamplingParams()
            sampleParams.coordSys = MaplySphericalMercator(webStandard: ())
            sampleParams.minImportance = self.minImportance
            sampleParams.singleLevel = true
            // If we don't have a solid underlayer for each tile, we can't really
            //  keep level 0 around all the time
            if !backgroundAllPolys {
                sampleParams.forceMinLevel = false
            } else {
                sampleParams.forceMinLevel = true
                sampleParams.minImportanceTop = 0.0
            }
            if viewC is WhirlyGlobeViewController {
                sampleParams.coverPoles = true
                sampleParams.edgeMatching = true
            } else {
                sampleParams.coverPoles = false
                sampleParams.edgeMatching = false
            }
            sampleParams.minZoom = zoom.min
            sampleParams.maxZoom = zoom.max

        // Image/vector hybrids draw the polygons into a background image
        if imageVectorHybrid {
            guard let imageLoader = MaplyQuadImageLoader(params: sampleParams, tileInfos: tileInfos, viewC: viewC) else {
                print("Failed to start image loader.  Nothing will appear.")
                self.stop()
                return
            }
            // TODO: Doesn't handle more than one local source
            if !localFetchers.isEmpty {
                imageLoader.setTileFetcher(localFetchers[0])
            }
            loader = imageLoader
            
            guard let styleSheetData = styleSheetData else {
                return
            }
                        
            if self.backgroundAllPolys {
                // Set up an offline renderer and a Mapbox vector style handler to render to it
                let imageSize = (width: 512.0, height: 512.0)
                guard let offlineRender = MaplyRenderController.init(size: CGSize.init(width: imageSize.width, height: imageSize.height)) else {
                    print("Failed to start offline renderer.  Nothing will appear.")
                    self.stop()
                    return
                }
                self.offlineRender = offlineRender
                let imageStyleSettings = MaplyVectorStyleSettings.init(scale: UIScreen.main.scale)
                imageStyleSettings.baseDrawPriority = styleSettings.baseDrawPriority
                imageStyleSettings.arealShaderName = kMaplyShaderDefaultTriNoLighting

                guard var styleDictImage = try? JSONSerialization.jsonObject(with: styleSheetData, options: []) as? [String: Any] else {
                    print("Failed to parse JSON style")
                    self.stop()
                    return
                }
                
                // Leave only the background and fill layers
                if let layers = styleDictImage["layers"] as? [[String: Any]] {
                    var newLayers = [ [String: Any] ]()
                    for layer in layers {
                        if let type = layer["type"] as? String {
                            if type == "background" || type == "fill" {
                                newLayers.append(layer)
                            }
                        }
                    }
                    styleDictImage["layers"] = newLayers
                }
                
                // We only want the polygons in the image
                guard let styleSheetImage = MapboxVectorStyleSet(dict: styleDictImage,
                                                             settings: imageStyleSettings,
                                                                viewC: offlineRender) else {
                        print("Failed to set up image style sheet.  Nothing will appear.")
                        self.stop()
                        return
                }
                                
                self.styleSheetImage = styleSheetImage
            }
            
            guard var styleDictVector = try? JSONSerialization.jsonObject(with: styleSheetData, options: []) as? [String: Any] else {
                print("Failed to parse JSON style")
                self.stop()
                return
            }
            
            // If we're backgrounding all the polys, we don't want them in this version
            if self.backgroundAllPolys {
                if let layers = styleDictVector["layers"] as? [[String: Any]] {
                    var newLayers = [ [String: Any] ]()
                    for layer in layers {
                        if let type = layer["type"] as? String {
                            if type != "background" && type != "fill" {
                                newLayers.append(layer)
                            }
                        }
                    }
                    styleDictVector["layers"] = newLayers
                }
            }
            
            // Just the linear and point vectors in the overlay
            guard let styleSheetVector = MapboxVectorStyleSet(dict: styleDictVector,
                                                          settings: styleSettings,
                                                             viewC: viewC) else {
                    print("Failed to set up vector style sheet.  Nothing will appear.")
                    self.stop()
                    return
            }
            if let spriteJSON = spriteJSON,
                let image = spritePNG {
                guard let spriteDict  = try? JSONSerialization.jsonObject(with: spriteJSON, options: []) as? [String: Any] else {
                    print("Failed to parse sprite sheet JSON")
                    self.stop()
                    return
                }

                if !styleSheetVector.addSprites(spriteDict, image: image) {
                    print("Failed to parse sprite sheet.")
                    self.stop()
                    return
                }
                
                // Need this associated if we're looking at the sprite sheet for info
                styleSheet.addSprites(spriteDict, image: image)
            }
            self.styleSheetVector = styleSheetVector

            if !(viewC is WhirlyGlobeViewController) {
                // Set the background clear to the color at level 0
                // TODO: Make this change by level
                if let color = styleSheetVector.backgroundColor(forZoom: 0.0) {
                    viewC.clearColor = color
                }
            }

            if let offlineRender = offlineRender,
                let styleSheetImage = styleSheetImage {
                // The interpreter does the work off offline render and conversion to WG-Maply objects
                guard let mapboxInterp = MapboxVectorInterpreter(imageStyle: styleSheetImage,
                                                            offlineRender: offlineRender,
                                                            vectorStyle: styleSheetVector,
                                                                viewC: viewC) else {
                     print("Failed to set up Mapbox interpreter.  Nothing will appear.")
                     self.stop()
                     return
                }
                self.mapboxInterp = mapboxInterp
            } else {
                // Simpler overlay
                guard let mapboxInterp = MapboxVectorInterpreter(vectorStyle: styleSheetVector,
                                                                 viewC: viewC) else {
                     print("Failed to set up Mapbox interpreter.  Nothing will appear.")
                     self.stop()
                     return
                }
                self.mapboxInterp = mapboxInterp
            }
            imageLoader.setInterpreter(self.mapboxInterp!)
            
        } else {
            // This version is just a simple overlay
            // So we don't expect full coverage
            
            // Deal with the sprite sheets if they're present
            if let spriteJSON = spriteJSON,
                let image = spritePNG {
                guard let spriteDict  = try? JSONSerialization.jsonObject(with: spriteJSON, options: []) as? [String: Any] else {
                    print("Failed to parse sprite sheet JSON")
                    self.stop()
                    return
                }

                if !styleSheet.addSprites(spriteDict, image: image) {
                    print("Failed to parse sprite sheet.")
                    self.stop()
                    return
                }
            }
            // Interpreter for mapbox data
            guard let mapboxInterp = MapboxVectorInterpreter(vectorStyle: styleSheet,
                                                             viewC: viewC) else {
                 print("Failed to set up Mapbox interpreter.  Nothing will appear.")
                 self.stop()
                 return
            }
            self.mapboxInterp = mapboxInterp

            // A simple paging loader, which will act as an overlay
            if let pagingLoader = MaplyQuadPagingLoader(params: sampleParams,
                                                        tileInfo: tileInfos[0],
                                                        loadInterp: mapboxInterp,
                                                        viewC: viewC) {
                pagingLoader.flipY = false
                if !localFetchers.isEmpty {
                    pagingLoader.setTileFetcher(localFetchers[0])
                }
                self.pagingLoader = pagingLoader
            }
        }
        
        postSetup(self)
    }
    
    public func stop() {
        // If we're still fetching config data, cancel that
        outstandingFetches.forEach {
            $0?.cancel()
        }
        outstandingFetches = []

        loader?.shutdown()
        loader = nil
        pagingLoader?.shutdown()
        pagingLoader = nil
        mapboxInterp = nil
    }
}
