package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.Color;

import com.mousebird.maply.ComponentObject;
import com.mousebird.maply.GlobeController;
import com.mousebird.maply.LinearTextureBuilder;
import com.mousebird.maply.MapController;
import com.mousebird.maply.BaseController;
import com.mousebird.maply.MaplyTexture;
import com.mousebird.maply.RenderController;
import com.mousebird.maply.VectorInfo;
import com.mousebird.maply.VectorObject;
import com.mousebird.maply.WideVectorInfo;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;

import java.io.InputStream;
import java.nio.charset.Charset;
import java.util.ArrayList;

import okio.Okio;

public class WideVectorsTestCase extends MaplyTestCase {
    public WideVectorsTestCase(Activity activity) {
        super(activity);

        setTestName("Wide Vectors Test");
        setDelay(4);
        this.implementation = TestExecutionImplementation.Both;
    }

    void addGeoJSON(BaseController baseController, String name) {
        // Build a dashed pattern
        LinearTextureBuilder texBuild = new LinearTextureBuilder();
        int[] pattern = new int[2];
        pattern[0] = 8;
        pattern[1] = 8;
        texBuild.setPattern(pattern);
        Bitmap patternImage = texBuild.makeImage();
        RenderController.TextureSettings texSet = new RenderController.TextureSettings();
        texSet.wrapU = true;  texSet.wrapV = true;
        MaplyTexture tex = baseController.addTexture(patternImage, texSet, RenderController.ThreadMode.ThreadCurrent);

        WideVectorInfo wideVecInfo = new WideVectorInfo();
        wideVecInfo.setColor(Color.BLUE);
        wideVecInfo.setLineWidth(20.0f);
        wideVecInfo.setTexture(tex);
        wideVecInfo.setTextureRepeatLength(16.0);

        VectorInfo vecInfo = new VectorInfo();
        vecInfo.setLineWidth(4.0f);
        vecInfo.setColor(Color.BLACK);

        ArrayList<ComponentObject> compObjs = new ArrayList<ComponentObject>();

        try {
            InputStream stream = getActivity().getAssets().open("wide_vecs/" + name);
            String json = Okio.buffer(Okio.source(stream)).readUtf8();

            VectorObject vecObj = new VectorObject();
            vecObj.fromGeoJSON(json);

//            ComponentObject compObj = baseController.addVector(vecObj, vecInfo, MaplyBaseController.ThreadMode.ThreadAny);
//            compObjs.add(compObj);
            ComponentObject compObj = baseController.addWideVector(vecObj, wideVecInfo, RenderController.ThreadMode.ThreadAny);
            compObjs.add(compObj);
        } catch (Exception e) {
        }
    }

    void wideVecTest(BaseController baseController) {
//        addGeoJSON(baseController, "sawtooth.geojson");
        addGeoJSON(baseController, "mowing-lawn.geojson");
        addGeoJSON(baseController, "spiral.geojson");
        addGeoJSON(baseController, "square.geojson");
        addGeoJSON(baseController, "track.geojson");
        addGeoJSON(baseController, "uturn2.geojson");
        addGeoJSON(baseController, "testJson.geojson");
    }

    @Override
    public boolean setUpWithMap(MapController mapVC) throws Exception {
        CartoLightTestCase baseTestCase = new CartoLightTestCase(getActivity());
        baseTestCase.setUpWithMap(mapVC);

        wideVecTest(mapVC);

        return true;
    }

    @Override
    public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
        CartoLightTestCase baseTestCase = new CartoLightTestCase(getActivity());
        baseTestCase.setUpWithGlobe(globeVC);

        wideVecTest(globeVC);

        return true;
    }
}