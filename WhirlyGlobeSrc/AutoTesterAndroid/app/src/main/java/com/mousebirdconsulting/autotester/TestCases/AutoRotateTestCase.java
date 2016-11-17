package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;

import com.mousebird.maply.GlobeController;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;

public class AutoRotateTestCase extends MaplyTestCase {
    public AutoRotateTestCase(Activity activity) {
        super(activity);

        setTestName("Auto Rotate Test Case");
        setDelay(4);
        this.implementation = TestExecutionImplementation.Globe;

        // Just trying...
        //this.remoteResources.add("https://manuals.info.apple.com/en_US/macbook_retina_12_inch_early2016_essentials.pdf");
    }

    @Override
    public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
        StamenRemoteTestCase baseView = new StamenRemoteTestCase(getActivity());
        baseView.setUpWithGlobe(globeVC);

        globeVC.setAutoRotate(5.f,45);

        return true;
    }

}
