package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;

import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MapController;
import com.mousebird.maply.MaplyBaseController;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;

/**
 * Project AutoTesterAndroid
 * Created by jmc on 09/08/2017.
 * (c) Iron Bird. All rights reserved.
 */

public class IBSTestCase extends MaplyTestCase {


    public IBSTestCase(Activity activity) {
        super(activity);

        setTestName("IBS Test Case - Issue #xxxx");
        setDelay(4);
        this.implementation = TestExecutionImplementation.Both;

    }


    @Override
    public boolean setUpWithMap(MapController mapVC) throws Exception {
        CartoDBMapTestCase mapBoxSatelliteTestCase = new CartoDBMapTestCase(getActivity());
        mapBoxSatelliteTestCase.setUpWithMap(mapVC);
        this.setUpReprocase(mapVC);
        return true;
    }

    @Override
    public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
        CartoDBMapTestCase mapBoxSatelliteTestCase = new CartoDBMapTestCase(getActivity());
        mapBoxSatelliteTestCase.setUpWithGlobe(globeVC);
        this.setUpReprocase(globeVC);
        return true;
    }



    private void setUpReprocase(MaplyBaseController controller) {

    }


}
