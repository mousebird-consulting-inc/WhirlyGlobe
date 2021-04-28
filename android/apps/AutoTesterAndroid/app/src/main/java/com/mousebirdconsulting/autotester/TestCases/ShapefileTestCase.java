package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;
import android.util.Log;

import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MapController;
import com.mousebird.maply.BaseController;
import com.mousebird.maply.Mbr;
import com.mousebird.maply.Point2d;
import com.mousebird.maply.RenderController;
import com.mousebird.maply.VectorInfo;
import com.mousebird.maply.VectorObject;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;

import java.io.File;

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
    private File copyFile(String srcName, String destName) {
        return copyAssetFile(srcName, "sf_shapefile", destName);
    }

    public Mbr addShapeFile(BaseController baseVC) {
        try {
            File dbfFile = copyFile("sf_roads/tl_2013_06075_roads.dbf", "sf_roads.dbf");
            File shpFile = copyFile("sf_roads/tl_2013_06075_roads.shp", "sf_roads.shp");
            File shxFile = copyFile("sf_roads/tl_2013_06075_roads.shx", "sf_roads.shx");
            if (dbfFile != null && shpFile != null && shxFile != null) {
                VectorObject vecObj = new VectorObject();
                vecObj.fromShapeFile(shpFile.getAbsolutePath());
                //int numPoints = vecObj.countPoints();
                VectorInfo vecInfo = new VectorInfo();
                //vecInfo.setColor(Color.RED);
                baseVC.addVector(vecObj,vecInfo, RenderController.ThreadMode.ThreadAny);

                Point2d ll = new Point2d();
                Point2d ur = new Point2d();
                vecObj.boundingBox(ll, ur);
                return new Mbr(ll, ur);
            }
        }
        catch (Exception e) {
            Log.d("Maply", e.toString());
        }
        return null;
    }

    @Override
    public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
        StamenRemoteTestCase baseView = new StamenRemoteTestCase(getActivity());
        baseView.setUpWithGlobe(globeVC);

        Mbr bbox = addShapeFile(globeVC);
        if (bbox != null) {
            final Point2d center = bbox.middle();
            // note: this doesn't work because something about the controller isn't sufficiently initialized
            //double height = globeVC.findHeightToViewBounds(bbox, center);
            globeVC.addPostSurfaceRunnable(() -> {
                double height = globeVC.findHeightToViewBounds(bbox, center);
                globeVC.animatePositionGeo(center.getX(),center.getY(),height,2.0);
            });
        }

        return true;
    }

    @Override
    public boolean setUpWithMap(MapController mapVC) {
        StamenRemoteTestCase baseView = new StamenRemoteTestCase(getActivity());
        baseView.setUpWithMap(mapVC);

        Mbr bbox = addShapeFile(mapVC);
        if (bbox != null) {
            Point2d center = bbox.middle();
            double height = mapVC.findHeightToViewBounds(bbox, center);
            mapVC.animatePositionGeo(center.getX(),center.getY(),height,2.0);
        }

        return true;
    }

}
