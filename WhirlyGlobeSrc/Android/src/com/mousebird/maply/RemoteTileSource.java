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
import java.util.ArrayList;

import org.json.*;

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
 * This object works on conjuction with a QuadImageTileLayer.
 *
 */
public class RemoteTileSource implements QuadImageTileLayer.TileSource
{
	ArrayList<String> baseURLs = new ArrayList<String>();
	String ext = null;
	int minZoom = 0;
	int maxZoom = 0;
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
		public void tileDidLoad(RemoteTileSource tileSource,MaplyTileID tileID);
		/**
		 * Tile failed to load.
		 * @param tileSource Tile source that failed to load the tile.
		 * @param tileID Tile ID.
		 */
		public void tileDidNotLoad(RemoteTileSource tileSource,MaplyTileID tileID);
	}
	
	/**
	 * Set this delegate to get callbacks when tiles load or fail to load.
	 */
	public TileSourceDelegate delegate = null;

	/**
	 * Construct a remote tile source that fetches from a single URL.  You provide
	 * the base URL and the extension as well as min and max zoom levels.
	 * 
	 * @param inBase The base URL we'll fetching tiles from
	 * @param inExt
	 * @param inMinZoom
	 * @param inMaxZoom
	 */
	public RemoteTileSource(String inBase,String inExt,int inMinZoom,int inMaxZoom)
	{
		baseURLs.add(inBase);
		ext = inExt;
		minZoom = inMinZoom;
		maxZoom = inMaxZoom;
	}
	
	/**
	 * Construct a remote tile source based on a JSON spec.  This includes multiple
	 * paths to fetch from, min and max zoom and other information.
	 * 
	 * @param json The parsed JSON object to tease our information from.
	 */
	public RemoteTileSource(JSONObject json)
	{
		try
		{
			JSONArray tileSources = json.getJSONArray("tiles");
			for (int ii=0;ii<tileSources.length();ii++)
			{
				String tileURL = tileSources.getString(ii);
				baseURLs.add(tileURL);
			}
			minZoom = json.getInt("minzoom");
			maxZoom = json.getInt("maxzoom");
			ext = "png";
		}
		catch (JSONException e)
		{
			// Note: Do something useful here
		}
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
		return minZoom;
	}

	@Override
	public int maxZoom() 
	{
		return maxZoom;
	}
	
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

				    Response response = client.newCall(request).execute();
				    byte[] rawImage = response.body().bytes();
		    		
		    		bm = BitmapFactory.decodeByteArray(rawImage, 0, rawImage.length);				
		    		
		    		// Save to cache
		    		if (cacheFile != null)
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
		    			tileSource.delegate.tileDidLoad(tileSource,tileID);
		    		layer.loadedTile(tileID, imageTile);
		    	} else {
		    		if (tileSource.delegate != null)
		    			tileSource.delegate.tileDidNotLoad(tileSource,tileID);
		    		layer.loadedTile(tileID, null);
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
	public void startFetchForTile(QuadImageTileLayer layer, MaplyTileID tileID) 
	{
//		Log.d("Maply","Starting fetch for tile " + tileID.level + ": (" + tileID.x + "," + tileID.y + ")");
		
		// Form the tile URL
		int maxY = 1<<tileID.level;
		int remoteY = maxY - tileID.y - 1;
		final String tileURL = baseURLs.get(tileID.x % baseURLs.size()) + tileID.level + "/" + tileID.x + "/" + remoteY + "." + ext;

		String cacheFile = null;
		if (cacheDir != null)
			cacheFile = cacheDir.getAbsolutePath() + "/" + tileID.level + "_" + tileID.x + "_" + tileID.y + "."  + ext;
		ConnectionTask task = new ConnectionTask(layer,this,tileID,tileURL,cacheFile);
		String[] params = new String[1];
		params[0] = tileURL;
		task.executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR,params);
	}
	
}
