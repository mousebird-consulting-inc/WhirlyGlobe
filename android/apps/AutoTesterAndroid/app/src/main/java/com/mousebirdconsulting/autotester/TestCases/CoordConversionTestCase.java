package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;
import android.util.Log;
import android.widget.Toast;

import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MapController;
import com.mousebird.maply.BaseController;
import com.mousebird.maply.Point2d;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;

import java.util.Locale;

/**
 * Test coordinate conversion batch functions.
 */

public class CoordConversionTestCase extends MaplyTestCase
{
    public CoordConversionTestCase(Activity activity) {
        super(activity, "Coord Conversion Test", TestExecutionImplementation.Both);
    }

    @Override
    public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
        baseView.setDoColorChange(false);
        baseView.setUpWithGlobe(globeVC);

        globeVC.animatePositionGeo(-3.6704803, 40.5023056, 5, 1.0);
//		globeVC.setZoomLimits(0.0,1.0);
        return true;
    }

    @Override
    public boolean setUpWithMap(MapController mapVC) throws Exception {
        baseView.setDoColorChange(false);
        baseView.setUpWithMap(mapVC);

        mapVC.animatePositionGeo(-3.6704803, 40.5023056, 5, 1.0);
        mapVC.setAllowRotateGesture(true);
//		mapVC.setZoomLimits(0.0,1.0);
        return true;
    }

    @Override
    public void shutdown() {
        baseView.shutdown();
        super.shutdown();
    }

    protected void processTap(BaseController control, Point2d loc, Point2d screenLoc) {
        Log.d("Maply","User tapped at (" + loc.getX()*180/Math.PI + "," + loc.getY()*180/Math.PI + ") on screen (" + screenLoc.getX() + "," + screenLoc.getY() + ")");

        final int numPts = 5;
        final double[] screenX = new double[5];
        final double[] screenY = new double[5];
        final double[] geoX = new double[5];
        final double[] geoY = new double[5];
        final double[] geoZ = new double[5];
        for (int ii=0;ii<numPts;ii++) {
            screenX[ii] = screenLoc.getX() + ii;
            screenY[ii] = screenLoc.getY() + ii;
        }
        control.geoPointFromScreenBatch(screenX, screenY, geoX, geoY);
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
        control.screenPointFromGeoBatch(geoX, geoY, geoZ, screenX, screenY);
        for (int ii=0;ii<numPts;ii++)
        {
            Log.d("Maply","  ScreenPt = (" + screenX[ii] + "," + screenY[ii] + ")");
        }

        Toast.makeText(getActivity().getApplicationContext(),
                String.format(Locale.getDefault(),
                        "(%.1f,%.1f) = (%.6f,%.6f) = (%.1f,%.1f)",
                        screenLoc.getX(), screenLoc.getY(),
                        geoX[0]*180/Math.PI, geoY[0]*180/Math.PI,
                        screenX[0], screenY[0]),
                Toast.LENGTH_SHORT).show();
    }

    public void userDidTap(GlobeController globeControl, Point2d loc, Point2d screenLoc) {
        processTap(globeControl,loc,screenLoc);
    }

    public void userDidTap(MapController mapControl,Point2d loc,Point2d screenLoc) {
        processTap(mapControl,loc,screenLoc);
    }

    private final StamenRemoteTestCase baseView = new StamenRemoteTestCase(getActivity());
}
