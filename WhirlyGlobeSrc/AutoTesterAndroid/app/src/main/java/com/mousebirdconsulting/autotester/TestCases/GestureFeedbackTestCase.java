package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;
import android.util.Log;

import com.mousebird.maply.ComponentObject;
import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MapController;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.Point2d;
import com.mousebird.maply.Point3d;
import com.mousebird.maply.VectorInfo;
import com.mousebird.maply.VectorObject;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;

/**
 * Created by sjg on 1/27/16.
 */
public class GestureFeedbackTestCase extends MaplyTestCase implements GlobeController.GestureDelegate {
    public GestureFeedbackTestCase(Activity activity) {
        super(activity);
        setTestName("Gesture Feedback Test");
        setDelay(2);
        this.implementation = TestExecutionImplementation.Both;
    }

    @Override
    public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
        CartoDBMapTestCase mapBoxSatelliteTestCase = new CartoDBMapTestCase(this.getActivity());
        mapBoxSatelliteTestCase.setUpWithGlobe(globeVC);
        globeVC.gestureDelegate = this;
        return true;
    }

    @Override
    public boolean setUpWithMap(MapController mapVC) throws Exception {
        CartoDBMapTestCase mapBoxSatelliteTestCase = new CartoDBMapTestCase(this.getActivity());
        mapBoxSatelliteTestCase.setUpWithMap(mapVC);
        return true;
    }

    @Override
    public void userDidSelect(GlobeController globeVC, Object o, Point2d loc, Point2d screenLoc) {
        Log.i("AutoTester","User selected feature at");
    }

    @Override
    public void userDidTap(GlobeController globeVC, Point2d loc, Point2d screenLoc) {
        Log.i("AutoTester","User tapped at " + loc.getX() + " " + loc.getY());
    }

    @Override
    public void userDidLongPress(GlobeController globeController, Object o, Point2d loc, Point2d screenLoc) {
        Log.i("AutoTester","User long pressed at " + loc.getX() + " " + loc.getY());
    }

    @Override
    public void globeDidStartMoving(GlobeController globeVC, boolean b) {
        Log.i("AutoTester","User did start moving");
    }

    @Override
    public void globeDidStopMoving(GlobeController globeVC, Point3d[] corners, boolean userMotion) {
        updateBbox(globeVC,corners);
    }

    // Called for every frame
    @Override
    public void globeDidMove(GlobeController globeVC, Point3d[] corners, boolean userMotion) {
        updateBbox(globeVC,corners);
    }

    VectorInfo vecInfo = null;
    ComponentObject compObj = null;

    // Draw an inset bounding box
    void updateBbox(GlobeController globeVC,Point3d[] corners)
    {
        if (vecInfo == null) {
            vecInfo = new VectorInfo();
            vecInfo.setLineWidth(4.f);
        }

        for (int ii=0;ii<corners.length;ii++)
            if (corners[ii] == null)
                return;

        double fac = 0.01;
        double width = corners[1].getX() - corners[0].getX();
        double height = corners[2].getY() - corners[0].getY();

        Point2d newCorners[] = new Point2d[4];
        newCorners[0] = new Point2d(corners[0].getX()+width*fac,corners[0].getY()+height*fac);
        newCorners[1] = new Point2d(corners[1].getX()-width*fac,corners[1].getY()+height*fac);
        newCorners[2] = new Point2d(corners[2].getX()-width*fac,corners[2].getY()-height*fac);
        newCorners[3] = new Point2d(corners[3].getX()+width*fac,corners[3].getY()-height*fac);

        VectorObject vecObj = new VectorObject();
        Point2d pts[] = new Point2d[5];
        for (int ii=0;ii<corners.length+1;ii++)
            pts[ii] = new Point2d(newCorners[ii%corners.length].getX(),newCorners[ii%corners.length].getY());
        vecObj.addLinear(pts);

        globeVC.removeObject(compObj, MaplyBaseController.ThreadMode.ThreadCurrent);
        compObj = globeVC.addVector(vecObj, vecInfo, MaplyBaseController.ThreadMode.ThreadCurrent);
    }
}
