/*
 *  ParticleSystemTestCase.java
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro on 26/1/16
 *  Copyright 2011-2015 mousebird consulting
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
import android.util.Log;

import com.mousebird.maply.ComponentObject;
import com.mousebird.maply.GlobeController;
import com.mousebird.maply.LayerThread;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.MaplyTileID;
import com.mousebird.maply.ParticleBatch;
import com.mousebird.maply.ParticleSystem;
import com.mousebird.maply.ParticleSystemAttribute;
import com.mousebird.maply.Point2d;
import com.mousebird.maply.QuadPagingLayer;
import com.mousebird.maply.SphericalMercatorCoordSystem;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;
import com.squareup.okhttp.OkHttpClient;
import com.squareup.okhttp.Request;
import com.squareup.okhttp.Response;

import java.io.IOException;
import java.net.URL;
import java.util.Date;
import java.util.HashMap;

public class ParticleSystemTestCase extends MaplyTestCase {

    public ParticleSystemTestCase(Activity activity) {
        super(activity);

        this.setTestName("Particle System Test");
        this.setDelay(2000);
        this.implementation = TestExecutionImplementation.Globe;
    }

    LayerThread particleThread = null;
    ParticleSystemAdapter particleAdapter;


    @Override
    public boolean setUpWithGlobe(final GlobeController globeVC) throws Exception {

        StamenRemoteTestCase baseView = new StamenRemoteTestCase(getActivity());
        baseView.setUpWithGlobe(globeVC);

        globeVC.addPostSurfaceRunnable(new Runnable() {
            @Override
            public void run() {
                particleThread = globeVC.makeLayerThread(false);
                particleAdapter = new ParticleSystemAdapter(globeVC, particleThread,
                        "http://tilesets.s3-website-us-east-1.amazonaws.com/wind_test/{dir}_tiles/{z}/{x}/{y}.png", 2, 5);

            }
        });




        return true;
    }

}