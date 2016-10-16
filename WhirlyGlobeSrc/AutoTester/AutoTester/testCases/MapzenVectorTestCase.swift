//
//  MapzenVectorTestCase.swift
//  AutoTester
//
//  Created by jmnavarro on 29/10/15.
//  Copyright © 2015 mousebird consulting. All rights reserved.
//

import UIKit

class MapzenVectorTestCase: MaplyTestCase {

	override init() {
		super.init()

		self.name = "Mapzen Vectors"
		self.captureDelay = 5
		self.implementations = [ .Map, .Globe]
	}
    
    func setupMapzenLayer(baseVC: MaplyBaseViewController)
    {
        //let styleData = NSData(contentsOfFile: NSBundle.mainBundle().pathForResource("MapzenGLStyle", ofType: "json")!)
        
        
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, Int64(1 * Double(NSEC_PER_SEC))), dispatch_get_main_queue(), {
            
            let styleData = NSData(contentsOfFile: NSBundle.mainBundle().pathForResource("MapzenSLDStyle", ofType: "sld")!)
            
            let mzSource = MapzenSource(
                base: "http://vector.mapzen.com/osm",
                layers: ["all"],
                apiKey: "vector-tiles-ejNTZ28",
                sourceType: MapzenSourcePBF,
                styleData: styleData,
                styleType: .SLDStyle,
                viewC: baseVC)
            
            mzSource.minZoom = Int32(0)
            mzSource.maxZoom = Int32(24)
            
            let pageLayer = MaplyQuadPagingLayer(
                coordSystem: MaplySphericalMercator(),
                delegate: mzSource)
            
            pageLayer?.numSimultaneousFetches = Int32(8)
            pageLayer?.flipY = false
            pageLayer?.importance = 512*512
            pageLayer?.useTargetZoomLevel = true
            pageLayer?.singleLevelLoading = true
            baseVC.addLayer(pageLayer!)
            
        });
    }
    
    override func setUpWithGlobe(globeVC: WhirlyGlobeViewController) {
        let baseLayer = CartoDBLightTestCase()
        baseLayer.setUpWithGlobe(globeVC)

        setupMapzenLayer(globeVC)

        globeVC.animateToPosition(MaplyCoordinateMakeWithDegrees(-122.290,37.7793), height: 0.0005, heading: 0.0, time: 0.1)
    }

	override func setUpWithMap(mapVC: MaplyViewController) {
		let baseLayer = CartoDBLightTestCase()
		baseLayer.setUpWithMap(mapVC)
        
        setupMapzenLayer(mapVC)
        
        mapVC.animateToPosition(MaplyCoordinateMakeWithDegrees(-122.290,37.7793), height: 0.0005, time: 0.1)
	}

}
