package com.mousebirdconsulting.maply;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import org.apache.http.util.ByteArrayBuffer;

import android.content.Context;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.Looper;

public class OSMVectorTilePager implements QuadPagingLayer.PagingInterface
{
	MaplyController maplyControl = null;
	String remotePath = null;
	int minZoom = 0;
	int maxZoom = 0;
	File cacheDir = null;
	
	OSMVectorTilePager(MaplyController inMaplyControl,String inRemotePath, int inMinZoom, int inMaxZoom)
	{
		maplyControl = inMaplyControl;
		remotePath = inRemotePath;
		minZoom = inMinZoom;
		maxZoom = inMaxZoom;
	}

	@Override
	public int minZoom() {
		return minZoom;
	}

	@Override
	public int maxZoom() {
		return maxZoom;
	}

	// Connection task fetches the JSON as a string
	private class ConnectionTask extends AsyncTask<String, Void, String>
	{
		QuadPagingLayer layer = null;
		MaplyTileID tileID = null;
		OSMVectorTilePager pager = null;
		
		ConnectionTask(OSMVectorTilePager inPager,QuadPagingLayer inLayer, MaplyTileID inTileID)
		{
			layer = inLayer;
			tileID = inTileID;
			pager = inPager;
		}
		
	    @Override
	    protected String doInBackground(String... urls) {
	    	String aString = null;
	    	try {
		    	URL url = new URL(urls[0]);

	    		/* Open a connection to that URL. */
	    		final HttpURLConnection aHttpURLConnection = (HttpURLConnection) url.openConnection();

	    		/* Define InputStreams to read from the URLConnection. */
	    		InputStream aInputStream = aHttpURLConnection.getInputStream();
	    		BufferedInputStream aBufferedInputStream = new BufferedInputStream(
	    				aInputStream);

	    		/* Read bytes to the Buffer until there is nothing more to read(-1) */
	    		ByteArrayBuffer aByteArrayBuffer = new ByteArrayBuffer(50);
	    		int current = 0;
	    		while ((current = aBufferedInputStream.read()) != -1) {
	    			aByteArrayBuffer.append((byte) current);
	    		}


	    		/* Convert the Bytes read to a String. */
	    		aString = new String(aByteArrayBuffer.toByteArray());               
	    	} 
	    	catch (IOException e) {
//	    		Log.d("OSMVectorTilePager", e.toString());
	    		pager.didNotLoad(layer,tileID);
	    	}
	    	return aString;
	    }

	    @Override
	    protected void onPostExecute(String result) 
	    {
	    	if (result != null)
	    		pager.didLoad(layer,tileID,result,false);
	    	else
	    		pager.didNotLoad(layer,tileID);
	    }

	}
	
	// Generate the cache file name
	String makeTileFileName(MaplyTileID tileID)
	{
		return tileID.level + "_" + tileID.x + "_" + tileID.y + ".json";		
	}

	// The paging layer calls us here to start paging a tile
	@Override
	public void startFetchForTile(final QuadPagingLayer layer,final MaplyTileID tileID) 
	{
//		Log.i("OSMVectorTilePager","Starting Tile : " + tileID.level + " (" + tileID.x + "," + tileID.y + ")");
		
		// Look for it in the cache
		if (cacheDir != null)
		{
			String tileFileName = makeTileFileName(tileID);
			String json = readFromFile(tileFileName);
			if (!json.isEmpty())
			{
				didLoad(layer,tileID,json,true);
				return;
			}
		}
		
		// Form the tile URL
		int maxY = 1<<tileID.level;
		int remoteY = maxY - tileID.y - 1;
		final String tileURL = remotePath + "/" + tileID.level + "/" + tileID.x + "/" + remoteY + ".json";
		
		// Need to kick this task off on the main thread
		final OSMVectorTilePager pager = this;
		Handler handle = new Handler(Looper.getMainLooper());
		handle.post(new Runnable()
			{
				@Override
				public void run()
				{
					ConnectionTask task = new ConnectionTask(pager,layer,tileID);
					String[] params = new String[1];
					params[0] = tileURL;
					task.execute(params);				
				}
			});
	}
	
	// Style roads based on their type
	void styleRoads(VectorObject roads,List<ComponentObject> compObjs)
	{
		if (roads == null)
			return;
				
		for (VectorObject road : roads)
		{
			AttrDictionary attrs = road.getAttributes();
			String kind = attrs.getString("kind");
			// Note: Not really using kind yet
			
		}
		
		VectorInfo roadInfo = new VectorInfo();
		roadInfo.setColor(10.f/255.f,204/255.f,141/255.f,1.f);
		ComponentObject compObj = maplyControl.addVector(roads, roadInfo);
		compObjs.add(compObj);
	}
	
