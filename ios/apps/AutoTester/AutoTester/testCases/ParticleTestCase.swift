//
//  ParticleTestCase.swift
//  AutoTester
//
//  Created by jmnavarro on 18/1/16.
//  Copyright © 2016-2017 mousebird consulting. All rights reserved.
//
import UIKit

class ParticleTestCase: MaplyTestCase {
	
	override init() {
		super.init()
		
		self.name = "Wind Particle Test"
		self.captureDelay = 5
		self.implementations = [.globe]
	}
	
	fileprivate func setUpOverlay (_ baseView: MaplyBaseViewController) {
		let partDelegate = ParticleTileDelegate(url: "http://tilesets.s3-website-us-east-1.amazonaws.com/wind_test/{dir}_tiles/{z}/{x}/{y}.png", minZoom: Int32(2), maxZoom: Int32(5), viewC: baseView)
		let layer = MaplyQuadPagingLayer(coordSystem: (partDelegate?.coordSys)!, delegate: partDelegate!)
		layer?.flipY = false;
		baseView.add(layer!)
	}
	
	override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
		let baseLayer = CartoDBLightTestCase()
		baseLayer.setUpWithGlobe(globeVC)
		setUpOverlay(globeVC)
		globeVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(-122.4192, 37.7793) , time: 1.0)
	}

}
