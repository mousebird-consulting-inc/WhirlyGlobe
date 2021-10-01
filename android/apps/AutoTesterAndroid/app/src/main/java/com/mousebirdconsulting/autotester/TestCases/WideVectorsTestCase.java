package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.util.Log;

import com.mousebird.maply.ComponentObject;
import com.mousebird.maply.GlobeController;
import com.mousebird.maply.LinearTextureBuilder;
import com.mousebird.maply.MapController;
import com.mousebird.maply.BaseController;
import com.mousebird.maply.MaplyTexture;
import com.mousebird.maply.Point2d;
import com.mousebird.maply.RenderController;
import com.mousebird.maply.VectorInfo;
import com.mousebird.maply.VectorObject;
import com.mousebird.maply.WideVectorInfo;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;

import org.jetbrains.annotations.NotNull;

import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;

import okio.Okio;

public class WideVectorsTestCase extends MaplyTestCase {
    public WideVectorsTestCase(Activity activity) {
        super(activity, "Wide Vectors Test", TestExecutionImplementation.Both);
        setDelay(4);
    }

    @NotNull
    private ArrayList<ComponentObject> addGeoJSON(
            @NotNull BaseController baseController, @NotNull String name,
            float width, int drawPriority, int color,
            boolean usePattern, double edge) {
        MaplyTexture tex = null;
        if (usePattern) {
            // Build a dashed pattern
            LinearTextureBuilder texBuild = new LinearTextureBuilder();
            int[] pattern = new int[2];
            pattern[0] = 4;
            pattern[1] = 4;
            texBuild.setPattern(pattern);
            Bitmap patternImage = texBuild.makeImage();
            RenderController.TextureSettings texSet = new RenderController.TextureSettings();
            texSet.wrapU = true;
            texSet.wrapV = true;
            tex = baseController.addTexture(patternImage, texSet, RenderController.ThreadMode.ThreadCurrent);
        }

        final WideVectorInfo wideVecInfo = new WideVectorInfo();
        wideVecInfo.setColor(color);
        wideVecInfo.setLineWidth(width);
        wideVecInfo.setDrawPriority(drawPriority);
        if (tex != null) {
            wideVecInfo.setTexture(tex);
            wideVecInfo.setTextureRepeatLength(8.0);
        }

        wideVecInfo.setEdgeFalloff(edge);

        final VectorInfo vecInfo = new VectorInfo();
        vecInfo.setLineWidth(4.0f);
        vecInfo.setColor(Color.BLACK);

        ArrayList<ComponentObject> compObjs = new ArrayList<>();
        try {
            InputStream stream = getActivity().getAssets().open("wide_vecs/" + name);
            String json = Okio.buffer(Okio.source(stream)).readUtf8();

            VectorObject vecObj = VectorObject.createFromGeoJSON(json);
            if (vecObj != null) {
                vecObj = vecObj.subdivideToGlobeGreatCircle(0.0001f);
            }
            if (vecObj != null) {
                final ComponentObject compObj = baseController.addVector(
                        vecObj, vecInfo, RenderController.ThreadMode.ThreadCurrent);
                final ComponentObject compObj2 = baseController.addWideVector(
                        vecObj, wideVecInfo, RenderController.ThreadMode.ThreadCurrent);

                if (compObj != null) {
                    compObjs.add(compObj);
                }
                if (compObj2 != null) {
                    compObjs.add(compObj2);
                }
            }
        } catch (Exception e) {
            Log.e(getClass().getSimpleName(), "Failed", e);
        }
        return compObjs;
    }

    private void wideVecTest(@NotNull BaseController baseController) {
        componentObjects.addAll(addGeoJSON(baseController, "mowing-lawn.geojson", 20.0f, VectorInfo.VectorPriorityDefault, Color.BLUE, true, 1));
        componentObjects.addAll(addGeoJSON(baseController, "spiral.geojson", 20.0f, VectorInfo.VectorPriorityDefault, Color.BLUE, true, 1));
        componentObjects.addAll(addGeoJSON(baseController, "square.geojson", 20.0f, VectorInfo.VectorPriorityDefault, Color.BLUE, true, 1));
        componentObjects.addAll(addGeoJSON(baseController, "line.geojson", 100.0f, VectorInfo.VectorPriorityDefault, Color.BLUE, true, 1));
        componentObjects.addAll(addGeoJSON(baseController, "line.geojson", 60.0f, VectorInfo.VectorPriorityDefault+10, Color.RED, true, 1));
        componentObjects.addAll(addGeoJSON(baseController, "track.geojson", 20.0f, VectorInfo.VectorPriorityDefault, Color.BLUE, true, 1));
        componentObjects.addAll(addGeoJSON(baseController, "uturn2.geojson", 20.0f, VectorInfo.VectorPriorityDefault, Color.BLUE, true, 1));
        componentObjects.addAll(addGeoJSON(baseController, "testJson.geojson", 20.0f, VectorInfo.VectorPriorityDefault, Color.BLUE, true, 1));
        componentObjects.addAll(addGeoJSON(baseController, "sawtooth.geojson", 50.0f, VectorInfo.VectorPriorityDefault, Color.RED, false,20));
    }

    @Override
    public boolean setUpWithMap(MapController mapVC) {
        CartoLightTestCase baseTestCase = new CartoLightTestCase(getActivity());
        baseTestCase.setUpWithMap(mapVC);
        wideVecTest(mapVC);
        mapVC.animatePositionGeo(Point2d.FromDegrees(-100,40),0.5,0.0,1.0);
        return true;
    }

    @Override
    public boolean setUpWithGlobe(GlobeController globeVC) {
        CartoLightTestCase baseTestCase = new CartoLightTestCase(getActivity());
        baseTestCase.setUpWithGlobe(globeVC);
        wideVecTest(globeVC);
        globeVC.animatePositionGeo(Point2d.FromDegrees(-100,40),0.5,0.0,1.0);
        return true;
    }

    @Override
    public void shutdown() {
        controller.removeObjects(componentObjects, RenderController.ThreadMode.ThreadCurrent);
        componentObjects.clear();
        baseTestCase.shutdown();
        super.shutdown();
    }

    private final CartoLightTestCase baseTestCase = new CartoLightTestCase(getActivity());
    private final List<ComponentObject> componentObjects = new ArrayList<>();
}