/*
 *  TestImagePagerSource.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2014 mousebird consulting
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

package com.mousebird.maplytester;

import android.app.Activity;

import com.mousebird.maply.MaplyController;
import com.mousebird.maply.QuadImageTileLayer;
import com.mousebird.maply.SphericalMercatorCoordSystem;
import com.mousebird.maply.TestImageSource;

/**
 * This test class invokes the TestImageSource, which creates color tiles
 * with tile IDs in the middle for debugging.
 */
public class TestImagePagerSource 
{
	Activity activity = null;
	MaplyController mapControl = null;
	
	TestImagePagerSource(Activity inActivity,MaplyController inMapControl)
	{
		activity = inActivity;
		mapControl = inMapControl;
	}
	
	public void start()
	{
		SphericalMercatorCoordSystem coordSys = new SphericalMercatorCoordSystem();
		TestImageSource tileSource = new TestImageSource(activity.getMainLooper(),0,22);
		QuadImageTileLayer layer = new QuadImageTileLayer(mapControl,coordSys,tileSource);
		layer.setSimultaneousFetches(4);
		mapControl.getLayerThread().addLayer(layer);
	}
}
