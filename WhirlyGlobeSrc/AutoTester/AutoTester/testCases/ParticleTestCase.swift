//
//  ParticleTestCase.swift
//  AutoTester
//
//  Created by jmnavarro on 18/1/16.
//  Copyright © 2016 mousebird consulting. All rights reserved.
//
import UIKit

class ParticleTestCase: MaplyTestCase {
	
	override init() {
		super.init()
		
		self.name = "Wind Particle Test"
		self.captureDelay = 5
		self.implementations = [.Globe]
	}
	
	private func setUpOverlay (baseView: MaplyBaseViewController) {
		let partDelegate = ParticleTileDelegate(URL: "http://tilesets.s3-website-us-east-1.amazonaws.com/wind_test/{dir}_tiles/{z}/{x}/{y}.png", minZoom: Int32(2), maxZoom: Int32(5), viewC: baseView)
		let layer = MaplyQuadPagingLayer(coordSystem: partDelegate.coordSys, delegate: partDelegate)
		layer?.flipY = false;
		baseView.addLayer(layer!)
	}
	
	override func setUpWithGlobe(globeVC: WhirlyGlobeViewController) {
		let baseLayer = CartoDBLightTestCase()
		baseLayer.setUpWithGlobe(globeVC)
		setUpOverlay(globeVC)
		globeVC.animateToPosition(MaplyCoordinateMakeWithDegrees(-122.4192, 37.7793) , time: 1.0)
	}

}
