package com.mousebirdconsulting.maplyandroidtester;

/**
 * Project MaplyAndroidTester
 * Created by jmc on 22/01/16.
 */

import android.content.Context;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.AsyncTask;
import android.util.Log;

import com.mousebird.maply.MaplyImageTile;
import com.mousebird.maply.MaplyTileID;
import com.mousebird.maply.QuadImageTileLayer;
import com.mousebird.maply.QuadImageTileLayerInterface;

import java.io.IOException;

/**
 * Custom TileSource that reads OSM tiles from a local asset directory
 */
public class LocalAssetsTileProvider implements QuadImageTileLayer.TileSource {

    //***********************************************************************//
    //                           Inner classes                               //
    //***********************************************************************//

    private class TileLoaderTask {
        public QuadImageTileLayerInterface layerInterface;
        public MaplyTileID tileID;
        public int frame;


        public TileLoaderTask(QuadImageTileLayerInterface layerInterface, MaplyTileID tileID, int frame) {
            this.layerInterface = layerInterface;
            this.tileID = tileID;
            this.frame = frame;
        }

    }

    private class AsyncTileLoader extends AsyncTask<TileLoaderTask, Integer, Boolean> {

        @Override
        protected Boolean doInBackground(TileLoaderTask... tasks) {

            for (TileLoaderTask task : tasks) {

                MaplyImageTile imageTile = null;

                // Needs to convert y to the OSM reference
                int osmY = (int)Math.pow(2, task.tileID.level) - task.tileID.y - 1;

                Bitmap bm = this.getBitmap(task.tileID.x, osmY, task.tileID.level);

                if (bm != null) {
                    imageTile = new MaplyImageTile(bm);
                }

                task.layerInterface.loadedTile(task.tileID, task.frame, imageTile);
            }
            return null;
        }


        public Bitmap getBitmap(int x, int y, int l) {

            String imgSrc = String.format("%s/%s/%s.%s", l, x, y, extension);

            AssetManager assetManager = context.getAssets();

            Bitmap bitmap;
            try {
                bitmap = BitmapFactory.decodeStream(assetManager.open(imgSrc));
            } catch (IOException e) {

                Log.e(TAG, String.format("Tile %s not found...", imgSrc));
                return null;
            }

            Log.v(TAG, String.format("Returned tile [%d, %d, %d]", x, y, l));
            return bitmap;
        }
    }



    //***********************************************************************//
    //                          Class variables                              //
    //***********************************************************************//

    private static String TAG;


    //***********************************************************************//
    //                         Instance variables                            //
    //***********************************************************************//

    private int minZoom = 0;
    private int maxZoom = 0;

    // Sub directory of application assets that do contain the tiles
    private String tileDirectory;
    // Extension of the tiles ("png" or "jpg" or ...)
    private String extension;
    // Needed to access the assets
    private Context context;


    //***********************************************************************//
    //                            Constructors                               //
    //***********************************************************************//

    public LocalAssetsTileProvider(Context context, String tileDirectory, String extension, int maxZoom, int minZoom) {

        super();

        TAG = this.getClass().getSimpleName();

        this.context = context;
        this.minZoom = minZoom;
        this.maxZoom = maxZoom;
        this.tileDirectory = tileDirectory;
        this.extension = extension;
    }


    //***********************************************************************//
    //                         Getters and setters                           //
    //***********************************************************************//


    //***********************************************************************//
    //                               Interfaces                              //
    //***********************************************************************//

    /* Implements QuadImageTileLayer.TileSource */

    @Override
    public int minZoom() {
        return minZoom;
    }

    @Override
    public int maxZoom() {
        return maxZoom;
    }

    @Override
    public int pixelsPerSide() {
        return 256;
    }

    @Override
    public void startFetchForTile(QuadImageTileLayerInterface quadImageTileLayerInterface, MaplyTileID maplyTileID, int frame) {

        Log.i(TAG, "Will fire tech...");

        TileLoaderTask task = new TileLoaderTask(quadImageTileLayerInterface, maplyTileID, frame);
        AsyncTileLoader tileLoader = new AsyncTileLoader();
        tileLoader.execute(task);

        Log.v(TAG, "  -> Fired!");
    }


    //***********************************************************************//
    //                               Overrides                               //
    //***********************************************************************//


    //***********************************************************************//
    //                           Public methods                              //
    //***********************************************************************//


    //***********************************************************************//
    //                           Private methods                             //
    //***********************************************************************//


}
