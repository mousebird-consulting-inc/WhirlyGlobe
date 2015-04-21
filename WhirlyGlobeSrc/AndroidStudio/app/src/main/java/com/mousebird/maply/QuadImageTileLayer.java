/*
 *  QuadImageTileLayer.java
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

import android.graphics.Bitmap;
import android.os.Handler;
import android.os.Looper;

/**
 * The quad image tiling layer manages a self contained basemap.  Basemaps are
 * local or remote collections of tiny images, often 256 pixels on a side.  This
 * object pulls the ones it needs from a remote or local source and displays them
 * as a coherent whole to the user
 * <p>
 * You'll need to create one of these and add it to the layerThread.  You'll
 * also need to pass it a TileSource.  That will do the actual data fetching
 * on demand.
 * 
 * @author sjg
 *
 */
public class QuadImageTileLayer extends Layer implements LayerThread.ViewWatcherInterface
{
	// Set when the layer is active.
	boolean valid = false;
	
	private QuadImageTileLayer()
	{
	}

	/**
	 * The Tile Source is the interface used to actually fetch individual images for tiles.
	 * You fill this in to provide data for remote or local tile queries.
	 * 
	 * @author sjg
	 *
	 */
	public interface TileSource
	{
		/**
		 * The minimum zoom level you'll be called about to create a tile for.
		 */
		public int minZoom();
		/**
		 * The maximum zoom level you'll be called about to create a tile for.
		 */
		public int maxZoom();

		/**
		 * This tells you when to start fetching a given tile. When you've fetched
		 * the image you'll want to call loadedTile().  If you fail to fetch an image
		 * call that with nil.
		 * 
		 * @param layer The layer asking for the fetch.
		 * @param tileID The tile ID to fetch.
		 */
		public void startFetchForTile(QuadImageTileLayer layer,MaplyTileID tileID);
	}
	
	public MaplyBaseController maplyControl = null;
	public CoordSystem coordSys = null;
	TileSource tileSource = null;
	
	/**
	 * Construct a quad image tile layer.  You'll pass it over to the MaplyController to
	 * handle.
	 * 
	 * @param inMaplyControl The maply controller this will be part of.
	 * @param inCoordSys Coordinate system the layer will work in, probably Spherical Mercator.
	 * @param inTileSource Tile source for images.
	 */
	public QuadImageTileLayer(MaplyBaseController inMaplyControl,CoordSystem inCoordSys,TileSource inTileSource)
	{
		maplyControl = inMaplyControl;
		coordSys = inCoordSys;
		tileSource = inTileSource;
		ChangeSet changes = new ChangeSet();
		initialise(coordSys,changes);
		maplyControl.layerThread.addChanges(changes);
		setSimultaneousFetches(8);
	}
	
	public void finalize()
	{
		dispose();
	}
	
	@Override
	public float getMinTime() 
	{
		// Update every 1/10s
		return 0.1f;
	}

	@Override
	public float getMaxLagTime() 
	{
		// Want an update no less often than this
		// Note: What?
		return 4.0f;
	}	

	/**
	 * Called by the layer thread.  Don't call this directly.
	 */
	public void startLayer(LayerThread layerThread)
	{
		super.startLayer(layerThread);
		layerThread.addWatcher(this);
		Point2d ll = new Point2d(coordSys.ll.getX(),coordSys.ll.getY());
		Point2d ur = new Point2d(coordSys.ur.getX(),coordSys.ur.getY());
		nativeStartLayer(layerThread.scene,layerThread.renderer,ll,ur,0,tileSource.maxZoom());
		valid = true;
	}

	/**
	 * Called by the layer thread.  Don't call this directly.
	 */
	public void shutdown()
	{
		valid = false;
		layerThread.removeWatcher(this);
		cancelEvalStep();
		ChangeSet changes = new ChangeSet();
		nativeShutdown(changes);
		layerThread.addChanges(changes);
		super.shutdown();
	}

	/**
	 * The view updated.  Called by the layer thread.  Don't call this directly.
	 */
	@Override
	public void viewUpdated(ViewState viewState) 
	{
		if (!valid)
			return;
		
		nativeViewUpdate(viewState);

		scheduleEvalStep();
	}

	Handler evalStepHandle = null;
	Runnable evalStepRun = null;

	// Cancel the current evalStep
	void cancelEvalStep()
	{
		if (!valid)
			return;

		synchronized(this)
		{
			if (evalStepHandle != null)
			{
				evalStepHandle.removeCallbacks(evalStepRun);
				evalStepHandle = null;
				evalStepRun = null;
			}		
		}
	}

