package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;
import android.os.Looper;

import com.mousebird.maply.CoordSystem;
import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MapController;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.Point2d;
import com.mousebird.maply.QuadImageTileLayer;
import com.mousebird.maply.TestImageSource;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;

/**
 * Created by sjg on 2/13/16.
 */
public class CustomBNGCoordAdapter extends MaplyTestCase
{
    public CustomBNGCoordAdapter(Activity activity) {
        super(activity);
        this.setTestName("Custom BNG Coord Adapter");
        this.setDelay(2000);
        this.implementation = TestExecutionImplementation.Both;
    }

    public QuadImageTileLayer makeTestLayer(MaplyBaseController viewC)
    {
        CoordSystem bngCoordSystem = CustomBNGTileSource.MakeBNGCoordSystem(getActivity(),false);

        TestImageSource tileSource = new TestImageSource(Looper.getMainLooper(),0,14);
        tileSource.alpha = 64;

        QuadImageTileLayer baseLayer = new QuadImageTileLayer(viewC, bngCoordSystem, tileSource);
        baseLayer.setCoverPoles(false);
        baseLayer.setHandleEdges(false);
        baseLayer.setDrawPriority(1000);

        return baseLayer;
    }

    @Override
    protected MapController makeMapController()
    {
        CoordSystem coordSys = CustomBNGTileSource.MakeBNGCoordSystem(getActivity(),true);

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

        QuadImageTileLayer layer = makeTestLayer(globeVC);
        if (layer != null)
            globeVC.addLayer(layer);

        Point2d pt = Point2d.FromDegrees(-0.1275, 51.507222);
        globeVC.setPositionGeo(pt.getX(), pt.getY(), 0.4);

        return true;
    }

    @Override
    public boolean setUpWithMap(MapController mapVC) throws Exception
    {
        StamenRemoteTestCase baseView = new StamenRemoteTestCase(getActivity());
        baseView.setUpWithMap(mapVC);

        QuadImageTileLayer layer = makeTestLayer(mapVC);
        if (layer != null)
            mapVC.addLayer(layer);

        Point2d pt = Point2d.FromDegrees(-0.1275, 51.507222);
        mapVC.setPositionGeo(pt.getX(), pt.getY(), 0.4);

        return true;
    }

}
