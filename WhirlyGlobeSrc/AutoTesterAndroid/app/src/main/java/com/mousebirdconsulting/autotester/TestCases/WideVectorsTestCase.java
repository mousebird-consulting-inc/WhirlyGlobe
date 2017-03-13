package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.Color;

import com.mousebird.maply.ComponentObject;
import com.mousebird.maply.GlobeController;
import com.mousebird.maply.LinearTextureBuilder;
import com.mousebird.maply.MapController;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.MaplyTexture;
import com.mousebird.maply.VectorInfo;
import com.mousebird.maply.VectorObject;
import com.mousebird.maply.WideVectorInfo;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;

import org.apache.commons.io.IOUtils;

import java.io.InputStream;
import java.nio.charset.Charset;
import java.util.ArrayList;

public class WideVectorsTestCase extends MaplyTestCase {
    public WideVectorsTestCase(Activity activity) {
        super(activity);

        setTestName("Wide Vectors Test");
        setDelay(4);
        this.implementation = TestExecutionImplementation.Both;
    }

    void addGeoJSON(MaplyBaseController baseController, String name) {
        // Build a dashed pattern
        LinearTextureBuilder texBuild = new LinearTextureBuilder();
        int[] pattern = new int[2];
        pattern[0] = 4;
        pattern[1] = 4;
        texBuild.setPattern(pattern);
        Bitmap patternImage = texBuild.makeImage();
        MaplyBaseController.TextureSettings texSet = new MaplyBaseController.TextureSettings();
        texSet.wrapU = true;  texSet.wrapV = true;
        MaplyTexture tex = baseController.addTexture(patternImage,new MaplyBaseController.TextureSettings(), MaplyBaseController.ThreadMode.ThreadCurrent);

        WideVectorInfo wideVecInfo = new WideVectorInfo();
        wideVecInfo.setColor(Color.RED);
        wideVecInfo.setLineWidth(20.0f);
        wideVecInfo.setTexture(tex);
        wideVecInfo.setTextureRepeatLength(8.0);

        VectorInfo vecInfo = new VectorInfo();
        vecInfo.setLineWidth(4.0f);
        vecInfo.setColor(Color.BLACK);

        ArrayList<ComponentObject> compObjs = new ArrayList<ComponentObject>();

        try {
            InputStream stream = getActivity().getAssets().open("wide_vecs/" + name);
            String json = IOUtils.toString(stream, Charset.defaultCharset());

            VectorObject vecObj = new VectorObject();
            vecObj.fromGeoJSON(json);

            ComponentObject compObj = baseController.addVector(vecObj, vecInfo, MaplyBaseController.ThreadMode.ThreadAny);
            compObjs.add(compObj);
            compObj = baseController.addWideVector(vecObj, wideVecInfo, MaplyBaseController.ThreadMode.ThreadAny);
            compObjs.add(compObj);
        } catch (Exception e) {
        }
    }

    void wideVecTest(MaplyBaseController baseController) {
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
        CartoDBMapTestCase baseTestCase = new CartoDBMapTestCase(getActivity());
        baseTestCase.setUpWithMap(mapVC);

        wideVecTest(mapVC);

        return true;
    }

    @Override
    public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
        CartoDBMapTestCase baseTestCase = new CartoDBMapTestCase(getActivity());
        baseTestCase.setUpWithGlobe(globeVC);

        wideVecTest(globeVC);

        return true;
    }
}