	// Post an evalStep if there isn't one scheduled
	void scheduleEvalStep()
	{
		if (!valid)
			return;
//		cancelEvalStep();

		synchronized(this)
		{
			if (evalStepHandle == null)
			{
				evalStepRun = new Runnable()
				{
					@Override
					public void run()
					{
						evalStep();
					}
				};
				evalStepHandle = layerThread.addTask(evalStepRun,true);
			}
		}
	}
	
	// Do something small and then return
	void evalStep()
	{
		if (!valid)
			return;

		synchronized(this)
		{
			evalStepHandle = null;
			evalStepRun = null;
		}
		
		// Note: Check that the renderer is set up and such.
		ChangeSet changes = new ChangeSet();
		boolean didSomething = nativeEvalStep(changes);
		layerThread.addChanges(changes);
		if (didSomething)
			scheduleEvalStep();
	}

	/**
	 * If you call this, the layer will clear out all current geometry
	 * and refetch everything.
	 */
	public void refresh()
	{
		if (!valid)
			return;

		// Make sure this runs on the layer thread
		if (Looper.myLooper() != layerThread.getLooper())
		{
			Handler handle = new Handler();
			handle.post(
					new Runnable() 
					{
						@Override
						public void run()
						{
							refresh();
						}
					});
			return;
		}
		
		ChangeSet changes = new ChangeSet();
		boolean doEvalStep = nativeRefresh(changes);
		layerThread.addChanges(changes);
		if (doEvalStep)
			scheduleEvalStep();
	}
	
	/* Called by the JNI side.  We need to start fetching
	 * the given tile.
	 */
	void startFetch(int level,int x,int y)
	{
		if (!valid)
			return;

		MaplyTileID tileID = new MaplyTileID(x,y,level);
		tileSource.startFetchForTile(this, tileID);
	}
	
	/*
	 * Called by the JNI side.  We're being woken up 
	 */
	
	/**
	 * When a tile source finishes loading a given image tile,
	 * it calls this method to let the quad image tile layer know
	 * about it.  You can call this on any thread.
	 * 
	 * @param imageTile The image tile we've just loaded.  Pass in null on failure.
	 */
	public void loadedTile(final MaplyTileID tileID,final MaplyImageTile imageTile)
	{
		if (!valid)
			return;

		if (Looper.myLooper() != layerThread.getLooper())
		{
			layerThread.addTask(new Runnable()
			{
				@Override
				public void run()
				{
					loadedTile(tileID,imageTile);
//					Log.d("Maply","Responding to load for tile: " + tileID.level + ": (" + tileID.x + "," + tileID.y);
				}
			});
			return;
		}
		
		ChangeSet changes = new ChangeSet();
		if (imageTile != null)
			nativeTileDidLoad(tileID.x,tileID.y,tileID.level,imageTile.bitmap,changes);
		else
			nativeTileDidNotLoad(tileID.x,tileID.y,tileID.level,changes);
		layerThread.addChanges(changes);
	}
	
	native void nativeShutdown(ChangeSet changes);
	
	/**
	 * We can only have a certain number of fetches going at once.
	 * We'll create this number of threads (in some cases) based
	 * on this number.
	 */
	public native void setSimultaneousFetches(int numFetches);
	
	/**
	 * If set we'll calculate a single target zoom level for the whole
	 * viewport, rather than evaluating tiles individually.  This works
	 * for 2D maps, but not for 3D maps or globes.
	 */
	public native void setUseTargetZoomLevel(boolean newVal);
	
	/**
	 * If set we'll skip the lower levels of the pyramid and load only
	 * the current target zoom level.
	 */
	public native void setSingleLevelLoading(boolean newVal);
	
	/**
	 * If set, we'll generate edges between the map tiles to deal with
	 * diverged tile zoom levels loaded at once.
	 */
	public native void setHandleEdges(boolean newVal);

	/**
	 * If set, we'll provide geometry for the north and south poles.
	 */
	public native void setCoverPoles(boolean newVal);

	/**
	 * By default the quad layer is always visible.  If you set these
	 * then the layer will only be visible in the given range.
	 * @param minVis The close zoom range at which to drop out the layer.  0.0 by default.
	 * @param maxVis The far zoom range at which to drop out the layer.
	 * Something implausibly large by default.
	 */
	public native void setVisibility(double minVis,double maxVis);

	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise(CoordSystem coordSys,ChangeSet changes);
	native void dispose();
	private long nativeHandle;

	native void nativeStartLayer(Scene scene,MaplyRenderer renderer,Point2d ll,Point2d ur,int minZoom,int maxZoom);
	native void nativeViewUpdate(ViewState viewState);	
	native boolean nativeEvalStep(ChangeSet changes);
	native boolean nativeRefresh(ChangeSet changes);
	native void nativeTileDidLoad(int x,int y,int level,Bitmap bitmap,ChangeSet changes);
	native void nativeTileDidNotLoad(int x,int y,int level,ChangeSet changes);
}
