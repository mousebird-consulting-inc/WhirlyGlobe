//
//  AnimatedBasemapTestCase.swift
//  AutoTester
//
//  Created by jmnavarro on 26/10/15.
//  Copyright © 2015-2017 mousebird consulting.
//

import UIKit

class AnimatedBasemapTestCase: MaplyTestCase {

	let geographyClass = GeographyClassTestCase()
	let cacheDir = NSSearchPathForDirectoriesInDomains(.cachesDirectory, .userDomainMask, true)[0]
    var imageLayer : MaplyQuadImageFrameLoader? = nil
    var imageAnimator : MaplyQuadImageFrameAnimator? = nil
    var varTarget : MaplyVariableTarget? = nil

	override init() {
		super.init()

		self.name = "Animated basemap"
		self.implementations = [.globe, .map]
	}
    
    var rampTex : MaplyTexture? = nil
    
    var timer : Timer? = nil

    // Put together a sampling layer and loader
    func setupLoader(_ baseVC: MaplyBaseViewController) {
        var tileSources : [MaplyRemoteTileInfoNew] = []

        for i in 0...4 {
            let precipSource = MaplyRemoteTileInfoNew(baseURL: "http://a.tiles.mapbox.com/v3/mousebird.precip-example-layer\(i)/{z}/{x}/{y}.png",
                minZoom: 0,
                maxZoom: 6)
            precipSource.cacheDir = "\(cacheDir)/forecast_io_weather_layer\(i)/"
            tileSources.append(precipSource)
        }
        
        // Set up a variable target for two pass rendering
        varTarget = MaplyVariableTarget(type: .imageIntRGBA, viewC: baseVC)
        varTarget?.setScale(0.5)
        varTarget?.clearEveryFrame = true
        varTarget?.drawPriority = kMaplyImageLayerDrawPriorityDefault + 1000
        
        // Parameters describing how we want a globe broken down
        let sampleParams = MaplySamplingParams()
        sampleParams.coordSys = MaplySphericalMercator(webStandard: ())
        sampleParams.coverPoles = false
        sampleParams.edgeMatching = false
        sampleParams.maxZoom = 6
        sampleParams.singleLevel = true
        sampleParams.minImportance = 1024.0*1024.0

        imageLayer = MaplyQuadImageFrameLoader(params: sampleParams, tileInfos: tileSources, viewC: baseVC)
//        imageLayer?.debugMode = true;
        if let varTarget = varTarget {
            imageLayer?.setRenderTarget(varTarget.renderTarget)
        }

        guard let shader = baseVC.getShaderByName(kMaplyShaderDefaultTriMultiTexRamp) else {
            return
        }
        imageLayer?.setShader(shader)
        
        // Assign the ramp texture to the first entry in the texture lookup slot
        guard let rampTex = baseVC.addTexture(UIImage.init(named: "colorramp.png")!, desc: nil, mode: .current) else {
            return
        }
        shader.setTexture(rampTex, for: 0, viewC: baseVC)
        
        // Animator
        imageAnimator = MaplyQuadImageFrameAnimator(frameLoader: imageLayer!, viewC: baseVC)
        imageAnimator?.period = 15.0
        imageAnimator?.pauseLength = 3.0
        
        // Periodic stats
        timer = Timer.scheduledTimer(withTimeInterval: 2.0, repeats: true)
        {
            timer in
            let stats = self.imageLayer!.getFrameStats()
            print("Loading stats")
            var which = 0
            for frame in stats.frames {
                print("frame \(which):  \(frame.tilesToLoad) to load of \(frame.totalTiles))")
                which += 1
            }
        }
        
        // Color changing test
//        imageLayer?.color = UIColor.green
//        DispatchQueue.main.asyncAfter(deadline: .now() + 10.0) {
//            self.imageLayer?.color = UIColor.blue
//        }
    }

	override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
        geographyClass.setUpWithGlobe(globeVC)
        setupLoader(globeVC)
	}

	override func setUpWithMap(_ mapVC: MaplyViewController) {
        geographyClass.setUpWithMap(mapVC)
        setupLoader(mapVC)
	}

}
