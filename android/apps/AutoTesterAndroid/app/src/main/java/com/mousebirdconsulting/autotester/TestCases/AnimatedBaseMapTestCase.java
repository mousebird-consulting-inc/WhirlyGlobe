/*
 *  AnimatedBaseMapTestCase.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/22/19.
 *  Copyright 2011-2019 mousebird consulting
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;

import com.mousebird.maply.*;
import com.mousebirdconsulting.autotester.ConfigOptions;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;
import com.mousebirdconsulting.autotester.R;
import com.mousebirdconsulting.autotester.WeatherShader;

import java.io.File;


public class AnimatedBaseMapTestCase extends MaplyTestCase {

	public AnimatedBaseMapTestCase(Activity activity) {
		super(activity);
		setTestName("Animated basemap");
		this.implementation = TestExecutionImplementation.Both;
	}

	QuadImageFrameLoader loader = null;
	QuadImageFrameAnimator animator = null;
	Shader shader = null;

	static String vertexShaderTriMultiTex =
		"    precision highp float;\n" +
		"\n" +
		"    uniform mat4  u_mvpMatrix;\n" +
		"    uniform float u_fade;\n" +
		"    attribute vec3 a_position;\n" +
		"    attribute vec2 a_texCoord0;\n" +
		"    attribute vec4 a_color;\n" +
		"    attribute vec3 a_normal;\n" +
		"\n" +
		"    uniform vec2 u_texOffset0;\n" +
		"    uniform vec2 u_texScale0;\n" +
		"    uniform vec2 u_texOffset1;\n" +
		"    uniform vec2 u_texScale1;\n" +
		"\n" +
		"    varying vec2 v_texCoord0;\n" +
		"    varying vec2 v_texCoord1;\n" +
		"    varying vec4 v_color;\n" +
		"\n" +
		"    void main()\n" +
		"    {\n" +
		"        if (u_texScale0.x != 0.0)\n" +
		"            v_texCoord0 = vec2(a_texCoord0.x*u_texScale0.x,a_texCoord0.y*u_texScale0.y) + u_texOffset0;\n" +
		"        else\n" +
		"            v_texCoord0 = a_texCoord0;\n" +
		"        if (u_texScale1.x != 0.0)\n" +
		"            v_texCoord1 = vec2(a_texCoord0.x*u_texScale1.x,a_texCoord0.y*u_texScale1.y) + u_texOffset1;\n" +
		"        else\n" +
		"            v_texCoord1 = a_texCoord0;\n" +
		"       v_color = a_color * u_fade;\n" +
		"    \n" +
		"       gl_Position = u_mvpMatrix * vec4(a_position,1.0);\n" +
		"    }\n";

	static String fragmentShaderTriMultiTex =
		"    precision highp float;\n" +
		"\n" +
		"    uniform sampler2D s_baseMap0;\n" +
		"    uniform sampler2D s_baseMap1;\n" +
		"    uniform sampler2D s_colorRamp;\n" +
		"    uniform float u_interp;\n" +
		"\n" +
		"    varying vec2      v_texCoord0;\n" +
		"    varying vec2      v_texCoord1;\n" +
		"    varying vec4      v_color;\n" +
		"\n" +
		"    void main()\n" +
		"    {\n" +
		"      float baseVal0 = texture2D(s_baseMap0, v_texCoord0).r;\n" +
		"      float baseVal1 = texture2D(s_baseMap1, v_texCoord1).r;\n" +
		"      float index = mix(baseVal0,baseVal1,u_interp);\n" +
		"      gl_FragColor = texture2D(s_colorRamp,vec2(index,0.5));\n" +
		"    }\n";

	private void setupImageLoader(BaseController vc, ConfigOptions.TestType testType) throws Exception {
		// Five frames, cached locally
		String cacheDirName = "forecastio";
		File cacheDir = new File(getActivity().getCacheDir(), cacheDirName);
		cacheDir.mkdir();
		RemoteTileInfoNew[] sources = new RemoteTileInfoNew[5];
		for (int i = 0; i < 5; i++) {
			sources[i] = new RemoteTileInfoNew("http://a.tiles.mapbox.com/v3/mousebird.precip-example-layer" + i + "/{z}/{x}/{y}.png", 0, 6);
		}

		SamplingParams params = new SamplingParams();
		params.setCoordSystem(new SphericalMercatorCoordSystem());
		if (testType == ConfigOptions.TestType.GlobeTest) {
			params.setCoverPoles(true);
			params.setEdgeMatching(true);
		} else {
			params.setCoverPoles(false);
			params.setEdgeMatching(false);
		}
		params.setMinZoom(0);
		params.setMaxZoom(6);

		// Custom shader to turn these into colors
		shader = new Shader("Weather Shader", vertexShaderTriMultiTex, fragmentShaderTriMultiTex, vc);
		vc.addShaderProgram(shader);
		Bitmap colorRamp = BitmapFactory.decodeResource(getActivity().getResources(), R.drawable.colorramp);
		MaplyTexture tex = vc.addTexture(colorRamp,null, RenderControllerInterface.ThreadMode.ThreadCurrent);
		shader.addTexture("s_colorRamp", tex);

		loader = new QuadImageFrameLoader(params,sources,vc);
		loader.setDrawPriorityPerLevel(QuadImageLoaderBase.BaseDrawPriorityDefault+1000);
		loader.setShader(shader);
		animator = new QuadImageFrameAnimator(loader,vc);
		animator.period = 6.0;
	}

	CartoLightTestCase baseCase = null;

	@Override
	public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
		baseCase = new CartoLightTestCase(this.getActivity());
		baseCase.setUpWithGlobe(globeVC);
		setupImageLoader(globeVC, ConfigOptions.TestType.GlobeTest);
		return true;
	}

	@Override
	public boolean setUpWithMap(MapController mapVC) throws Exception {
		baseCase = new CartoLightTestCase(this.getActivity());
		baseCase.setUpWithMap(mapVC);
		setupImageLoader(mapVC, ConfigOptions.TestType.MapTest);
		return true;
	}
}
