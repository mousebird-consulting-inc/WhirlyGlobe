//
//  PagingTestDelegate.swift
//  WhirlyGlobeSwiftTester
//
//  Created by jmnavarro on 25/09/15.
//  Copyright (c) 2015 Mousebird. All rights reserved.
//

import Foundation


let NumMarkers = 200
let MaxDelay = 1.0


/** @details Generates lots of random markers for testing.
*/
class PagingTestDelegate: NSObject, MaplyPagingDelegate {

	var coordSys: MaplyCoordinateSystem?

	private var image: UIImage

	override init() {
		image = UIImage(named: "map_pin")!
		coordSys = MaplySphericalMercator()
		super.init()
	}

	func minZoom() -> Int32 {
		return 0
	}

	func maxZoom() -> Int32 {
		return 22
	}

	func startFetchForTile(tileID: MaplyTileID, forLayer layer: MaplyQuadPagingLayer) {
		let bbox = layer.geoBoundsForTile(tileID)

		dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0)) {
			// Random delay
			usleep(UInt32(drand48() * MaxDelay * 1e6))

			var markers = [MaplyScreenMarker]()

			for _ in 0..<NumMarkers {
				let marker = MaplyScreenMarker()
				marker.layoutImportance = MAXFLOAT
				marker.image = self.image
				marker.size = CGSizeMake(32.0, 32.0);

				let lon = Double(bbox.ur.x - bbox.ll.x) * drand48() + Double(bbox.ll.x)
				let lat = Double(bbox.ur.y - bbox.ll.y) * drand48() + Double(bbox.ll.y)
				marker.loc = MaplyCoordinateMake(Float(lon), Float(lat))

				markers.append(marker)
			}

			let compObj = layer.viewC?.addScreenLabels(markers,
				desc: [kMaplyEnable: false],
				mode: .Current)
			layer.addData([compObj!], forTile: tileID)
			layer.tileDidLoad(tileID)
		}
	}


}
