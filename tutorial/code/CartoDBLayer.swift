//
//  CartoDBLayer.swift
//  HelloEarth
//
//  Created by jmnavarro on 17/08/15.
//  Copyright (c) 2015 Mousebird. All rights reserved.
//

import Foundation

class CartoDBLayer: NSObject, MaplyPagingDelegate {

	private var _minZoom = Int32(0)
	private var _maxZoom = Int32(0)

	private var search: String
	private var opQueue: NSOperationQueue?

	// Create with the search string we'll use
	init(search: String) {
		self.search = search
		self.opQueue = NSOperationQueue()

		super.init()
	}

	func minZoom() -> Int32 {
		return _minZoom
	}

	func maxZoom() -> Int32 {
		return _maxZoom
	}

	func setMinZoom(value: Int32) {
		_minZoom = value
	}

	func setMaxZoom(value: Int32) {
		_maxZoom = value
	}

	func startFetchForTile(tileID: MaplyTileID, forLayer layer: MaplyQuadPagingLayer) {
		// bounding box for tile
		let bbox = layer.boundsForTile(tileID)
		let urlReq = constructRequest(bbox)

		let session = NSURLSession(configuration: NSURLSessionConfiguration.defaultSessionConfiguration())
		session.dataTaskWithRequest(urlReq) { (data, response, error) in
			// parse the resulting GeoJSON
			let str = NSString(data: data!, encoding: NSUTF8StringEncoding)
			print(str)
			if let data = data,
				vecObj = MaplyVectorObject(fromGeoJSON: data) {
				// display a transparent filled polygon
				let filledObj = layer.viewC!.addVectors([vecObj],
				                                        desc: [
															kMaplyColor: UIColor(red: 0.25, green: 0.0, blue: 0.0, alpha: 0.25),
															kMaplyFilled: true,
															kMaplyEnable: false],
				                                        mode: .Current)

				// display a line around the lot
				let outlineObj = layer.viewC!.addVectors([vecObj],
				                                         desc: [
															kMaplyColor: UIColor.redColor(),
															kMaplyFilled: false,
															kMaplyEnable: false],
				                                         mode: .Current)

				// keep track of it in the layer
				layer.addData([filledObj!, outlineObj!], forTile: tileID)

				// let the layer know the tile is done
				layer.tileDidLoad(tileID)
			}
			}.resume()
	}

	func constructRequest(bbox: MaplyBoundingBox) -> NSURLRequest {
		let toDeg = Float(180.0/M_PI)
		let query = NSString(format: search, bbox.ll.x * toDeg, bbox.ll.y * toDeg,bbox.ur.x * toDeg, bbox.ur.y * toDeg)
		var encodeQuery = query.stringByAddingPercentEncodingWithAllowedCharacters(NSCharacterSet.URLQueryAllowedCharacterSet())
		let range = encodeQuery!.startIndex..<encodeQuery!.endIndex
		encodeQuery = encodeQuery!.stringByReplacingOccurrencesOfString("&", withString: "%26", options: [], range: range)
		let fullUrl = NSString(format: "https://pluto.cartodb.com/api/v2/sql?format=GeoJSON&q=%@", encodeQuery!) as String
		return NSURLRequest(URL: NSURL(string: fullUrl)!)
	}

}
