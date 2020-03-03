/*
 *  StamenRemoteTestCase.java
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
import android.graphics.Color;
import android.os.Handler;

import com.mousebird.maply.BaseController;
import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MapController;
import com.mousebird.maply.QuadImageLoader;
import com.mousebird.maply.RemoteTileInfoNew;
import com.mousebird.maply.RenderController;
import com.mousebird.maply.SamplingParams;
import com.mousebird.maply.SphericalMercatorCoordSystem;
import com.mousebirdconsulting.autotester.ConfigOptions;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;

import java.io.File;


public class StamenRemoteTestCase extends MaplyTestCase {

	public StamenRemoteTestCase(Activity activity) {
		super(activity);

		setTestName("Stamen Watercolor Remote");
		setDelay(4);
		this.implementation = TestExecutionImplementation.Both;
	}

	private QuadImageLoader setupImageLoader(ConfigOptions.TestType testType, BaseController baseController) {
		String cacheDirName = "stamen_watercolor6";
		File cacheDir = new File(getActivity().getCacheDir(), cacheDirName);
		cacheDir.mkdir();

		RemoteTileInfoNew tileInfo = new RemoteTileInfoNew("http://tile.stamen.com/watercolor/{z}/{x}/{y}.png",0, 18);
		tileInfo.cacheDir = cacheDir;

		SamplingParams params = new SamplingParams();
		params.setCoordSystem(new SphericalMercatorCoordSystem());
		params.setCoverPoles(true);
		params.setEdgeMatching(true);
		params.setMinZoom(tileInfo.minZoom);
		params.setMaxZoom(tileInfo.maxZoom);
		params.setSingleLevel(true);

		QuadImageLoader loader = new QuadImageLoader(params,tileInfo,baseController);
		loader.setImageFormat(RenderController.ImageFormat.MaplyImageUShort565);
		loader.setDebugMode(true);

		// Change the color and then change it back
		final Handler handler = new Handler();
		handler.postDelayed(new Runnable() {
			@Override
			public void run() {
				loader.setColor(Color.RED);
				final Handler handler = new Handler();
				handler.postDelayed(new Runnable() {
					@Override
					public void run() {
						loader.setColor(Color.WHITE);
					}
				}, 4000);
			}
		}, 4000);

		return loader;
	}

	@Override
	public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
		setupImageLoader(ConfigOptions.TestType.GlobeTest,globeVC);
		globeVC.setClearColor(Color.RED);
		globeVC.animatePositionGeo(-3.6704803, 40.5023056, 5, 1.0);
//		globeVC.setZoomLimits(0.0,1.0);
		return true;
	}

	@Override
	public boolean setUpWithMap(MapController mapVC)
	{
		setupImageLoader(ConfigOptions.TestType.MapTest,mapVC);
		mapVC.animatePositionGeo(-3.6704803, 40.5023056, 5, 1.0);
		mapVC.setAllowRotateGesture(true);
//		mapVC.setZoomLimits(0.0,1.0);
		return true;
	}
}
