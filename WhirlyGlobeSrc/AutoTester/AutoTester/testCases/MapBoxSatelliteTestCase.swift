//
//  MapBoxSatelliteTestCase.swift
//  AutoTester
//
//  Created by jmnavarro on 24/10/15.
//  Copyright © 2015 mousebird consulting. All rights reserved.
//

import UIKit

@objc class MapBoxSatelliteTestCase: MaplyTestCase {

	let jsonTileSpec = "http://a.tiles.mapbox.com/v3/examples.map-zyt2v9k2.json"
	let cacheDir = NSSearchPathForDirectoriesInDomains(.CachesDirectory, .UserDomainMask, true)[0]
	let zoomLimit = Int32(0)

	override init() {
		super.init()

		self.name = "MapBox Satellite"
		self.captureDelay = 4
	}

	override func setUpWithGlobe(globeVC: WhirlyGlobeViewController) -> Bool {
		let thisCacheDir = "\(cacheDir)/stamentiles/"
        
        guard let endpoint = NSURL(string: jsonTileSpec) else { print("Error creating endpoint"); return false }
        let request = NSMutableURLRequest(URL:endpoint)
        NSURLSession.sharedSession().dataTaskWithRequest(request) { (data, response, error) -> Void in
            do
            {
                guard let dat = data else { print("ERROR: no data"); return }
                guard let json = try NSJSONSerialization.JSONObjectWithData(dat, options: []) as? NSDictionary else { print("ERROR: conversion from JSON failed"); return }
                let tileSource = MaplyRemoteTileSource(tilespec: json as [NSObject : AnyObject])
                tileSource!.cacheDir = thisCacheDir
                if self.zoomLimit != 0 && self.zoomLimit < tileSource!.maxZoom() {
                    tileSource!.tileInfo.maxZoom = self.zoomLimit
                }
                let layer = MaplyQuadImageTilesLayer(tileSource: tileSource!)
                layer!.handleEdges = true
                layer!.waitLoad = false
                layer!.requireElev = false
                layer!.maxTiles = 256
                globeVC.heading = 0.0
                globeVC.addLayer(layer!)
                layer!.drawPriority = kMaplyImageLayerDrawPriorityDefault
                //Rotate to Position (Madrid)
                globeVC.animateToPosition(MaplyCoordinateMakeWithDegrees(-3.6704803, 40.5023056), time: 1.0)
                //Zoom to much closer position
                globeVC.height = 0.1
                //Zoom to another Position (Sidney)
                globeVC.animateToPosition(MaplyCoordinateMakeWithDegrees(151.211111, -33.859972), time: 1.0)
            }
            catch
            {
            }
        }.resume()
		return true
	}
	
	override func setUpWithMap(mapVC: MaplyViewController) -> Bool {
		let thisCacheDir = "\(cacheDir)/stamentiles/"
        
        guard let endpoint = NSURL(string: jsonTileSpec) else { print("Error creating endpoint"); return false }
        let request = NSMutableURLRequest(URL:endpoint)
        NSURLSession.sharedSession().dataTaskWithRequest(request) { (data, response, error) -> Void in
            do
            {
                guard let dat = data else { print("ERROR: no data"); return }
                guard let json = try NSJSONSerialization.JSONObjectWithData(dat, options: []) as? NSDictionary else { print("ERROR: conversion from JSON failed"); return }
                let tileSource = MaplyRemoteTileSource(tilespec: json as [NSObject : AnyObject])
                tileSource!.cacheDir = thisCacheDir
                if self.zoomLimit != 0 && self.zoomLimit < tileSource!.maxZoom() {
                    tileSource!.tileInfo.maxZoom = self.zoomLimit
                }
                let layer = MaplyQuadImageTilesLayer(tileSource: tileSource!)
                layer!.handleEdges = true
                layer!.waitLoad = false
                layer!.requireElev = false
                layer!.maxTiles = 256
                
                mapVC.addLayer(layer!)
                layer!.drawPriority = kMaplyImageLayerDrawPriorityDefault
                //Rotate to Position
                mapVC.animateToPosition(MaplyCoordinateMakeWithDegrees(-3.6704803, 40.5023056), time: 1.0)
                //Zoom to much closer position
                mapVC.height = 0.1
                //Zoom to another Position
                mapVC.animateToPosition(MaplyCoordinateMakeWithDegrees(151.211111, -33.859972), time: 1.0)
            }
            catch
            {
            }
        }.resume()
		return true
	}
}
