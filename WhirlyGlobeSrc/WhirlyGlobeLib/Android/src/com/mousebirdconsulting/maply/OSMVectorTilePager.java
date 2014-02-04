package com.mousebirdconsulting.maply;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import org.apache.http.util.ByteArrayBuffer;

import android.graphics.Color;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.Looper;

/**
 * The OSM Vector Tile Pager reads vector tiles from a remote source,
 * probably the US OpenStreetMap server, and displays them on the
 * screen as the user moves around.
 * <p>
 * This is an example implementation of the QuadPagingLayer.PagingInterface
 * for OSM vector tiles.
 *
 * @author sjg
 *
 */
public class OSMVectorTilePager implements QuadPagingLayer.PagingInterface
{
	MaplyController maplyControl = null;
	String remotePath = null;
	int minZoom = 0;
	int maxZoom = 0;
	File cacheDir = null;
	ExecutorService executor = null;
	
	/**
	 * Construct with the data we need to start.
	 * 
	 * @param inMaplyControl The control we'll add geometry to as we go.
	 * @param inRemotePath The remote path to the vector tiles.
	 * @param inMinZoom Minimum zoom level to start at.  Normally 0, but you can cut off levels.
	 * @param inMaxZoom The maximum zoom level to zoom into.
	 * @param numThreads Number of threads we're allowed to use in the process of fetching data.
	 */
	OSMVectorTilePager(MaplyController inMaplyControl,String inRemotePath, int inMinZoom, int inMaxZoom, int numThreads)
	{
		maplyControl = inMaplyControl;
		remotePath = inRemotePath;
		minZoom = inMinZoom;
		maxZoom = inMaxZoom;
		
		// We'll keep 4 threads in a pool for parsing data
		executor = Executors.newFixedThreadPool(numThreads);
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
	
	// The paging layer calls us here to start paging a tile
	@Override
	public void startFetchForTile(final QuadPagingLayer layer,final MaplyTileID tileID) 
	{
//		Log.i("OSMVectorTilePager","Starting Tile : " + tileID.level + " (" + tileID.x + "," + tileID.y + ")");
		
		// Use one of our threads to work through this logic
		final OSMVectorTilePager thePager = this;
		executor.execute(new Runnable()
		{
			@Override
			public void run()
			{
				// Look for it in the cache
				if (cacheDir != null)
				{
					Map<String,VectorObject> vecData = readFromCache(tileID);
					if (vecData != null)
					{
						showData(layer,vecData,tileID);
						return;
					}
				}
				
				// Form the tile URL
				int maxY = 1<<tileID.level;
				int remoteY = maxY - tileID.y - 1;
				final String tileURL = remotePath + "/" + tileID.level + "/" + tileID.x + "/" + remoteY + ".json";
				
				// Need to kick this task off on the main thread
				final OSMVectorTilePager pager = thePager;
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
		});
	}

	// Group data together for efficiency
	class VectorGroup
	{
		public ArrayList<VectorObject> vecs = new ArrayList<VectorObject>();
	};
	
	// Sort vectors into groups based on their kind
	HashMap<String,VectorGroup> sortIntoGroups(VectorObject vecs)
	{
		HashMap<String,VectorGroup> groups = new HashMap<String,VectorGroup>();

		// Sort the roads based on types
		for (VectorObject vec : vecs)
		{
			AttrDictionary attrs = vec.getAttributes();
			String kind = attrs.getString("kind");

			// Change how it looks depending on the kind
			VectorGroup group = null;
			if (groups.containsKey(kind))
				group = groups.get(kind);
			else {
				group = new VectorGroup();
				groups.put(kind, group);
			}
			group.vecs.add(vec);
		}	
		
		return groups;
	}
	
	// Style for a particular kind of road
	class RoadStyle
	{
		RoadStyle(boolean inTL,float inW,float inR,float inG,float inB,int inDP)
		{
			twoLines = inTL;
			width = inW;
			red = inR/255.f;
			green = inG/255.f;
			blue = inB/255.f;
			drawPriority = inDP;
		}
		public boolean twoLines;
		public float width;
		public float red,green,blue;
		public int drawPriority;
	};
	
	HashMap<String,RoadStyle> roadStyles = null;
	
	// Initialize road styles
	void initRoadStyles()
	{
		roadStyles = new HashMap<String,RoadStyle>();
		roadStyles.put("highway", new RoadStyle(true,10.f,204.f,141.f,4.f,400));
		roadStyles.put("major_road", new RoadStyle(true,6.f,239.f,237.f,88.f,402));
		roadStyles.put("minor_road", new RoadStyle(false,2.f,64,64,64,404));
		roadStyles.put("rail", new RoadStyle(true,6.f,100,100,100,406));
		roadStyles.put("path", new RoadStyle(false,1.f,64,64,64,408));
	}
		
	// Style roads based on their type
	void styleRoads(VectorObject roads,List<ComponentObject> compObjs)
	{
		if (roads == null)
			return;
		
		if (roadStyles == null)
			initRoadStyles();
/*
		VectorInfo roadInfo = new VectorInfo();
		roadInfo.setColor(0,0,0,1.f);
		roadInfo.setDrawPriority(400);
		roadInfo.setLineWidth(2.f*2);
		roadInfo.setEnable(false);
		compObjs.add(maplyControl.addVector(roads, roadInfo));
	*/	
		HashMap<String,VectorGroup> groups = sortIntoGroups(roads);
		
		// Note: Scale up for high res displays
		float scale = 2;
		
		// Now work through what we find, matching up to styles
		for (String roadType : groups.keySet())
		{
			VectorGroup group = groups.get(roadType);
			RoadStyle roadStyle = roadStyles.get(roadType);
			if (roadStyle == null)
				roadStyle = roadStyles.get("minor_road");

			// Base line underneath road
			if (roadStyle.twoLines)
			{
				VectorInfo roadInfo = new VectorInfo();
				roadInfo.setColor(roadStyle.red/2.f,roadStyle.green/2.f,roadStyle.blue/2.f,1.f);
				roadInfo.setDrawPriority(roadStyle.drawPriority);
				roadInfo.setLineWidth(roadStyle.width*scale);
				roadInfo.setEnable(false);
				compObjs.add(maplyControl.addVectors(group.vecs, roadInfo));
			}
			
			// Road itself
			VectorInfo roadInfo = new VectorInfo();
			roadInfo.setColor(roadStyle.red,roadStyle.green,roadStyle.blue,1.f);
			roadInfo.setDrawPriority(roadStyle.drawPriority+1);
			roadInfo.setLineWidth(roadStyle.width*scale);
			roadInfo.setEnable(false);
			compObjs.add(maplyControl.addVectors(group.vecs, roadInfo));
		}
	}
	
	void styleRoadLabels(VectorObject roads,List<ComponentObject> compObjs)
	{		
	}

	void styleBuildings(VectorObject buildings,List<ComponentObject> compObjs)
	{
		if (buildings == null)
			return;
		
		VectorInfo buildingInfo = new VectorInfo();
		buildingInfo.setColor(1.f,186.f/255.f,103.f/255.f,1.f);
		buildingInfo.setFilled(true);
		buildingInfo.setDrawPriority(601);
		ComponentObject compObj = maplyControl.addVector(buildings,buildingInfo);
		compObjs.add(compObj);
	}
	
	// Land styles are just colors
	HashMap<String,Integer> landStyles = null;
	void initLandStyles()
	{
		landStyles = new HashMap<String,Integer>();
		int alpha = 255/4;
		Integer green = Color.argb(alpha,111,224,136);
		Integer darkGreen = Color.argb(alpha,111,224,136);
		Integer tan = Color.argb(alpha,210,180,140);
		Integer gray = Color.argb(alpha,(int) 0.1f*255,0,0);
		Integer grayer = Color.argb(alpha,(int) 0.2*255,0,0);
		Integer grayest = Color.argb(alpha,(int) 0.3*255,0,0);
		landStyles.put("scrub", green);
		landStyles.put("park", green);
		landStyles.put("school", gray);
		landStyles.put("meadow", tan);
		landStyles.put("nature_reserve", green);
		landStyles.put("garden", green);
		landStyles.put("pitch", green);
		landStyles.put("wood", darkGreen);
		landStyles.put("farm", tan);
		landStyles.put("farmyard", tan);
		landStyles.put("recreation_ground", green);
		// Note: There's an awful lot of these
		landStyles.put("commercial", grayer);
		landStyles.put("residential", gray);
		landStyles.put("industrial", grayest);
		landStyles.put("common", gray);
		landStyles.put("parking", gray);
		landStyles.put("default", gray);
	}

	// Style land use based on the types
	void styleLandUsage(VectorObject land,List<ComponentObject> compObjs)
	{
		if (land == null)
			return;
		
		if (landStyles == null)
			initLandStyles();

		HashMap<String,VectorGroup> groups = sortIntoGroups(land);
		
		// Now work through what we find, matching up to styles
		for (String landType : groups.keySet())
		{
			VectorGroup group = groups.get(landType);
			Integer landStyle = landStyles.get(landType);
			if (landStyle == null)
				landStyle = landStyles.get("default");

			if (landStyle != null)
			{
				VectorInfo landInfo = new VectorInfo();
				landInfo.setColor(Color.red(landStyle)/255.f, Color.green(landStyle)/255.f, Color.blue(landStyle)/255.f, Color.alpha(landStyle)/255.f);
				landInfo.setFilled(true);
				compObjs.add(maplyControl.addVectors(group.vecs, landInfo));
			}
		}		
	}

	void styleWater(VectorObject water,List<ComponentObject> compObjs)
	{
		if (water == null)
			return;

		// Filled water
		VectorInfo waterInfo = new VectorInfo();
		waterInfo.setFilled(true);
		waterInfo.setColor(137.f/255.f,188.f/255.f,228.f/255.f,1.f);
		waterInfo.setDrawPriority(200);
		ComponentObject compObj = maplyControl.addVector(water, waterInfo);
		compObjs.add(compObj);
	}
	
	// Apply the styling to show the data, and let the layer know
	void showData(QuadPagingLayer layer,Map<String,VectorObject> vecData,MaplyTileID tileID)
	{
		// Work through the various top level types
		ArrayList<ComponentObject> compObjs = new ArrayList<ComponentObject>();
		styleRoads(vecData.get("highroad"),compObjs);
		styleRoadLabels(vecData.get("skeletron"),compObjs);
		styleBuildings(vecData.get("buildings"),compObjs);
		// Note: The land usage is kind of a mess
//		styleLandUsage(vecData.get("land-usages"),compObjs);
		styleWater(vecData.get("water-areas"),compObjs);
		
		layer.addData(compObjs, tileID);
		layer.tileDidLoad(tileID);		
	}

	// The connection task loaded data.  Yay!
	void didLoad(final QuadPagingLayer layer,final MaplyTileID tileID,final String json,final boolean wasCached)
	{
//		Log.i("OSMVectorTilePager","Loaded Tile : " + tileID.level + " (" + tileID.x + "," + tileID.y + ")");

		// Do the merge (which can take a while) on one of our thread pool threads
		executor.execute(
				new Runnable()
				{
					@Override
					public void run()
					{						
						// Parse the GeoJSON assembly into groups based on the type
						Map<String,VectorObject> vecData = VectorObject.FromGeoJSONAssembly(json);
						
						// And display it
						showData(layer,vecData,tileID);
					
						// Write it out to the cache
						if (!wasCached)
							writeToCache(vecData,tileID);
					}
				}
		);
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
	
	// Filename for cache with type
	String cacheName(MaplyTileID tileID,String type)
	{
		String fileName = type + "_" + tileID.level + "_" + tileID.x + "_" + tileID.y + ".bvec";
		
		return maplyControl.activity.getCacheDir().getAbsolutePath() + "/" + fileName;
	}
	
	// Write the data layers out to the cache
	void writeToCache(Map<String,VectorObject> vecData,MaplyTileID tileID)
	{
		// Write each type into its own cache file
		for (String type : vecData.keySet())
		{
			VectorObject vecObj = vecData.get(type);
			String fileName = cacheName(tileID,type);
			vecObj.writeToFile(fileName);
		}
	}
	
	// Read the data layers out of the cache
	Map<String,VectorObject> readFromCache(MaplyTileID tileID)
	{
		Map<String,VectorObject> vecData = new HashMap<String,VectorObject>();

		// Work through the types we're expecting to find
		String[] types = {"highroad","skeletron","buildings","land-usages","water-areas"};
		for (String type : types)
		{
			String fileName = cacheName(tileID,type);
			File theFile = new File(fileName);
			if (theFile.exists())
			{
				VectorObject vecObj = new VectorObject();
				vecObj.readFromFile(fileName);
				vecData.put(type, vecObj);
			}
		}
		
		if (vecData.size() > 0)
			return vecData;
		return null;
	}
}
