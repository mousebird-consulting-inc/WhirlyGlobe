/*
 *  MultiplexTileSource.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 8/24/15.
 *  Copyright 2011-2015 mousebird consulting
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

package com.mousebird.maply;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.util.Log;

import com.squareup.okhttp.OkHttpClient;
import com.squareup.okhttp.Request;
import com.squareup.okhttp.Response;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.net.URL;
import java.util.HashMap;

/**
 * The multiplex tile source takes a list of remote tile info objects for the
 * purpose of fetching multiple frames per tile.  These are used by quad image
 * layers that need to animate between frames.
 */
public class MultiplexTileSource implements QuadImageTileLayer.TileSource
{
    MaplyBaseController controller = null;
	CoordSystem coordSys = null;
	RemoteTileInfo[] sources = null;
	int minZoom = 0;
	int maxZoom = 0;
	int pixelsPerSide = 256;
	OkHttpClient client = null;

	/**
	 * Return the number of individual sources and/or frames.
     */
	public int getDepth() {
		if (sources == null)
			return 0;
		return sources.length;
	}
	
	/**
	 * Set this delegate to get callbacks when tiles load or fail to load.
	 */
	public RemoteTileSource.TileSourceDelegate delegate = null;
	
	// Connection task fetches a single image
	private class ConnectionTask implements com.squareup.okhttp.Callback {
        MultiplexTileSource tileSource = null;
        QuadImageTileLayerInterface layer = null;
        MaplyTileID tileID = null;
        int frame = -1;
        URL url = null;
        String locFile = null;
        com.squareup.okhttp.Call call;
        Bitmap bm = null;
        File cacheFile = null;
        boolean isCanceled = false;
		public boolean singleFetch = false;

        ConnectionTask(QuadImageTileLayerInterface inLayer, MultiplexTileSource inTileSource, MaplyTileID inTileID, int inFrame, String inURL, String inFile) {
            tileSource = inTileSource;
            layer = inLayer;
            tileID = inTileID;
            locFile = inFile;
            frame = inFrame;
            try {
                url = new URL(inURL);
            } catch (IOException e) {

            }
        }

        // Either fetch the tile from the local cache or fetch it remotely
        protected void fetchTile() {
            try {
                // See if it's here locally
                if (locFile != null) {
                    cacheFile = new File(locFile);
                    if (cacheFile.exists()) {
                        BufferedInputStream aBufferedInputStream = new BufferedInputStream(new FileInputStream(cacheFile));
                        bm = BitmapFactory.decodeStream(aBufferedInputStream);
//                        Log.d("Maply", "Read cached file for tile " + tileID.level + ": (" + tileID.x + "," + tileID.y + ")");
                    }
                }

                if (bm != null) {
                    reportTile(true);
                    return;
                }

                // Load the data from that URL
                Request request = new Request.Builder().url(url).build();

                call = client.newCall(request);
                call.enqueue(this);
            } catch (Exception e) {
            }
        }

        // Callback from OK HTTP on tile loading failure
        public void onFailure(Request request, IOException e) {
            Log.e("Maply", "Failed to fetch remote tile " + tileID.level + ": (" + tileID.x + "," + tileID.y + ")" + " " + frame);
        }

