package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;
import android.content.Context;
import android.content.ContextWrapper;
import android.graphics.Color;
import android.util.Log;

import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MBTileFetcher;
import com.mousebird.maply.MBTiles;
import com.mousebird.maply.MapController;
import com.mousebird.maply.BaseController;
import com.mousebird.maply.Mbr;
import com.mousebird.maply.OvlDebugImageLoaderInterpreter;
import com.mousebird.maply.Point2d;
import com.mousebird.maply.Point3d;
import com.mousebird.maply.QuadImageLoader;
import com.mousebird.maply.SamplingParams;
import com.mousebird.maply.SelectedObject;
import com.mousebird.maply.SphericalMercatorCoordSystem;
import com.mousebird.maply.VariableTarget;
import com.mousebirdconsulting.autotester.ConfigOptions;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * Created by sjg on 4/11/16.
 */
public class GeographyClass extends MaplyTestCase {

    private static double RAD_TO_DEG = 180.0 / Math.PI;

    private static String TAG = "AutoTester";
    private static String MBTILES_DIR = "mbtiles";

    private Activity activity;


    private GlobeController.GestureDelegate gestureDelegate = new GlobeController.GestureDelegate() {
        @Override
        public void userDidSelect(GlobeController controller, SelectedObject objs[], Point2d loc, Point2d screenLoc) {
            GeographyClass.super.userDidSelect(controller, objs, loc, screenLoc);
        }

        @Override
        public void userDidTap(GlobeController controller, Point2d loc, Point2d screenLoc) {
            GeographyClass.super.userDidTap(controller, loc, screenLoc);
        }

        @Override
        public void userDidTapOutside(GlobeController globeControl,Point2d screenLoc)
        {
            Log.d("Maply","User tapped outside globe.");
        }

        @Override
        public void userDidLongPress(GlobeController globeController, SelectedObject[] selObjs, Point2d loc, Point2d screenLoc) {
            // Intentionally blank
        }

        @Override
        public void globeDidStartMoving(GlobeController controller, boolean userInitiated) {
            // Intentionally blank
        }

        @Override
        public void globeDidStopMoving(GlobeController controller, Point3d[] corners, boolean userInitiated) {

            Point3d center = controller.getPositionGeo();
            GlobeController.ViewState viewState = controller.getViewState();

            Log.v(TAG, String.format("Globe did stop moving (lat: %.6f° lon: %.6f° z: %.6f)",
                    center.getY() * RAD_TO_DEG, center.getX() * RAD_TO_DEG, center.getZ()));

        }

        @Override
        public void globeDidMove(GlobeController controller, Point3d[] corners, boolean userInitiated) {

//            Point3d center = controller.getPosition();
//
//            Log.v(TAG, String.format("Globe did move to (lat: %.6f° lon: %.6f° z: %.6f)",
//                    center.getY() * RAD_TO_DEG, center.getX() * RAD_TO_DEG, center.getZ()));

            Mbr bb = controller.getCurrentViewGeo();
            if (bb != null) {
                Point2d center = bb.middle().toDegrees();
                Point2d span = bb.span().toDegrees();

                Log.v(TAG, String.format("Globe did move to (lat: %.6f° lon: %.6f°), span (lat: %.6f° lon: %.6f°))",
                        center.getY() * RAD_TO_DEG, center.getX() * RAD_TO_DEG,
                        span.getY() * RAD_TO_DEG, span.getX() * RAD_TO_DEG));
            }
        }
    };


    public GeographyClass(Activity activity) {
        super(activity);
        setTestName("Geography Class");
        setDelay(1000);

        this.activity = activity;
        this.implementation = TestExecutionImplementation.Both;
    }

    VariableTarget varTarget = null;

    private void setupImageLoader(BaseController baseController, String mbTilesName, int drawPriority, boolean useOffscreen, boolean transparent, ConfigOptions.TestType testType) throws Exception
    {
        File mbTiles;

        // We need to copy the file from the asset so that it can be used as a file
        try {
            mbTiles = this.getMbTileFile("mbtiles/" + mbTilesName, mbTilesName);
        }
        catch (Exception e)
        {
            return;
        }

        if (!mbTiles.exists()) {
            throw new FileNotFoundException(String.format("Could not copy MBTiles asset to \"%s\"", mbTiles.getAbsolutePath()));
        }

        Log.d(TAG, String.format("Obtained MBTiles SQLLite database \"%s\"", mbTiles.getAbsolutePath()));

        // The fetcher fetches tile from the MBTiles file
        MBTileFetcher mbTileFetcher = new MBTileFetcher(mbTiles);
        if (mbTileFetcher == null)
            return;

        // Set up the parameters to match the MBTile file
        SamplingParams params = new SamplingParams();
        params.setCoordSystem(new SphericalMercatorCoordSystem());
        if (testType == ConfigOptions.TestType.GlobeTest) {
            params.setCoverPoles(true);
            params.setEdgeMatching(true);
        }
        if (transparent)
            params.setForceMinLevel(false);
        params.setSingleLevel(true);
        params.setMinZoom(0);
        params.setMaxZoom(mbTileFetcher.maxZoom);

        // Render this to an offscreen buffer first
        if (useOffscreen) {
            varTarget = new VariableTarget(baseController);
            varTarget.buildRectangle = true;
            varTarget.drawPriority = drawPriority;
            varTarget.scale = 1.0;
        }

        QuadImageLoader loader = new QuadImageLoader(params,mbTileFetcher.getTileInfo(),baseController);
        loader.setTileFetcher(mbTileFetcher);
        loader.setBaseDrawPriority(drawPriority);
        if (transparent) {
            OvlDebugImageLoaderInterpreter interp = new OvlDebugImageLoaderInterpreter();
            loader.setLoaderInterpreter(interp);
        }
        if (useOffscreen) {
            loader.setRenderTarget(varTarget.renderTarget);
        }
    }

    @Override
    public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
        setupImageLoader(globeVC, "geography-class_medres.mbtiles", 1000,  false, false, ConfigOptions.TestType.GlobeTest);

        globeVC.gestureDelegate = gestureDelegate;

        return true;
    }

    @Override
    public boolean setUpWithMap(MapController mapVC) throws Exception {
        setupImageLoader(mapVC, "geography-class_medres.mbtiles", 1000, false, false, ConfigOptions.TestType.MapTest);
        return true;
    }

    private File getMbTileFile(String assetMbTile, String mbTileFilename) throws IOException {

        ContextWrapper wrapper = new ContextWrapper(activity);
        File mbTilesDirectory =  wrapper.getDir(MBTILES_DIR, Context.MODE_PRIVATE);

        InputStream is = activity.getAssets().open(assetMbTile);
        File of = new File(mbTilesDirectory, mbTileFilename);

        if (of.exists()) {
            return of;
        }

        OutputStream os = new FileOutputStream(of);
        byte[] mBuffer = new byte[1024];
        int length;
        while ((length = is.read(mBuffer))>0) {
            os.write(mBuffer, 0, length);
        }
        os.flush();
        os.close();
        is.close();

        return of;

    }

}
