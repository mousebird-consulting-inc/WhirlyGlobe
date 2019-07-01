package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;
import android.os.Looper;

import com.mousebird.maply.CoordSystem;
import com.mousebird.maply.GlobeController;
import com.mousebird.maply.ImageLoaderInterpreter;
import com.mousebird.maply.MapController;
import com.mousebird.maply.BaseController;
import com.mousebird.maply.OvlDebugImageLoaderInterpreter;
import com.mousebird.maply.Point2d;
import com.mousebird.maply.QuadImageLoader;
import com.mousebird.maply.SamplingParams;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;

/**
 * Created by sjg on 2/13/16.
 */
public class CustomBNGCoordAdapter extends MaplyTestCase
{
    public CustomBNGCoordAdapter(Activity activity) {
        super(activity);
        this.setTestName("British National Grid (custom map)");
        this.implementation = TestExecutionImplementation.Map;
    }

    ImageLoaderInterpreter testInterp = null;
    CoordSystem coordSys = null;

    public void makeTestLoader(BaseController viewC)
    {
        SamplingParams params = new SamplingParams();
        params.setCoordSystem(coordSys);
        params.setCoverPoles(false);
        params.setEdgeMatching(false);
        params.setMinZoom(0);
        params.setMaxZoom(10);

        testInterp = new ImageLoaderInterpreter();

        QuadImageLoader loader = new QuadImageLoader(params,null,viewC);
        loader.setLoaderInterpreter(testInterp);
    }

    @Override
    protected MapController makeMapController()
    {
        coordSys = CustomBNGTileSource.MakeBNGCoordSystem(getActivity(),true);

        MapController.Settings settings = new MapController.Settings();
        settings.coordSys = coordSys;
        MapController mapControl = new MapController(activity,settings);
        mapControl.gestureDelegate = this;

        return mapControl;
    }

    @Override
    public boolean setUpWithGlobe(GlobeController globeVC) throws Exception
    {
        StamenRemoteTestCase baseView = new StamenRemoteTestCase(getActivity());
        baseView.setUpWithGlobe(globeVC);

        makeTestLoader(globeVC);

        Point2d pt = Point2d.FromDegrees(-0.1275, 51.507222);
        globeVC.setPositionGeo(pt.getX(), pt.getY(), 0.4);

        return true;
    }
}
