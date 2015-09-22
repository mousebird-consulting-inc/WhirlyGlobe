/*
 *  RemoteTileSource.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2014 mousebird consulting
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

import com.squareup.okhttp.OkHttpClient;
import com.squareup.okhttp.Request;
import com.squareup.okhttp.Response;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.AsyncTask;
import android.util.Log;

/**
 * The remote tile source fetches individual tiles from a remote image basemap
 * on request.  It needs basic information about the remote tile source to do this,
 * such as base URL, file extension and min and max zoom level.
 * 
 * This object works on conjunction with a QuadImageTileLayer.
 *
 */
public class RemoteTileSource implements QuadImageTileLayer.TileSource
{
	RemoteTileInfo tileInfo = null;
	public CoordSystem coordSys = new SphericalMercatorCoordSystem();
	OkHttpClient client = new OkHttpClient();
	
	/**
	 * The tile source delegate will be called back when a tile loads
	 * or fails to load.
	 * <p>
	 * You use these for tracking what the remote tile source is up to.
	 * They're not required, but they are handy for capturing failures.
	 *
	 */
	public interface TileSourceDelegate
	{
		/**
		 * Tile successfully loaded.
		 * @param tileSource Tile source that just loaded the tile.
		 * @param tileID Tile ID
		 */
		public void tileDidLoad(Object tileSource,MaplyTileID tileID,int frame);
		/**
		 * Tile failed to load.
		 * @param tileSource Tile source that failed to load the tile.
		 * @param tileID Tile ID.
		 */
		public void tileDidNotLoad(Object tileSource,MaplyTileID tileID,int frame);
	}
	
	/**
	 * Set this delegate to get callbacks when tiles load or fail to load.
	 */
	public TileSourceDelegate delegate = null;
	
	/**
	 * 
	 * @param inTileInfo
	 */
	public RemoteTileSource(RemoteTileInfo inTileInfo)
	{
		tileInfo = inTileInfo;
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

	@Override
	public int minZoom() 
	{
		return tileInfo.minZoom;
	}

	@Override
	public int maxZoom() 
	{
		return tileInfo.maxZoom;
	}

	@Override
	public int pixelsPerSide() { return tileInfo.pixelsPerSide; }
	
	// Connection task fetches the image
	private class ConnectionTask extends AsyncTask<String, Void, Bitmap>
	{
		RemoteTileSource tileSource = null;
		QuadImageTileLayer layer = null;
		MaplyTileID tileID = null;
		URL url = null;
		String locFile = null;
		
		ConnectionTask(QuadImageTileLayer inLayer,RemoteTileSource inTileSource, MaplyTileID inTileID,String inURL,String inFile)
		{
			tileSource = inTileSource;
			layer = inLayer;
			tileID = inTileID;
			locFile = inFile;
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
		    			tileSource.delegate.tileDidLoad(tileSource,tileID,-1);
		    		layer.loadedTile(tileID, -1, imageTile);
		    	} else {
		    		if (tileSource.delegate != null)
		    			tileSource.delegate.tileDidNotLoad(tileSource,tileID,-1);
		    		layer.loadedTile(tileID, -1, null);
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
	
	/**
	 * This is called by the quad image tile layer.  Don't call this yourself.
	 */
	@Override
	public void startFetchForTile(QuadImageTileLayer layer, MaplyTileID tileID, int frame) 
	{
//		Log.d("Maply","Starting fetch for tile " + tileID.level + ": (" + tileID.x + "," + tileID.y + ")");
		
		// Form the tile URL
		int maxY = 1<<tileID.level;
		int remoteY = maxY - tileID.y - 1;
		final String tileURL = tileInfo.buildURL(tileID.x,remoteY,tileID.level);
		
		String cacheFile = null;
		if (cacheDir != null)
			cacheFile = cacheDir.getAbsolutePath() + tileInfo.buildCacheName(tileID.x, tileID.y, tileID.level);
		ConnectionTask task = new ConnectionTask(layer,this,tileID,tileURL,cacheFile);
		String[] params = new String[1];
		params[0] = tileURL;
		task.executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR,params);
	}
	
}
