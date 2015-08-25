package com.mousebird.maply;

import java.util.ArrayList;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

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
		baseURLs.add(inBase);
		ext = inExt;
		minZoom = inMinZoom;
		maxZoom = inMaxZoom;
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
		return baseURLs.get(x % baseURLs.size()) + level + "/" + x + "/" + y + "." + ext;		
	}

	/**
	 * Return a unique name that can be used in the cache.
	 */
	public String buildCacheName(int x,int y,int level)
	{
		return "/" + level + "_" + x + "_" + y + "."  + ext;
	}
}
