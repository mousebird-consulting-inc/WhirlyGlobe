package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;

import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MapController;
import com.mousebird.maply.Point2d;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;

/**
 * Created by sjg on 8/23/16.
 */
public class BoundsTestCase extends MaplyTestCase
{
    public BoundsTestCase(Activity activity) {
        super(activity);

        setTestName("Bounds Test");
        setDelay(4);
        this.implementation = TestExecutionImplementation.Both;
    }

    @Override
    public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
        StamenRemoteTestCase baseView = new StamenRemoteTestCase(getActivity());
        baseView.setUpWithGlobe(globeVC);

        return true;
    }

    @Override
    public boolean setUpWithMap(MapController mapVC) throws Exception {
        StamenRemoteTestCase baseView = new StamenRemoteTestCase(getActivity());
        baseView.setUpWithMap(mapVC);

        mapVC.setPositionGeo(0,0,0.1);
        mapVC.setViewExtents(Point2d.FromDegrees(-60,-40), Point2d.FromDegrees(60,70));

        return true;
    }

}