	void styleRoadLabels(VectorObject roads,List<ComponentObject> compObjs)
	{
		
	}

	void styleBuildings(VectorObject buildings,List<ComponentObject> compObjs)
	{
		
	}

	void styleLandUsage(VectorObject land,List<ComponentObject> compObjs)
	{
		
	}

	void styleWater(VectorObject water,List<ComponentObject> compObjs)
	{
		if (water == null)
			return;

		// Filled water
		VectorInfo waterInfo = new VectorInfo();
		waterInfo.setFilled(true);
		waterInfo.setColor(137.f/255.f,188.f/255.f,228.f/255.f,1.f);
		ComponentObject compObj = maplyControl.addVector(water, waterInfo);
		compObjs.add(compObj);
		
		// Outline for water
//		VectorInfo waterOutlineInfo = new VectorInfo();
//		waterOutlineInfo.setFilled(false);
//		waterOutlineInfo.setColor(137.f/255.f,188.f/255.f,228.f/255.f,1.f);
//		compObj = maplyControl.addVector(water, waterOutlineInfo);
//		compObjs.add(compObj);
	}

	// The connection task loaded data.  Yay!
	void didLoad(final QuadPagingLayer layer,final MaplyTileID tileID,final String json,final boolean wasCached)
	{
//		Log.i("OSMVectorTilePager","Loaded Tile : " + tileID.level + " (" + tileID.x + "," + tileID.y + ")");
		
		// Do the merge back on the layer thread
		layer.layerThread.addTask(
				new Runnable()
				{
					@Override
					public void run()
					{
						// Write it out to the cache
						if (!wasCached)
						{
							String tileFileName = makeTileFileName(tileID);
							writeToFile(tileFileName,json);
						}
						
						// Parse the GeoJSON assembly into groups based on the type
						Map<String,VectorObject> vecData = VectorObject.FromGeoJSONAssembly(json);

						// Work through the various top level types
						ArrayList<ComponentObject> compObjs = new ArrayList<ComponentObject>();
						styleRoads(vecData.get("highroad"),compObjs);
						styleRoadLabels(vecData.get("skeletron"),compObjs);
						styleBuildings(vecData.get("buildings"),compObjs);
						styleLandUsage(vecData.get("land-usages"),compObjs);
						styleWater(vecData.get("water-areas"),compObjs);
						
						layer.addData(compObjs, tileID);
						layer.tileDidLoad(tileID);
						
					}
				});
	}
	
	// The connection task failed to load data.  Boo!
	void didNotLoad(final QuadPagingLayer layer,final MaplyTileID tileID)
	{
//		Log.i("OSMVectorTilePager","Failed Tile : " + tileID.level + " (" + tileID.x + "," + tileID.y + ")");

		layer.layerThread.addTask(
				new Runnable()
				{
					@Override
					public void run()
					{
						layer.tileFailedToLoad(tileID);
					}
				});
	}
	
	// Write a string to file
	// Courtesy: http://stackoverflow.com/questions/14376807/how-to-read-write-string-from-a-file-in-android
	private void writeToFile(String fileName,String data) {
	    try {
	        OutputStreamWriter outputStreamWriter = new OutputStreamWriter(maplyControl.activity.openFileOutput(fileName, Context.MODE_PRIVATE));
	        outputStreamWriter.write(data);
	        outputStreamWriter.close();
	    }
	    catch (IOException e) {
//	        Log.e("Exception", "File write failed: " + e.toString());
	    } 
	}
	
	// Create a file into a string
	private String readFromFile(String inputFile) 
	{
	    String ret = "";

	    try {
	        InputStream inputStream = maplyControl.activity.openFileInput(inputFile);

	        if ( inputStream != null ) {
	            InputStreamReader inputStreamReader = new InputStreamReader(inputStream);
	            BufferedReader bufferedReader = new BufferedReader(inputStreamReader);
	            String receiveString = "";
	            StringBuilder stringBuilder = new StringBuilder();

	            while ( (receiveString = bufferedReader.readLine()) != null ) {
	                stringBuilder.append(receiveString);
	            }

	            inputStream.close();
	            ret = stringBuilder.toString();
	        }
	    }
	    catch (FileNotFoundException e) {
//	        Log.e("Maply", "File not found: " + e.toString());
	    } catch (IOException e) {
//	        Log.e("Maply", "Can not read file: " + e.toString());
	    }

	    return ret;
	}
}
