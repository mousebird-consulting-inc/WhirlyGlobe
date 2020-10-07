package com.mousebirdconsulting.helloearth;

import android.content.Context;
import android.content.ContextWrapper;
import android.util.Log;

import com.mousebird.maply.MBTileFetcher;
import com.mousebird.maply.QuadImageLoader;
import com.mousebird.maply.SamplingParams;
import com.mousebird.maply.SphericalMercatorCoordSystem;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class LocalGlobeFragment extends HelloGlobeFragment {

    @Override
    protected void controlHasStarted() {

        File mbTileFile;
        try {
            mbTileFile = getMBTileAsset("geography-class_medres.mbtiles");
        } catch (IOException e) {
            Log.d("HelloEarth", e.getMessage());
            return;
        }

        MBTileFetcher mbTileFetcher = new MBTileFetcher(mbTileFile);

        SamplingParams params = new SamplingParams();
        params.setCoordSystem(new SphericalMercatorCoordSystem());
        params.setCoverPoles(true);
        params.setEdgeMatching(true);
        params.setSingleLevel(true);
        params.setMinZoom(0);
        params.setMaxZoom(mbTileFetcher.maxZoom);

        QuadImageLoader loader = new QuadImageLoader(params,mbTileFetcher.getTileInfo(), baseControl);
        loader.setTileFetcher(mbTileFetcher);

        double latitude = 40.5023056 * Math.PI / 180;
        double longitude = -3.6704803 * Math.PI / 180;
        double zoom_earth_radius = 0.5;
        globeControl.animatePositionGeo(longitude, latitude, zoom_earth_radius, 1.0);
    }

    private File getMBTileAsset(String name) throws IOException {
        ContextWrapper wrapper = new ContextWrapper(getActivity());
        File mbTilesDirectory =  wrapper.getDir("mbtiles", Context.MODE_PRIVATE);

        InputStream is = getActivity().getAssets().open("mbtiles/" + name);
        File of = new File(mbTilesDirectory, name);

        if (!of.exists()) {
            OutputStream os = new FileOutputStream(of);
            byte[] mBuffer = new byte[4096];
            int length;
            while ((length = is.read(mBuffer)) > 0) {
                os.write(mBuffer, 0, length);
            }
            os.flush();
            os.close();
            is.close();
        }

        return of;
    }
}
