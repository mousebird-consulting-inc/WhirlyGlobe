package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;
import android.content.Context;
import android.content.ContextWrapper;
import android.graphics.Color;
import android.util.Log;

import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MapController;
import com.mousebird.maply.BaseController;
import com.mousebird.maply.RenderController;
import com.mousebird.maply.VectorInfo;
import com.mousebird.maply.VectorObject;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * Created by sjg on 12/21/17.
 */
public class ShapefileTestCase extends MaplyTestCase
{
    public ShapefileTestCase(Activity activity) {
        super(activity);

        setTestName("Shapefile Test");
        setDelay(4);
        this.implementation = TestExecutionImplementation.Both;
    }

    // Copy a file out of the bundle
    private File copyFile(String srcName, String destName) throws IOException {

        ContextWrapper wrapper = new ContextWrapper(getActivity());
        File destDir =  wrapper.getDir("sf_shapefile", Context.MODE_PRIVATE);

        InputStream is = getActivity().getAssets().open(srcName);
        File of = new File(destDir, destName);

        if (of.exists()) {
            return of;
        }

        OutputStream os = new FileOutputStream(of);
        byte[] mBuffer = new byte[1024];
        int length;
        while ((length = is.read(mBuffer))>0) {
            os.write(mBuffer, 0, length);
        }
        os.flush();
        os.close();
        is.close();

        return of;
    }

    public void addShapeFile(BaseController baseVC) {
        try {
            File dbfFile = copyFile("sf_roads/tl_2013_06075_roads.dbf", "sf_roads.dbf");
            File shpFile = copyFile("sf_roads/tl_2013_06075_roads.shp", "sf_roads.shp");
            File shxFile = copyFile("sf_roads/tl_2013_06075_roads.shx", "sf_roads.shx");
            if (dbfFile != null && shpFile != null && shxFile != null) {
                VectorObject vecObj = new VectorObject();
                vecObj.fromShapeFile(shpFile.getAbsolutePath());
                int numPoints = vecObj.countPoints();
                VectorInfo vecInfo = new VectorInfo();
                vecInfo.setColor(Color.RED);
                baseVC.addVector(vecObj,vecInfo, RenderController.ThreadMode.ThreadAny);
            }
        }
        catch (Exception e) {
            Log.d("Maply", e.toString());
        }
    }

    @Override
    public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
        StamenRemoteTestCase baseView = new StamenRemoteTestCase(getActivity());
        baseView.setUpWithGlobe(globeVC);

        addShapeFile(globeVC);

        return true;
    }

    @Override
    public boolean setUpWithMap(MapController mapVC) throws Exception {
        StamenRemoteTestCase baseView = new StamenRemoteTestCase(getActivity());
        baseView.setUpWithMap(mapVC);

        addShapeFile(mapVC);

        return true;
    }

}
