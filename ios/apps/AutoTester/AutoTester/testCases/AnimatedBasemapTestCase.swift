//
//  AnimatedBasemapTestCase.swift
//  AutoTester
//
//  Created by jmnavarro on 26/10/15.
//  Copyright © 2015-2017 mousebird consulting. All rights reserved.
//

import UIKit

class AnimatedBasemapTestCase: MaplyTestCase {

	let geographyClass = GeographyClassTestCase()
	let cacheDir = NSSearchPathForDirectoriesInDomains(.cachesDirectory, .userDomainMask, true)[0]
    var imageLayer : MaplyQuadImageFrameLoader? = nil

	override init() {
		super.init()

		self.name = "Animated basemap"
		self.captureDelay = 2
		self.implementations = [.globe, .map]
	}

    // Note: Rather than copying the shader code in here, we should have a way to look it up
    let vertexShaderNoLightTri = """
    uniform mat4  u_mvpMatrix;
    uniform float u_fade;
    attribute vec3 a_position;
    attribute vec2 a_texCoord0;
    attribute vec4 a_color;
    attribute vec3 a_normal;

    varying vec2 v_texCoord;
    varying vec4 v_color;

    void main()
    {
       v_texCoord = a_texCoord0;
       v_color = a_color * u_fade;
    
       gl_Position = u_mvpMatrix * vec4(a_position,1.0);
    }
    """
    
    let fragmentShaderTriMultiTexRamp = """
    precision mediump float;

    uniform sampler2D s_baseMap0;
    uniform sampler2D s_baseMap1;
    uniform sampler2D s_colorRamp;
    uniform float u_interp;

    varying vec2      v_texCoord;
    varying vec4      v_color;

    void main()
    {
      float baseVal0 = texture2D(s_baseMap0, v_texCoord).r;
      float baseVal1 = texture2D(s_baseMap1, v_texCoord).r;
      float index = mix(baseVal0,baseVal1,u_interp);
      gl_FragColor = texture2D(s_colorRamp,vec2(index,0.5));
    }
    """

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
        
        // Parameters describing how we want a globe broken down
        let sampleParams = MaplySamplingParams()
        sampleParams.coordSys = MaplySphericalMercator(webStandard: ())
        sampleParams.coverPoles = false
        sampleParams.edgeMatching = true
        sampleParams.minZoom = 0
        sampleParams.maxZoom = 6
        sampleParams.singleLevel = true
        sampleParams.minImportance = 1024.0*1024.0

        imageLayer = MaplyQuadImageFrameLoader(params: sampleParams, tileInfos: tileSources, viewC: baseVC)
        imageLayer?.baseDrawPriority = kMaplyImageLayerDrawPriorityDefault + 1000

        // Color ramp shader
        if let colorRamp = UIImage(named: "colorramp.png"),
            let shader = MaplyShader(name: "AnimatedBasemap Shader",
                                     vertex: vertexShaderNoLightTri,
                                     fragment: fragmentShaderTriMultiTexRamp,
                                     viewC: baseVC) {
            baseVC.addShaderProgram(shader, sceneName: "AnimatedBasemap Shader")
            imageLayer?.setShader(shader)
            shader.addTextureNamed("s_colorRamp", image: colorRamp)
        }
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
