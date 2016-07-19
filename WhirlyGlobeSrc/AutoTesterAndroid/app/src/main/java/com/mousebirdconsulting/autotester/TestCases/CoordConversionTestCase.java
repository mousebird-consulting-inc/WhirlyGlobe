package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;
import android.util.Log;

import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MapController;
import com.mousebird.maply.Point2d;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;

/**
 * Test coordinate conversion batch functions.
 */

public class CoordConversionTestCase extends MaplyTestCase
{
    public CoordConversionTestCase(Activity activity) {
        super(activity);

        setTestName("Coord Conversion Test");
        setDelay(4);
        this.implementation = TestExecutionImplementation.Both;
    }

    @Override
    public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
        StamenRemoteTestCase baseView = new StamenRemoteTestCase(getActivity());
        baseView.setUpWithGlobe(globeVC);

        globeVC.animatePositionGeo(-3.6704803, 40.5023056, 5, 1.0);
//		globeVC.setZoomLimits(0.0,1.0);
        return true;
    }

    @Override
    public boolean setUpWithMap(MapController mapVC)
    {
        StamenRemoteTestCase baseView = new StamenRemoteTestCase(getActivity());
        baseView.setUpWithMap(mapVC);

        mapVC.animatePositionGeo(-3.6704803, 40.5023056, 5, 1.0);
        mapVC.setAllowRotateGesture(true);
//		mapVC.setZoomLimits(0.0,1.0);
        return true;
    }

    public void userDidTap(GlobeController globeControl, Point2d loc, Point2d screenLoc)
    {
        Log.d("Maply","User tapped at (" + loc.getX()*180/Math.PI + "," + loc.getY()*180/Math.PI + ") on screen (" + screenLoc.getX() + "," + screenLoc.getY() + ")");

        int numPts = 5;
        double[] screenX = new double[5];
        double[] screenY = new double[5];
        double[] geoX = new double[5];
        double[] geoY = new double[5];
        double[] geoZ = new double[5];
        for (int ii=0;ii<numPts;ii++) {
            screenX[ii] = screenLoc.getX();
            screenY[ii] = screenLoc.getY();
        }
        globeControl.geoPointFromScreenBatch(screenX, screenY, geoX, geoY);
        for (int ii=0;ii<numPts;ii++)
        {
            Log.d("Maply","  GeoPt = (" + geoX[ii]*180/Math.PI + "," + geoY[ii]*180/Math.PI + ")");
        }

        for (int ii=0;ii<numPts;ii++)
        {
            geoX[ii] = loc.getX();
            geoY[ii] = loc.getY();
            screenX[ii] = 0.0;  screenY[ii] = 0.0;
        }
        globeControl.screenPointFromGeoBatch(geoX, geoY, geoZ, screenX, screenY);
        for (int ii=0;ii<numPts;ii++)
        {
            Log.d("Maply","  ScreenPt = (" + screenX[ii] + "," + screenY[ii] + ")");
        }
    }

}
