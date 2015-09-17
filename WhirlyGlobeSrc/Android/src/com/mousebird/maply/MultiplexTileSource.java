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

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.net.URL;
import java.util.HashMap;

import com.squareup.okhttp.OkHttpClient;
import com.squareup.okhttp.Request;
import com.squareup.okhttp.Response;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.AsyncTask;
import android.util.Log;

/**
 * The multiplex tile source takes a list of remote tile info objects for the
 * purpose of fetching multiple frames per tile.  These are used by quad image
 * layers that need to animate between frames.
 */
public class MultiplexTileSource implements QuadImageTileLayer.TileSource
{
	CoordSystem coordSys = null;
	RemoteTileInfo[] sources = null;
	int minZoom = 0;
	int maxZoom = 0;
	OkHttpClient client = new OkHttpClient();
	
	/**
	 * Set this delegate to get callbacks when tiles load or fail to load.
	 */
	public RemoteTileSource.TileSourceDelegate delegate = null;
	
	// Connection task fetches a single image
	private class ConnectionTask extends AsyncTask<String, Void, Bitmap>
	{
		MultiplexTileSource tileSource = null;
		QuadImageTileLayer layer = null;
		MaplyTileID tileID = null;
		int frame = -1;
		URL url = null;
		String locFile = null;
		
		ConnectionTask(QuadImageTileLayer inLayer,MultiplexTileSource inTileSource, MaplyTileID inTileID, int inFrame,String inURL,String inFile)
		{
			tileSource = inTileSource;
			layer = inLayer;
			tileID = inTileID;
			locFile = inFile;
			frame = inFrame;
			try
			{
				url = new URL(inURL);
			}
			catch (IOException e)
			{
				
			}
		}
		
		@Override
		protected Bitmap doInBackground(String... urls)
		{
			try
			{
				// See if it's here locally
				File cacheFile = null;
				Bitmap bm = null;
				if (locFile != null)
				{
					cacheFile = new File(locFile);
					if (cacheFile.exists())
					{
						BufferedInputStream aBufferedInputStream = new BufferedInputStream(new FileInputStream(cacheFile));
			    		bm = BitmapFactory.decodeStream(aBufferedInputStream);				
//			    		Log.d("Maply","Read cached file for tile " + tileID.level + ": (" + tileID.x + "," + tileID.y + ")");
					}
				}
				
				// Wasn't cached
				if (bm == null)
				{
					// Load the data from that URL
				    Request request = new Request.Builder().url(url).build();

				    byte[] rawImage = null;
				    try
				    {
				    	Response response = client.newCall(request).execute();
				    	rawImage = response.body().bytes();
				    	bm = BitmapFactory.decodeByteArray(rawImage, 0, rawImage.length);				
				    }
				    catch (Exception e)
				    {
				    }
		    		
		    		// Save to cache
		    		if (cacheFile != null && rawImage != null)
		    		{
		    			OutputStream fOut;
		    			fOut = new FileOutputStream(cacheFile);
		    			fOut.write(rawImage);
		    			fOut.close();
		    		}
//		    		Log.d("Maply","Fetched remote file for tile " + tileID.level + ": (" + tileID.x + "," + tileID.y + ")");
				}
	    		
				// Let the layer and delegate know what happened with it
		    	if (bm != null)
		    	{
		    		MaplyImageTile imageTile = new MaplyImageTile(bm);
		    		if (tileSource.delegate != null)
		    			tileSource.delegate.tileDidLoad(tileSource,tileID,frame);
		    		layer.loadedTile(tileID, frame, imageTile);
		    	} else {
		    		if (tileSource.delegate != null)
		    			tileSource.delegate.tileDidNotLoad(tileSource,tileID,frame);
		    		layer.loadedTile(tileID, frame, null);
		    	}
		    	
		    	return null;
			}
			catch (IOException e)
			{
				return null;
			}
			catch (Exception e)
			{
				Log.d("Maply","Remote tile error: " + e);
				return null;
			}
		}

	    @Override
	    protected void onPostExecute(Bitmap bm) 
	    {
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
					fetches[ii].cancel(true);
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
				fetches[which].cancel(true);
				fetches[which] = null;
	        }	        
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
	public MultiplexTileSource(RemoteTileInfo[] inSources,CoordSystem inCoordSys)
	{
		sources = inSources;
		
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
	public void startFetchForTile(QuadImageTileLayer layer, MaplyTileID tileID, int frame) 
	{		
		Log.d("Maply","Multiplex Load: " + tileID.level + ": (" + tileID.x + "," + tileID.y + ")" + " " + frame);
		
		// Form the tile URL
		int maxY = 1<<tileID.level;
		int remoteY = maxY - tileID.y - 1;

		// Look for an existing tile
		synchronized(this)
		{
			SortedTile tile = tiles.get(tileID);
			if (tile == null)
			{
				tile = new SortedTile(tileID,sources.length);
				tiles.put(tileID, tile);
			}
			
			int start,end;
			if (frame == -1)
			{
				start = 0;
				end = sources.length-1;
			} else {
				start = frame;
				end = frame;
			}
			for (int which=start;which<=end;which++)
			{
				if (tile.fetches[which] == null)
				{
					String cacheFile = null;
					RemoteTileInfo tileInfo = sources[which];
					final String tileURL = tileInfo.buildURL(tileID.x,remoteY,tileID.level);
					if (cacheDir != null)
						cacheFile = cacheDir.getAbsolutePath() + tileInfo.buildCacheName(tileID.x, tileID.y, tileID.level,frame);
					ConnectionTask task = new ConnectionTask(layer,this,tileID,frame,tileURL,cacheFile);
					tile.fetches[which] = task;
					String[] params = new String[1];
					params[0] = tileURL;
					task.executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR,params);
				}
			}
		}
	}
}