        // Callback from OK HTTP on success
        public void onResponse(Response response) {
            if (isCanceled)
                return;

			if (response.code() != 404) {
				byte[] rawImage = null;
				try {
					rawImage = response.body().bytes();
					BitmapFactory.Options options = new BitmapFactory.Options();
					//                options.inScaled = false;
					//                options.inPremultiplied = false;
					options.inScaled = false;
					options.inDither = false;
					options.inPreferQualityOverSpeed = true;
					options.inPreferredConfig = Bitmap.Config.ARGB_8888;
					bm = BitmapFactory.decodeByteArray(rawImage, 0, rawImage.length, options);

					// Save to cache
					if (cacheFile != null && rawImage != null) {
						OutputStream fOut;
						fOut = new FileOutputStream(cacheFile);
						fOut.write(rawImage);
						fOut.close();
					}
//					Log.d("Maply", "Fetched remote file for tile " + tileID.level + ": (" + tileID.x + "," + tileID.y + ")");
				} catch (Exception e) {
//					Log.e("Maply", "Failed to fetch remote tile " + tileID.level + ": (" + tileID.x + "," + tileID.y + ")" + " " + frame);
				}
			} else {
//				Log.d("Maply", "Fetch failed for remote tile " + tileID.level + ": (" + tileID.x + "," + tileID.y + ")");
			}

			boolean reportThisTile = false;
			boolean tileSuccess = bm != null;

			// Fetched a frame.  So see if we need to report this tile.
			if (frame != -1) {
				SortedTile tile = null;
				synchronized (tileSource.tiles) {
					tile = tileSource.tiles.get(tileID);
				}
				if (tile != null) {
					tile.tileData[frame] = bm;
					if (singleFetch || tile.isDone())
						reportThisTile = true;
				}
			} else {
				reportThisTile = true;
			}

			if (reportThisTile)
				reportTile(tileSuccess);
        }

        // Let the system know we've got a tile
        protected void reportTile(final boolean tileSuccess) {
            layer.getLayerThread().addTask(new Runnable() {
                @Override
                public void run() {
					SortedTile tile = null;
					synchronized (tileSource.tiles) {
						tile = tileSource.tiles.get(tileID);
					}

                    // Let the layer and delegate know what happened with it
                    if (tileSuccess) {
						MaplyImageTile imageTile = null;
						if (singleFetch)
							imageTile = new MaplyImageTile(bm);
						else
							imageTile = new MaplyImageTile(tile.tileData);
						if (tileSource.delegate != null)
							tileSource.delegate.tileDidLoad(tileSource, tileID, frame);
						layer.loadedTile(tileID, frame, imageTile);
                    } else {
                        if (tileSource.delegate != null)
                            tileSource.delegate.tileDidNotLoad(tileSource, tileID, frame);
                        layer.loadedTile(tileID, frame, null);
                    }

                    // Tile was fetched, clean up
                    if (tile != null) {
                        tile.finish(frame);
                    }
                }
            });
        }

        // Cancel an outstanding request
        protected void cancel() {
            isCanceled = true;
            if (call != null)
                call.cancel();
        }
    }

	// Used to track tiles we're in the process of loading
	class SortedTile implements Comparable<SortedTile>
	{
		MaplyTileID ident = null;
		int depth;
		Bitmap[] tileData = null;
        ConnectionTask[] fetches = null;
		
		public SortedTile(MaplyTileID inTileID,int inDepth)
		{
			depth = inDepth;
			ident = inTileID;
			tileData = new Bitmap[depth];
			fetches = new ConnectionTask[depth];
			for (int ii=0;ii<depth;ii++)
			{
				tileData[ii] = null;
				fetches[ii] = null;
			}
		}
		
		@Override
		public int compareTo(SortedTile that) 
		{
			return ident.compareTo(that.ident);
		}
		
	    // Kill any outstanding fetches
		void cancelAll()
		{
			for (int ii=0;ii<depth;ii++)
			{
				if (fetches[ii] != null)
					fetches[ii].cancel();
				fetches[ii] = null;
				tileData[ii] = null;
			}
		}
		
	    // Kill a specific outstanding fetch
		void cancel(int frame)
		{
	        int which = (frame == -1 ? 0 : frame);

	        clearFetch(frame);
	        tileData[which] = null;
		}
		
	    // Clear the fetch for a given frame
		void clearFetch(int frame)
		{
	        int which = (frame == -1 ? 0 : frame);
			
	        if (fetches[which] != null)
	        {
				fetches[which].cancel();
				fetches[which] = null;
	        }	        
		}

		// Check if all the frames loaded
		boolean isDone()
		{
			for (Bitmap bm : tileData)
				if (bm == null)
					return false;

			return true;
		}

