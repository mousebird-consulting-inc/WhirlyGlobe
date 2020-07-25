/*
 *  SLDTestCase.kt
 *  WhirlyGlobeLib
 *
 *  Created by rghosh on 2017-03-14.
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

import com.mousebird.maply.LayerThread;
import com.mousebird.maply.Point2d;
import com.mousebird.maply.sld.sldstyleset.AssetWrapper;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;

import android.app.Activity;
import android.content.res.AssetFileDescriptor;
import android.content.res.AssetManager;
import android.graphics.Color;

import com.mousebird.maply.ComponentObject;
import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MapController;
import com.mousebird.maply.BaseController;
import com.mousebird.maply.VectorInfo;
import com.mousebird.maply.VectorObject;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;
import com.mousebird.maply.sld.sldstyleset.SLDStyleSet;
import com.mousebird.maply.GeoJSONSource;

import android.util.Log;

import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.Charset;
import java.util.ArrayList;

import org.xmlpull.v1.XmlPullParserException;

/**
 * Styled Layer Descriptor test with GeoJSON.
 */
public class SLDTestCase extends MaplyTestCase {

    public SLDTestCase(Activity activity) {
        super(activity);

        setTestName("GeoJSON SLD Style");
        setDelay(4);
        this.implementation = TestExecutionImplementation.Both;
    }

    private void testSLD() {

        try {
            LayerThread layerThread = controller.getWorkingThread();

            class AddGeoJSONRunnable implements Runnable {
                GeoJSONSource geoJSONSource;
                public void setGeoJSONSource(GeoJSONSource geoJSONSource) {
                    this.geoJSONSource = geoJSONSource;
                }
                @Override
                public void run() {
                    geoJSONSource.startParse();
                }
            }

            String[] slds = new String[]{
                    "osm_landuse.sld",
                    "osm_water.sld",
                    "water_lines.sld",
                    "osm_buildings.sld",
                    "osm_roads.sld",
                    "amenities.sld"};
            String[] geojsons = new String[]{
                    "belfast_ireland_landusages.geojson",
                    "belfast_ireland_waterareas.geojson",
                    "belfast_ireland_waterways.geojson",
                    "belfast_ireland_buildings.geojson",
                    "belfast_ireland_roads.geojson",
                    "belfast_ireland_amenities.geojson"};

            AssetWrapper assetWrap = new AssetWrapper(activity.getAssets());

            for (int i=0; i<slds.length; i++) {

                SLDStyleSet styleSet = new SLDStyleSet(controller, assetWrap, slds[i], activity.getResources().getDisplayMetrics(), false, i*100000);
                styleSet.loadSldInputStream();

                GeoJSONSource gjs = new GeoJSONSource();
                gjs.setBaseController(controller);
                gjs.setJsonStream(getActivity().getAssets().open(geojsons[i]));
                gjs.setStyleSet(styleSet);

                AddGeoJSONRunnable addGeoJSONRunnable = new AddGeoJSONRunnable();
                addGeoJSONRunnable.setGeoJSONSource(gjs);
                layerThread.addTask(addGeoJSONRunnable);
            }

        } catch (XmlPullParserException xppException) {
            Log.e("AutoTesterAndroid", "SLDStyleSet XPP exception", xppException);
        } catch (IOException ioException) {
            Log.e("AutoTesterAndroid", "SLDStyleSet IO exception", ioException);
        } catch (Exception exception) {
            Log.e("AutoTesterAndroid", "SLDStyleSet exception", exception);
        }

    }

    @Override
    public boolean setUpWithMap(MapController mapVC) throws Exception {
        CartoLightTestCase mapBoxSatelliteTestCase = new CartoLightTestCase(getActivity());
        mapBoxSatelliteTestCase.setUpWithMap(mapVC);
        Point2d loc = Point2d.FromDegrees(-5.93, 54.597);
        mapVC.setPositionGeo(loc.getX(), loc.getY(), 0.001);

        testSLD();
        return true;
    }

    @Override
    public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
        CartoLightTestCase mapBoxSatelliteTestCase = new CartoLightTestCase(getActivity());
        mapBoxSatelliteTestCase.setUpWithGlobe(globeVC);
        Point2d loc = Point2d.FromDegrees(-5.93, 54.597);
        globeVC.animatePositionGeo(loc.getX(), loc.getY(), 0.001, 1.0);

        testSLD();
        return true;
    }


}
