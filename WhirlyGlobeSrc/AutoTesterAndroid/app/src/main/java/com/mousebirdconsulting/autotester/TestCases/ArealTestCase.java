package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;
import android.graphics.Color;

import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MapController;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.Point2d;
import com.mousebird.maply.VectorInfo;
import com.mousebird.maply.VectorObject;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;

/**
 * Created by sjg on 5/31/16.
 */
public class ArealTestCase extends MaplyTestCase
{
    public ArealTestCase(Activity activity) {
        super(activity);

        setTestName("Areal Vector Test");
        setDelay(1000);
    }

    private void addAreal(MaplyBaseController baseVC) throws Exception {
        VectorInfo vectorInfo = new VectorInfo();
        vectorInfo.setColor(Color.RED);
        vectorInfo.setLineWidth(4.f);
        vectorInfo.setFilled(true);

        VectorObject vecObj = new VectorObject();
        Point2d[] outer = new Point2d[4];
        outer[0] = Point2d.FromDegrees(0,0);
        outer[1] = Point2d.FromDegrees(10,0);
        outer[2] = Point2d.FromDegrees(10,10);
        outer[3] = Point2d.FromDegrees(0,10);
        Point2d[][] innerLoops = new Point2d[1][4];
        innerLoops[0][0] = Point2d.FromDegrees(0.1,0.1);
        innerLoops[0][1] = Point2d.FromDegrees(0.1,0.2);
        innerLoops[0][2] = Point2d.FromDegrees(0.2,0.2);
        innerLoops[0][3] = Point2d.FromDegrees(0.2,0.1);
        vecObj.addAreal(outer,innerLoops);

        baseVC.addVector(vecObj,vectorInfo, MaplyBaseController.ThreadMode.ThreadAny);
    }

    @Override
    public boolean setUpWithMap(MapController mapVC) throws Exception {
        CartoDBDarkTestCase baseTestCase = new CartoDBDarkTestCase(getActivity());
        baseTestCase.setUpWithMap(mapVC);
        addAreal(mapVC);

        mapVC.setPositionGeo(0, 0, 1.0);

        return true;
    }

    @Override
    public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
        CartoDBDarkTestCase baseTestCase = new CartoDBDarkTestCase(getActivity());
        baseTestCase.setUpWithGlobe(globeVC);
        addAreal(globeVC);

        globeVC.setPositionGeo(0, 0, 1.0);

        return true;
    }
}
