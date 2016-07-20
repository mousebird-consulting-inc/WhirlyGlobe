package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;
import android.graphics.Color;
import android.util.Log;

import com.mousebird.maply.ComponentObject;
import com.mousebird.maply.CoordSystem;
import com.mousebird.maply.GlobeController;
import com.mousebird.maply.LabelInfo;
import com.mousebird.maply.LayerThread;
import com.mousebird.maply.MapController;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.MaplyTileID;
import com.mousebird.maply.Mbr;
import com.mousebird.maply.Point2d;
import com.mousebird.maply.QuadPagingLayer;
import com.mousebird.maply.ScreenLabel;
import com.mousebird.maply.SphericalMercatorCoordSystem;
import com.mousebird.maply.VectorInfo;
import com.mousebird.maply.VectorObject;
import com.mousebirdconsulting.autotester.ConfigOptions;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;

/**
 * Created by sjg on 6/8/16.
 */
public class PagingLayerTestCase extends MaplyTestCase implements QuadPagingLayer.PagingInterface {
    private static String TAG = "AutoTester";
    private Activity activity;

    CoordSystem coordSys = new SphericalMercatorCoordSystem();

    public PagingLayerTestCase(Activity activity) {
        super(activity);
        setTestName("Paging Layer Test");
        setDelay(1000);

        this.activity = activity;
    }

    /**
     * QuadPagingLayer PagingInterface
     */

    public int minZoom()
    {
        return 4;
    }

    public int maxZoom()
    {
        return 18;
    }

    // Colors to use for various layers
    int colors[] = new int[]{
        Color.RED,Color.BLUE,Color.CYAN,Color.GREEN,Color.MAGENTA,Color.YELLOW,Color.GRAY};

    public void startFetchForTile(final QuadPagingLayer layer,final MaplyTileID tileID)
    {
        Log.d("Maply","Paging layer: Loading tile " + tileID);

        // Ask for a thread that already has the setup we need to push stuff through
        LayerThread thread = layer.maplyControl.getWorkingThread();
        thread.addTask(new Runnable() {
                           @Override
                           public void run() {
                               Mbr mbr = layer.geoBoundsForTile(tileID);

                               // Create an areal feature
                               VectorObject vecObj = new VectorObject();
                               double spanX = mbr.ur.getX() - mbr.ll.getX();
                               double spanY = mbr.ur.getY() - mbr.ll.getY();
                               Point2d[] pts = new Point2d[]{
                                       new Point2d(mbr.ll.getX() + spanX * 0.1, mbr.ll.getY() + spanY * 0.1),
                                       new Point2d(mbr.ur.getX() - spanX * 0.1, mbr.ll.getY() + spanY * 0.1),
                                       new Point2d(mbr.ur.getX() - spanX * 0.1, mbr.ur.getY() - spanY * 0.1),
                                       new Point2d(mbr.ll.getX() + spanX * 0.1, mbr.ur.getY() - spanY * 0.1)};
                               vecObj.addAreal(pts);
                               vecObj.getAttributes().setString("tile", tileID.toString());

                               VectorInfo vecInfo = new VectorInfo();
                               int color = colors[tileID.level % colors.length];
                               float alpha = 0.5f;
                               vecInfo.setColor(Color.argb((int)(128*alpha), (int)(Color.red(color)*alpha), (int)(Color.green(color)*alpha), (int)(Color.blue(color)*alpha)));
                               vecInfo.setFilled(true);
                               vecInfo.setEnable(false);
                               vecInfo.setDrawPriority(VectorInfo.VectorPriorityDefault);
                               ComponentObject compObj = layer.maplyControl.addVector(vecObj, vecInfo, MaplyBaseController.ThreadMode.ThreadCurrent);
                               layer.addData(compObj, tileID);

//                               VectorInfo vecInfoOutline = new VectorInfo();
//                               vecInfoOutline.setColor(Color.argb(255, Color.red(color), Color.green(color), Color.blue(color)));
//                               vecInfoOutline.setFilled(false);
//                               vecInfoOutline.setEnable(false);
//                               vecInfoOutline.setLineWidth(10.f);
//                               vecInfo.setDrawPriority(VectorInfo.VectorPriorityDefault + 1);
//                               ComponentObject compObjOutline = layer.maplyControl.addVector(vecObj, vecInfoOutline, MaplyBaseController.ThreadMode.ThreadCurrent);
//                               layer.addData(compObjOutline, tileID);

                               // And a label right in the middle
                               ScreenLabel label = new ScreenLabel();
                               label.loc = new Point2d(mbr.middle());
                               label.text = tileID.toString();
                               LabelInfo labelInfo = new LabelInfo();
                               labelInfo.setLayoutPlacement(LabelInfo.LayoutCenter);
                               labelInfo.setTextColor(Color.BLACK);
                               labelInfo.setEnable(false);
                               labelInfo.setLayoutImportance(1.f);
                               ComponentObject compObj2 = layer.maplyControl.addScreenLabel(label, labelInfo, MaplyBaseController.ThreadMode.ThreadCurrent);
                               layer.addData(compObj2, tileID);

                               layer.tileDidLoad(tileID);
                           }
                       });
    }

    public void tileDidUnload(MaplyTileID tileID)
    {
    }

    private QuadPagingLayer setupPagingLayer(MaplyBaseController baseController, ConfigOptions.TestType testType) throws Exception
    {
        QuadPagingLayer layer = new QuadPagingLayer(baseController,coordSys,this);
        layer.setImportance(256*256);
        layer.setSimultaneousFetches(8);
        layer.setSingleLevelLoading(true);
        layer.setUseTargetZoomLevel(true);

        return layer;
    }

    @Override
    public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
        CartoDBMapTestCase baseCase = new CartoDBMapTestCase(getActivity());
        baseCase.setUpWithGlobe(globeVC);
        globeVC.addLayer(setupPagingLayer(globeVC, ConfigOptions.TestType.GlobeTest));

        Point2d loc = Point2d.FromDegrees(2.3508, 48.8567);
        globeVC.setPositionGeo(loc.getX(),loc.getY(),1.0);

        return true;
    }

    @Override
    public boolean setUpWithMap(MapController mapVC) throws Exception {
        CartoDBMapTestCase baseCase = new CartoDBMapTestCase(getActivity());
        baseCase.setUpWithMap(mapVC);
        mapVC.addLayer(setupPagingLayer(mapVC, ConfigOptions.TestType.MapTest));

        Point2d loc = Point2d.FromDegrees(2.3508, 48.8567);
        mapVC.setPositionGeo(loc.getX(),loc.getY(),1.0);

        return true;
    }
}
