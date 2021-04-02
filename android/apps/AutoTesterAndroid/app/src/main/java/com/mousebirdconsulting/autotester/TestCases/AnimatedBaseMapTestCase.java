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
import android.os.Handler;
import android.util.Log;

import com.mousebird.maply.*;
import com.mousebirdconsulting.autotester.ConfigOptions;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;
import com.mousebirdconsulting.autotester.R;

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
	VariableTarget varTarget = null;

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

		// Use two pass rendering to sort out priorities
		varTarget = new VariableTarget(vc);
		varTarget.scale = 0.5;
		varTarget.drawPriority = QuadImageLoaderBase.BaseDrawPriorityDefault+2000;

		// Ramp shader turns these into colors
		shader = vc.getShader(Shader.DefaultTriMultiTexRampShader);
		Bitmap colorRamp = BitmapFactory.decodeResource(getActivity().getResources(), R.drawable.colorramp);
		MaplyTexture tex = vc.addTexture(colorRamp,null, RenderControllerInterface.ThreadMode.ThreadCurrent);
		shader.addTexture("s_colorRamp", tex);

		loader = new QuadImageFrameLoader(params,sources,vc);
		loader.setBaseDrawPriority(QuadImageLoaderBase.BaseDrawPriorityDefault+1000);
		loader.setShader(shader);
		loader.setRenderTarget(varTarget.renderTarget);
		animator = new QuadImageFrameAnimator(loader,vc);
		animator.period = 6.0;

		// Test shutting things down
//		final Handler handler = new Handler();
//		handler.postDelayed(new Runnable() {
//			@Override
//			public void run() {
//				animator.shutdown();
//				loader.shutdown();
//			}
//		}, 10000);

		// Print out loading stats periodically
		final Handler timeHandler = new Handler();
		timeRun = new Runnable(){
			@Override
			public void run() {
				TileFetcher tileFetcher = loader.getTileFetcher();
				if (tileFetcher instanceof RemoteTileFetcher) {
					RemoteTileFetcher remoteTileFetcher = (RemoteTileFetcher)tileFetcher;
					remoteTileFetcher.getStats(false).dump("Animated Layer");
				}

				QuadImageFrameLoader.Stats stats = loader.getStats();
				Log.v("Maply", String.format("numTiles = %d, numFrames = %d",stats.numTiles,stats.frameStats.length));
				for (int ii=0;ii<stats.frameStats.length;ii++) {
					Log.v("Maply", String.format("  Loading %d out of %d tiles",stats.frameStats[ii].tilesToLoad,stats.frameStats[ii].totalTiles));
				}

				if (controller != null && controller.isRunning())
					timeHandler.postDelayed(timeRun, 3000);
			}
		};
		timeHandler.postDelayed(timeRun,1000);
	}

	Runnable timeRun = null;

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
