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

import java.util.ArrayList;
import java.util.List;

/**
 * The remote tile source fetches individual tiles from a remote image basemap
 * on request.  It needs basic information about the remote tile source to do this,
 * such as base URL, file extension and min and max zoom level.
 * 
 * This object works on conjuction with a QuadImageTileLayer.
 *
 */
public class MultiplexTileSource implements QuadImageTileLayer.TileSource
{
	ArrayList<QuadImageTileLayer.TileSource> tileSources = new ArrayList<QuadImageTileLayer.TileSource>();
	String ext = null;
	int minZoom = 0;
	int maxZoom = 0;
	public CoordSystem coordSys = new SphericalMercatorCoordSystem();

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
		public void tileDidLoad(MultiplexTileSource tileSource, MaplyTileID tileID, int frame);
		/**
		 * Tile failed to load.
		 * @param tileSource Tile source that failed to load the tile.
		 * @param tileID Tile ID.
		 */
		public void tileDidNotLoad(MultiplexTileSource tileSource, MaplyTileID tileID, int frame);
	}

	/**
	 * Set this delegate to get callbacks when tiles load or fail to load.
	 */
	public TileSourceDelegate delegate = null;

	/**
	 * Construct a remote tile source that fetches from a single URL.  You provide
	 * the base URL and the extension as well as min and max zoom levels.
	 *
	 * @param sources
	 */
	public MultiplexTileSource(List<QuadImageTileLayer.TileSource> sources)
	{
		tileSources.addAll(sources);
		for(int ii=0;ii<tileSources.size(); ii++) {
			maxZoom = tileSources.get(ii).maxZoom();
			minZoom = tileSources.get(ii).minZoom();
		}
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
	
	/**
	 * This is called by the quad image tile layer.  Don't call this yourself.
	 */
	@Override
	public void startFetchForTile(QuadImageTileLayer layer, MaplyTileID tileID, int frame)
	{
//		Log.d("Maply","Starting fetch for tile " + tileID.level + ": (" + tileID.x + "," + tileID.y + ")");

		if(frame<tileSources.size()) {
			if(frame<0) frame = 0;
			tileSources.get(frame).startFetchForTile(layer, tileID, frame);
		}
	}
	
}
