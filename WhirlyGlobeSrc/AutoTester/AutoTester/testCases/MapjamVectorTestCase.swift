//
//  MapjamVectorTestCase.swift
//  AutoTester
//
//  Created by Steve Gifford on 4/18/16.
//  Copyright © 2016 mousebird consulting. All rights reserved.
//

import Foundation

class MapjamVectorTestCase: MaplyTestCase {
    
    override init() {
        super.init()
        
        self.name = "Mapjam Vectors"
        self.captureDelay = 5
		self.implementations = [.Map]
    }
    
    override func setUpWithMap(mapVC: MaplyViewController) {
        let styleData = NSData(contentsOfFile: NSBundle.mainBundle().pathForResource("mapjam-street-map", ofType: "json")!)
        
        let mzSource = MapjamSource(base: "https://tiles-d.mapjam.com",
                                    apiKey: "mapjam_guest_tiles",
                                    styleJSON: styleData,
                                    viewC: mapVC)
        
        mapVC.clearColor = mzSource.backgroundColor
        
        mzSource.minZoom = Int32(0)
        mzSource.maxZoom = Int32(18)
        
        let pageLayer = MaplyQuadPagingLayer(
            coordSystem: MaplySphericalMercator(),
            delegate: mzSource)
        
        pageLayer?.numSimultaneousFetches = Int32(8)
        pageLayer?.flipY = false
        pageLayer?.importance = 1024*1024
        pageLayer?.useTargetZoomLevel = true
        pageLayer?.singleLevelLoading = true
        mapVC.addLayer(pageLayer!)
        mapVC.animateToPosition(MaplyCoordinateMakeWithDegrees(-122.290,37.7793), height: 0.0005, time: 0.1)
    }
    
}
