package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;
import android.content.Context;
import android.content.ContextWrapper;
import android.widget.Toast;

import com.mousebird.maply.CoordSystem;
import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MBTileFetcher;
import com.mousebird.maply.MapController;
import com.mousebird.maply.BaseController;
import com.mousebird.maply.Mbr;
import com.mousebird.maply.Point2d;
import com.mousebird.maply.Point3d;
import com.mousebird.maply.QuadImageLoader;
import com.mousebird.maply.SamplingParams;
import com.mousebird.maply.SphericalMercatorCoordSystem;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;

/**
 * Create a large number of loaders
 * Created by tds on 5/2/22.
 */
public class LoaderMosaic extends MaplyTestCase {

    private static final double RAD_TO_DEG = 180.0 / Math.PI;

    private static final String TAG = "AutoTester";
    private static final String MBTILES_DIR = "mbtiles";

    public LoaderMosaic(Activity activity) {
        super(activity, "Loader Mosaic", TestExecutionImplementation.Both);
    }

    private boolean setupImageLoader(BaseController baseController,
                                     String mbTilesName,
                                     Mbr clipBounds)
    {
        File mbTiles;

        // We need to copy the file from the asset so that it can be used as a file
        try {
            synchronized (this) {
                mbTiles = this.getMbTileFile("mbtiles/" + mbTilesName, mbTilesName);
            }
        }
        catch (Exception e)
        {
            String msg = String.format("Failed to load '%s':\n%s", mbTilesName, e.getLocalizedMessage());
            Toast.makeText(getActivity().getApplicationContext(), msg, Toast.LENGTH_LONG).show();
            return false;
        }

        if (!mbTiles.exists()) {
            String msg = String.format("Could not copy MBTiles asset to \"%s\"", mbTiles.getAbsolutePath());
            Toast.makeText(getActivity().getApplicationContext(), msg, Toast.LENGTH_LONG).show();
            return false;
        }

        // The fetcher fetches tile from the MBTiles file
        MBTileFetcher fetcher = new MBTileFetcher(baseController, mbTiles);

        // Set up the parameters to match the MBTile file
        SamplingParams params = new SamplingParams();
        params.setCoordSystem(new SphericalMercatorCoordSystem());
        if (baseController instanceof GlobeController) {
            params.setCoverPoles(true);
            params.setEdgeMatching(true);
        }
        params.setSingleLevel(true);
        params.setMinZoom(0);
        params.setMaxZoom(fetcher.maxZoom);
        params.setClipBounds(clipBounds);

        QuadImageLoader loader = new QuadImageLoader(params,fetcher.getTileInfo(),baseController);
        loader.setTileFetcher(fetcher);
        loader.setBaseDrawPriority(1000 + 100 * loaders.size());

        fetchers.add(fetcher);
        loaders.add(loader);
        return true;
    }

    private final List<MBTileFetcher> fetchers = new ArrayList<>();
    private final List<QuadImageLoader> loaders = new ArrayList<>();

    private Double R2D(Double n) { return n * 180 / Math.PI; }
    private void setupLoaders(BaseController vc) {
        final CoordSystem coordSys = vc.getCoordSystem();

        final String file = "geography-class_medres.mbtiles";

        int n = 3;
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                final Point2d llGeo = Point2d.FromDegrees(-180. + (i+0) * 360./n, -90. + (j+0) * 180./n);
                final Point2d urGeo = Point2d.FromDegrees(-180. + (i+1) * 360./n, -90. + (j+1) * 180./n);
                final Point2d llLoc = coordSys.geographicToLocal(new Point3d(llGeo, 0)).toPoint2d();
                final Point2d urLoc = coordSys.geographicToLocal(new Point3d(urGeo, 0)).toPoint2d();

                //android.util.Log.w("Maply", String.format("G:%.2f,%.2f/%.2f,%.2f L:%.2f,%.2f/%.2f,%.2f",
                //        R2D(llGeo.getX()), R2D(urGeo.getX()), R2D(llGeo.getY()), R2D(urGeo.getY()),
                //        R2D(llLoc.getX()), R2D(urLoc.getX()), R2D(llLoc.getY()), R2D(urLoc.getY())));

                final Mbr bounds = new Mbr(llGeo, urGeo);
                //final Mbr bounds = new Mbr(llLoc, urLoc);

                setupImageLoader(vc, file, bounds);
            }
        }
    }

    @Override
    public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
        setupLoaders(globeVC);
        return true;
    }

    @Override
    public boolean setUpWithMap(MapController mapVC) throws Exception {
        setupLoaders(mapVC);
        return true;
    }

    @Override
    public void shutdown() {
        for (MBTileFetcher fetcher : fetchers) {
            fetcher.shutdown();
        }
        fetchers.clear();

        for (QuadImageLoader loader : loaders) {
            loader.shutdown();
        }
        loaders.clear();

        super.shutdown();
    }

    private File getMbTileFile(String assetMbTile, String mbTileFilename) throws IOException {

        ContextWrapper wrapper = new ContextWrapper(getActivity());
        File mbTilesDirectory =  wrapper.getDir(MBTILES_DIR, Context.MODE_PRIVATE);

        InputStream is = getActivity().getAssets().open(assetMbTile);
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
