package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;
import android.os.Looper;

import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MapController;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.QuadImageTileLayer;
import com.mousebird.maply.SphericalMercatorCoordSystem;
import com.mousebird.maply.TestImageSource;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;

public class ImageSingleLevelTestCase extends MaplyTestCase {

    public ImageSingleLevelTestCase(Activity activity) {
        super(activity);
        setTestName("Single Level Image Test Case");
        setDelay(100);
        this.implementation = TestExecutionImplementation.Both;
    }

    void addSingleLayer(MaplyBaseController control)
    {
        TestImageSource tileSource = new TestImageSource(Looper.getMainLooper(),0,14);
        tileSource.alpha = 64;

        QuadImageTileLayer baseLayer = new QuadImageTileLayer(control, new SphericalMercatorCoordSystem(), tileSource);
        baseLayer.setCoverPoles(false);
        baseLayer.setHandleEdges(false);
        baseLayer.setDrawPriority(1000);
        baseLayer.setSingleLevelLoading(true);

        control.addLayer(baseLayer);
    }

    @Override
    public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
        CartoDBMapTestCase baseTestCase = new CartoDBMapTestCase(this.getActivity());
        baseTestCase.setUpWithGlobe(globeVC);

        addSingleLayer(globeVC);

        return true;
    }

    @Override
    public boolean setUpWithMap(MapController mapVC) throws Exception {
        CartoDBMapTestCase baseTestCase = new CartoDBMapTestCase(this.getActivity());
        baseTestCase.setUpWithMap(mapVC);

        addSingleLayer(mapVC);

        return true;
    }
}