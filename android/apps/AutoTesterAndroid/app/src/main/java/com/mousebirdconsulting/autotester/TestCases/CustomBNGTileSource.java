package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;

import com.mousebird.maply.BaseController;
import com.mousebird.maply.CoordSystem;
import com.mousebird.maply.GlobeController;
import com.mousebird.maply.ImageLoaderInterpreter;
import com.mousebird.maply.MapController;
import com.mousebird.maply.Mbr;
import com.mousebird.maply.OvlDebugImageLoaderInterpreter;
import com.mousebird.maply.Point2d;
import com.mousebird.maply.Proj4CoordSystem;
import com.mousebird.maply.QuadImageLoader;
import com.mousebird.maply.SamplingParams;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;
import java.io.File;

/**
 * Created by sjg on 2/13/16.
 */
public class CustomBNGTileSource extends MaplyTestCase
{
    public CustomBNGTileSource(Activity activity) {
        super(activity, "British National Grid");
        this.setDelay(2);
    }

    // Put together a British National Grid system
    static public CoordSystem MakeBNGCoordSystem(Activity activity, boolean displayVersion)
    {
        File bngFile = copyAssetFile(activity, "bng/OSTN02_NTv2.gsb", "bng", "OSTN02_NTv2.gsb");
        String gridRef = " +nadgrids=" + bngFile;

        String projStr = "+proj=tmerc +lat_0=49 +lon_0=-2 +k=0.9996012717 +x_0=400000 +y_0=-100000 +ellps=airy +units=m +no_defs" + gridRef;
        Proj4CoordSystem coordSys = new Proj4CoordSystem(projStr);

        // Set the bounding box for validity.  It assumes it can go everywhere by default
        Mbr bbox = new Mbr();
        bbox.addPoint(new Point2d(1393.0196,13494.9764));
        bbox.addPoint(new Point2d(671196.3657,1230275.0454));

        // Now expand it out so we can see the whole of the UK
        if (displayVersion)
        {
            // Note: There may be a center/offset problem with the bounds.  Made them bigger to compensate
            double spanX = bbox.ur.getX() - bbox.ll.getX();
            double spanY = bbox.ur.getY() - bbox.ll.getY();
            double extra = 1.5;
            bbox.addPoint(new Point2d(-extra*spanX,-extra*spanY));
            bbox.addPoint(new Point2d(extra*spanX,extra*spanY));
        }

        coordSys.setBounds(bbox);

        return coordSys;
    }

    public static QuadImageLoader makeTestLoader(Activity activity, BaseController viewC, boolean displayVersion)
    {
        CoordSystem bngCoordSystem = MakeBNGCoordSystem(activity, displayVersion);

        SamplingParams params = new SamplingParams();
        params.setCoordSystem(bngCoordSystem);
        params.setCoverPoles(false);
        params.setEdgeMatching(false);
        params.setMinZoom(0);
        params.setMaxZoom(10);
        //params.setMinImportance(256 * 256);

        QuadImageLoader loader = new QuadImageLoader(params,null,viewC);
        loader.setBaseDrawPriority(1000);
        //loader.setLoaderInterpreter(new ImageLoaderInterpreter());

        OvlDebugImageLoaderInterpreter interp = new OvlDebugImageLoaderInterpreter();
        interp.setParentInterpreter(new ImageLoaderInterpreter());
        loader.setLoaderInterpreter(interp);

        //baseLayer.setImportanceScale(4.f);
        //Mbr bounds = bngCoordSystem.getBounds();
        //viewC.setViewExtents(bounds.ll, bounds.ur);

        return loader;
    }

    @Override
    public boolean setUpWithGlobe(GlobeController globeVC) throws Exception
    {
        baseCase = new StamenRemoteTestCase(getActivity());
        baseCase.setUpWithGlobe(globeVC);

        loader = makeTestLoader(getActivity(), globeVC,true);

        Point2d pt = Point2d.FromDegrees(-0.1275, 51.507222);
        globeVC.setPositionGeo(pt.getX(), pt.getY(), 0.4);

        return true;
    }

    @Override
    public boolean setUpWithMap(MapController mapVC) throws Exception
    {
        baseCase = new StamenRemoteTestCase(getActivity());
        baseCase.setUpWithMap(mapVC);

        loader = makeTestLoader(getActivity(), mapVC,true);

        Point2d pt = Point2d.FromDegrees(-0.1275, 51.507222);
        mapVC.setPositionGeo(pt.getX(), pt.getY(), 0.4);

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
