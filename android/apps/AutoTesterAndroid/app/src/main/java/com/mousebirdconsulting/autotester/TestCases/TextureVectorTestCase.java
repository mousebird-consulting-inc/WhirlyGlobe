package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;

import com.mousebird.maply.ComponentObject;
import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MapController;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.MaplyTexture;
import com.mousebird.maply.Point2d;
import com.mousebird.maply.VectorInfo;
import com.mousebird.maply.VectorObject;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;
import com.mousebirdconsulting.autotester.R;

import org.apache.commons.io.IOUtils;

import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.Charset;
import java.util.ArrayList;

public class TextureVectorTestCase extends MaplyTestCase
{
    private ArrayList<VectorObject> vectors = new ArrayList<VectorObject>();

    public TextureVectorTestCase(Activity activity) {
        super(activity);

        setTestName("Texture Vectors");
        setDelay(4);
        this.implementation = TestExecutionImplementation.Both;
    }

    static double ClipGridSize = 2.0/180.0*Math.PI;

    private void overlayCountries(MaplyBaseController baseVC,boolean globeMode) throws Exception {
        AssetManager assetMgr = getActivity().getAssets();
        String[] paths = assetMgr.list("country_json_50m");
        for (String path : paths) {
            InputStream stream = assetMgr.open("country_json_50m/" + path);
            try {
                VectorObject vecObject = new VectorObject();
                String json = IOUtils.toString(stream, Charset.defaultCharset());
                if (vecObject.fromGeoJSON(json)) {
                    vecObject.selectable = true;

                    // Work through each invidiual loop (hopefully)
                    for (VectorObject thisVecObj : vecObject)
                    {
                        VectorObject tessObj = null;
                        Point2d center = thisVecObj.centroid();

                        if (globeMode)
                        {
                            // We adjust the grid clipping size based on the latitude
                            // This helps a lot near the poles.  Otherwise we're way oversampling
                            float thisClipGridLon = (float)ClipGridSize;
                            if (Math.abs(center.getY()) > 60.0/180.0 * Math.PI)
                                thisClipGridLon *= 4.0;
                            else if (Math.abs(center.getY()) > 45.0/180.0 * Math.PI)
                                thisClipGridLon *= 2.0;

                            thisVecObj.getAttributes().setDouble("veccenterx",center.getX());
                            thisVecObj.getAttributes().setDouble("veccentery",center.getX());

                            // We clip the vector to a grid and then tesselate the results
                            // This forms the vector closer to the globe, make it look nicer
                            VectorObject clipped = thisVecObj.clipToGrid(new Point2d(thisClipGridLon, ClipGridSize));
                            if (clipped != null)
                                tessObj = clipped.tesselate();
                        }

                        if (tessObj == null)
                            tessObj = thisVecObj.tesselate();

                        vectors.add(tessObj);
                    }
                }
            } finally {
                try {
                    stream.close();
                } catch (IOException e) {
                }
            }
        }

        // Set up a dots texture
        Bitmap bm = BitmapFactory.decodeResource(activity.getResources(), R.drawable.dots);
        MaplyBaseController.TextureSettings texSettings = new MaplyBaseController.TextureSettings();
        texSettings.wrapU = true;  texSettings.wrapV = true;
        MaplyTexture tex = baseVC.addTexture(bm, texSettings, MaplyBaseController.ThreadMode.ThreadCurrent);

        // Add the vectors with textures
        VectorInfo vectorInfo = new VectorInfo();
        vectorInfo.setFilled(true);
        vectorInfo.setTexture(tex);
        vectorInfo.setTextureProjection(VectorInfo.TextureProjection.TangentPlane);
        vectorInfo.setTexScale(6.0,6.0);
        vectorInfo.setColor(Color.WHITE);
        ComponentObject compObj = baseVC.addVectors(vectors, vectorInfo, MaplyBaseController.ThreadMode.ThreadAny);
    }

    @Override
    public boolean setUpWithMap(MapController mapVC) throws Exception {
        StamenRemoteTestCase baseCase = new StamenRemoteTestCase(getActivity());
        baseCase.setUpWithMap(mapVC);
        overlayCountries(mapVC,false);
        return true;
    }

    @Override
    public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
        StamenRemoteTestCase baseCase = new StamenRemoteTestCase(getActivity());
        baseCase.setUpWithGlobe(globeVC);
        overlayCountries(globeVC,true);
        return true;
    }

    public ArrayList<VectorObject> getVectors() {
        return vectors;
    }}
