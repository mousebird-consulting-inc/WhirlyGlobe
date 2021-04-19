package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;

import com.mousebird.maply.CoordSystem;
import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MapController;
import com.mousebird.maply.Point2d;
import com.mousebird.maply.QuadImageLoader;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;

/**
 * Created by sjg on 2/13/16.
 */
public class CustomBNGCoordAdapter extends MaplyTestCase
{
    public CustomBNGCoordAdapter(Activity activity) {
        super(activity);
        this.setTestName("British National Grid (custom map)");
        this.implementation = TestExecutionImplementation.Both;
    }

    @Override
    protected MapController makeMapController()
    {
        CoordSystem coordSys = CustomBNGTileSource.MakeBNGCoordSystem(getActivity(),true);

        MapController.Settings settings = new MapController.Settings();
        settings.coordSys = coordSys;
        MapController mapControl = new MapController(getActivity(),settings);
        mapControl.gestureDelegate = this;

        return mapControl;
    }

    @Override
    public boolean setUpWithMap(MapController mapVC) throws Exception {
        baseCase = new StamenRemoteTestCase(getActivity());
        baseCase.setUpWithMap(mapVC);

        loader = CustomBNGTileSource.makeTestLoader(getActivity(), mapVC, true);

        Point2d pt = Point2d.FromDegrees(-0.1275, 51.507222);
        mapVC.setPositionGeo(pt.getX(), pt.getY(), 0.4);

        return true;
    }

    @Override
    public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
        baseCase = new StamenRemoteTestCase(getActivity());
        baseCase.setUpWithGlobe(globeVC);

        loader = CustomBNGTileSource.makeTestLoader(getActivity(), globeVC, true);

        Point2d pt = Point2d.FromDegrees(-0.1275, 51.507222);
        globeVC.setPositionGeo(pt.getX(), pt.getY(), 0.4);

        return true;
    }

    @Override
    public void shutdown() {
        if (loader != null) {
            loader.shutdown();
            loader = null;
        }
        if (baseCase != null) {
            baseCase.shutdown();
        }
        super.shutdown();
    }

    private MaplyTestCase baseCase = null;
    private QuadImageLoader loader = null;
}
