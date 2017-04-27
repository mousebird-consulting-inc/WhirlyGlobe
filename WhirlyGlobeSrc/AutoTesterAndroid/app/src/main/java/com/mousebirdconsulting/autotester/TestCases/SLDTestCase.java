package com.mousebirdconsulting.autotester.TestCases;

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

            //SLDStyleSet styleSet = new SLDStyleSet(controller, activity.getAssets(), "osm_roads.sld", activity.getResources().getDisplayMetrics(), false, 0);
            //SLDStyleSet styleSet = new SLDStyleSet(controller, activity.getAssets(), "osm_landuse.sld", activity.getResources().getDisplayMetrics(), false, 0);
            SLDStyleSet styleSet = new SLDStyleSet(controller, activity.getAssets(), "amenities.sld", activity.getResources().getDisplayMetrics(), false, 0);

            styleSet.loadSldInputStream();


            GeoJSONSource gjs = new GeoJSONSource();
            gjs.setBaseController(controller);
            //gjs.setJsonStream(getActivity().getAssets().open("belfast_ireland_roads.geojson"));
            //gjs.setJsonStream(getActivity().getAssets().open("belfast_ireland_landusages.geojson"));
            gjs.setJsonStream(getActivity().getAssets().open("belfast_ireland_amenities.geojson"));
            gjs.setStyleSet(styleSet);
            gjs.setRelativeDrawPriority(0);
            gjs.startParse();

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
        testSLD();
        return true;
    }

    @Override
    public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
        CartoDBMapTestCase mapBoxSatelliteTestCase = new CartoDBMapTestCase(getActivity());
        mapBoxSatelliteTestCase.setUpWithGlobe(globeVC);
        testSLD();
        return true;
    }


}
