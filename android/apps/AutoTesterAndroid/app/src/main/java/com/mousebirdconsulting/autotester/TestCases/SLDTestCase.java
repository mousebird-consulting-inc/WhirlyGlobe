package com.mousebirdconsulting.autotester.TestCases;

import com.mousebird.maply.LayerThread;
import com.mousebird.maply.Point2d;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;

import android.app.Activity;
import android.content.res.AssetFileDescriptor;
import android.content.res.AssetManager;
import android.graphics.Color;

import com.mousebird.maply.ComponentObject;
import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MapController;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.VectorInfo;
import com.mousebird.maply.VectorObject;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;
import com.mousebird.maply.sld.sldstyleset.SLDStyleSet;
import com.mousebird.maply.GeoJSONSource;

import android.util.Log;

import org.apache.commons.io.IOUtils;

import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.Charset;
import java.util.ArrayList;

import org.xmlpull.v1.XmlPullParserException;

/**
 * Created by rghosh on 2017-03-14.
 */

public class SLDTestCase extends MaplyTestCase {

    public SLDTestCase(Activity activity) {
        super(activity);

        setTestName("SLD Test");
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

            for (int i=0; i<slds.length; i++) {

                SLDStyleSet styleSet = new SLDStyleSet(controller, activity.getAssets(), slds[i], activity.getResources().getDisplayMetrics(), false, i*100000);
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
        CartoDBMapTestCase mapBoxSatelliteTestCase = new CartoDBMapTestCase(getActivity());
        mapBoxSatelliteTestCase.setUpWithMap(mapVC);
        Point2d loc = Point2d.FromDegrees(-5.93, 54.597);
        mapVC.setPositionGeo(loc.getX(), loc.getY(), 0.001);

        testSLD();
        return true;
    }

    @Override
    public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
        CartoDBMapTestCase mapBoxSatelliteTestCase = new CartoDBMapTestCase(getActivity());
        mapBoxSatelliteTestCase.setUpWithGlobe(globeVC);
        Point2d loc = Point2d.FromDegrees(-5.93, 54.597);
        globeVC.animatePositionGeo(loc.getX(), loc.getY(), 0.001, 1.0);

        testSLD();
        return true;
    }


}