        // Clear out for a successful fetch
        void finish(int frame)
        {
            int which = (frame == -1 ? 0 : frame);
            if (fetches[which] != null)
                fetches[which] = null;
        }
		
	    // Number of active fetches
		int numActiveFetches()
		{
			int num = 0;
			for (int ii=0;ii<depth;ii++)
				if (fetches[ii] != null)
					num++;
			
			return num;
		}
	}
	
	// Tiles in the process of being loaded
	HashMap<MaplyTileID,SortedTile> tiles = new HashMap<MaplyTileID,SortedTile>();
	
	/**
	 * Construct with a list of tile sources.  One source per frame and each source
	 * needs to be identical in size and min/max zoom levels.
	 */
	public MultiplexTileSource(MaplyBaseController inController,RemoteTileInfo[] inSources,CoordSystem inCoordSys)
	{
        controller = inController;
		sources = inSources;

        client = controller.getHttpClient();
		
		if (sources.length == 0)
			return;
		
		minZoom = sources[0].minZoom;
		maxZoom = sources[0].maxZoom;
		coordSys = inCoordSys;
		
		for (RemoteTileInfo source : sources)
		{
			minZoom = Math.max(source.minZoom,minZoom);
			maxZoom = Math.min(source.maxZoom,maxZoom);
		}
		
		if (minZoom > maxZoom)
			throw new IllegalArgumentException();
	}
	
	File cacheDir = null;
	/**
	 * Set the cache directory for fetched images.  We'll look there first.
	 * There is no limiting or pruning going on, that directory will just get
	 * bigger and bigger.
	 * <p>
	 * By default that directory is null.
	 * 
	 * @param inCacheDir Cache directory for image tiles.
	 */
	public void setCacheDir(File inCacheDir)
	{
		cacheDir = inCacheDir;
	}
	
	// Clear fetches for a given tile/frame
	void clearFetches(MaplyTileID tileID,int frame)
	{
		synchronized(this)
		{
			SortedTile tile = tiles.get(tileID);
			if (tile != null)
			{
				tile.cancel(frame);
				if (tile.numActiveFetches() == 0)
					tiles.remove(tileID);
			}
		}
	}
	
	/**
	 * Returns the coordinate system for the remote tiles.
	 */
	public CoordSystem getCoordSystem()
	{
		return coordSys;
	}

	@Override
	public int minZoom() {
		return minZoom;
	}

	@Override
	public int maxZoom() {
		return maxZoom;
	}

	@Override
	public int pixelsPerSide() { return pixelsPerSide; }

	@Override
	public void startFetchForTile(QuadImageTileLayerInterface layer, MaplyTileID tileID, int frame)
	{		
//		Log.d("Maply","Multiplex Load: " + tileID.level + ": (" + tileID.x + "," + tileID.y + ")" + " " + frame);
		
		// Form the tile URL
		int maxY = 1<<tileID.level;
		int remoteY = maxY - tileID.y - 1;

		// Look for an existing tile
		synchronized(tiles)
		{
			SortedTile tile = tiles.get(tileID);
			if (tile == null)
			{
				tile = new SortedTile(tileID,sources.length);
				tiles.put(tileID, tile);
			}
			
			int start,end;
			boolean singleFetch = false;
			if (frame == -1)
			{
				start = 0;
				end = sources.length-1;
			} else {
				start = frame;
				end = frame;
				singleFetch = true;
			}
			for (int which=start;which<=end;which++)
			{
				if (tile.fetches[which] == null)
				{
					String cacheFile = null;
					RemoteTileInfo tileInfo = sources[which];
					final String tileURL = tileInfo.buildURL(tileID.x,remoteY,tileID.level);
					if (cacheDir != null)
						cacheFile = cacheDir.getAbsolutePath() + tileInfo.buildCacheName(tileID.x, tileID.y, tileID.level,which);
					ConnectionTask task = new ConnectionTask(layer,this,tileID,which,tileURL,cacheFile);
					task.singleFetch = singleFetch;
					tile.fetches[which] = task;
                    task.fetchTile();
				}
			}
		}
	}
}
