package com.mousebird.maply;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;

/**
 * The RemoteTileInfo class holds the contact info associated with a remote tile source.
 * This includes base URLs, min and max zoom levels and other info needed construct full URLs.
 * 
 */
public class RemoteTileInfo
{
	ArrayList<String> baseURLs = new ArrayList<String>();
	String ext = null;
	int minZoom = 0;
	int maxZoom = 0;
	int pixelsPerSide = 256;
	boolean replaceURL = false;

	/**
	 * Construct a remote tile source that fetches from a single URL.  You provide
	 * the base URL and the extension as well as min and max zoom levels.
	 * 
	 * @param inBase The base URL we'll fetching tiles from
	 * @param inExt
	 * @param inMinZoom
	 * @param inMaxZoom
	 */
	public RemoteTileInfo(String inBase,String inExt,int inMinZoom,int inMaxZoom)
	{
		if (inBase.contains("{x}") || inBase.contains("{y}"))
			replaceURL = true;

		baseURLs.add(inBase);
		ext = inExt;
		minZoom = inMinZoom;
		maxZoom = inMaxZoom;
	}

	String timeKey = null;

	/**
	 * We'll use the time key when caching local files.
	 * This avoids a naming problem with x_y_level that might occur if you're doing time based data sources.
	 * @param timeKey
     */
	public void setTimeKey(String inTimeKey)
	{
		timeKey = inTimeKey;
	}
	
	/**
	 * Construct a remote tile info based on a JSON spec.  This includes multiple
	 * paths to fetch from, min and max zoom and other information.
	 * 
	 * @param json The parsed JSON object to tease our information from.
	 */
	public RemoteTileInfo(JSONObject json)
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

	/**
	 * Construct a URL for a given tile
	 */
	public String buildURL(int x,int y,int level)
	{
		String url = null;
		if (replaceURL)
			url = baseURLs.get( x % baseURLs.size()).replace("{x}","" + x).replace("{y}","" + y).replace("{z}","" + level);
		else
			url = baseURLs.get(x % baseURLs.size()) + level + "/" + x + "/" + y;
		if (url !=null && ext != null)
			url = url + "." + ext;

		return url;
	}

	/**
	 * Return a unique name that can be used in the cache.
	 * This version doesn't support frames.
	 */
	public String buildCacheName(int x,int y,int level)
	{
		String name = "";
		if (timeKey != null)
			name += "/" + level + "_" + x + "_" + y + timeKey;
		else
			name += "/" + level + "_" + x + "_" + y;

		if (ext != null)
			name += "." + ext;

		return name;
	}

	/**
	 * Return a unique name that can be used in the cache.
	 * This version supports frames.
	 */
	public String buildCacheName(int x,int y,int level,int frame)
	{
		if (frame == -1)
			return buildCacheName(x, y, level);

		String name = "";
		if (timeKey != null) {
			name += "/" + level + "_" + x + "_" + y + "_" + timeKey;
		} else {
			name += "/" + level + "_" + x + "_" + y + "_" + frame;
		}
		if (ext != null)
			name += "." + ext;

		return name;
	}
}
