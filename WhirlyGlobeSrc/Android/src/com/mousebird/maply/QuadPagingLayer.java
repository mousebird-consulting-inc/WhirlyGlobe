/*
 *  QuadPagingLayer.java
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
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import android.os.Handler;
import android.os.Looper;

/**
 * The quad paging layer is a general purpose data paging layer.  You hand it
 * an object that implements the PagingInterface and it will do the rest.
 * <p>
 * It's up to you to create geometry when called upon, but this layer will
 * handle the deletion and turning it on and off as needed when the user moves
 * around.
 * 
 */
public class QuadPagingLayer extends Layer implements LayerThread.ViewWatcherInterface
{
	// Set when the layer is operating
	boolean valid = false;
	
	private QuadPagingLayer()
	{
	}
	
	/**
	 * This is the interface paging delegates must fill in.
	 * You'll get called back on the startFetchForTile on the layerThread.
	 * Ideally, you should spawn other threads to do your fetching and
	 * processing.
	 *
	 * @author sjg
	 *
	 */
	public interface PagingInterface
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
		 * The paging layer calls your class hear to start fetching a tile.
		 * We're assuming you're making network calls and it might take a while,
		 * so DO NOT BLOCK here.  Instead, kick off your network calls and deal
		 * with the data when it comes back.
		 * <p>
		 * Calls into the QuadPagingLayer
		 * are thread safe, so do your real work on your own thread, or use
		 * the LayerThread if you need to.
		 * <p>
		 * When your data comes in you should create the visual objects you
		 * want in the maply controller and then call addData() one or more
		 * times with your new ComponentObject's.  Those visual objects
		 * should start out with enable = false.
		 * <p>
		 * Once you've loaded data, call tileDidLoad() or if you've failed
		 * call tileFailedToLoad().  The paging layer will only have a few
		 * loads outstanding at once, so you must tell it to continue loading,
		 * even on a failure.
		 * 
		 * @param layer The quad paging layer asking you to start fetching.
		 * @param tileID The tile to start fetching
		 */
		public void startFetchForTile(QuadPagingLayer layer,MaplyTileID tileID);

		/**
		 * Called when the system unloads a tile.  You need this if you're tracking
		 * data separate from ComponentObjects, which the QuadLayer will take care of.
         */
		public void tileDidUnload(MaplyTileID tileID);
	}
	
	public MaplyBaseController maplyControl = null;
	public CoordSystem coordSys = null;
	PagingInterface pagingDelegate = null;
	boolean singleLevelLoading = false;
	
	/**
	 * Construct with the information needed to page geometry into the system.
	 * 
	 * @param inMaplyControl The Maply Controller we'll be using.
	 * @param inCoordSys The coordinate system the pager is in, though not necessarily the geometry.
	 * In most cases this will be SphericalMercatorCoordSystem.
	 * @param inDelegate The paging object itself.  This is what you need to implement.
	 */
	public QuadPagingLayer(MaplyBaseController inMaplyControl,CoordSystem inCoordSys,PagingInterface inDelegate)
	{
		maplyControl = inMaplyControl;
		coordSys = inCoordSys;
		pagingDelegate = inDelegate;
		ChangeSet changes = new ChangeSet();
		initialise(coordSys,pagingDelegate,changes);
		LayerThread layerThread = maplyControl.getLayerThread();
		if (layerThread != null)
			layerThread.addChanges(changes);
		setSimultaneousFetches(8);
	}
	
	public void finalize()
	{
		dispose();
	}
	
	/**
	 * If set we'll skip the lower levels of the pyramid and load only
	 * the current target zoom level.
	 */
	public void setSingleLevelLoading(boolean newVal)
	{
		singleLevelLoading = newVal;
		nativeSetSingleLevelLoading(newVal);
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
	
	Handler evalStepHandle = null;
	Runnable evalStepRun = null;

	// Called when the layer is ready to start doing things
	public void startLayer(LayerThread layerThread)
	{
		super.startLayer(layerThread);
		layerThread.addWatcher(this);
		Point2d ll = new Point2d(coordSys.ll.getX(),coordSys.ll.getY());
		Point2d ur = new Point2d(coordSys.ur.getX(),coordSys.ur.getY());
		nativeStartLayer(layerThread.scene,layerThread.renderer,ll,ur,0,pagingDelegate.maxZoom());
		valid = true;
	}
	
	// Called when the layer shuts down.  Won't be run again.
	public void shutdown()
	{
		synchronized(loadedTiles) {
			valid = false;
			cancelEvalStep();

			// Remove the contents of all the tiles
			// Note: This is being done on the current thread.  Do we want that?
			for (LoadedTile tile : loadedTiles.values())
				tile.clear(maplyControl);
			loadedTiles.clear();

			ChangeSet changes = new ChangeSet();
			nativeShutdown(changes);
			layerThread.addChanges(changes);
			super.shutdown();
		}

		dispose();
	}

	// View state was updated (filtered through our schedule)
	@Override
	public void viewUpdated(ViewState viewState) 
	{
		if (!valid)
			return;
		
		nativeViewUpdate(viewState);

		scheduleEvalStep();
	}
	
	// Cancel the current evalStep
	void cancelEvalStep()
	{
		if (!valid)
			return;

		if (evalStepHandle != null)
		{
			evalStepHandle.removeCallbacks(evalStepRun);
			evalStepHandle = null;
			evalStepRun = null;
		}		
	}
	
	// Cancel the current evalStep and post a new one
	void scheduleEvalStep()
	{
		if (!valid)
			return;

		cancelEvalStep();
		
		evalStepRun = new Runnable()
		{
			@Override
			public void run()
			{
				evalStep();
			}
		};
		evalStepHandle = new Handler();
		evalStepHandle.post(evalStepRun);	
	}
	
	// Do something small and then return
	void evalStep()
	{
		if (!valid)
			return;

		evalStepHandle = null;
		evalStepRun = null;
		
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

	/**
	 * Calculate the bounding box in the local coordinate system.
	 * @param tileID
	 * @return
     */
	public Mbr boundsForTile(MaplyTileID tileID)
	{
		Mbr mbr = new Mbr(new Point2d(0,0),new Point2d(0,0));
		boundsForTileNative(tileID.x,tileID.y,tileID.level,mbr.ll,mbr.ur);
		return mbr;
	}
	protected native void boundsForTileNative(int x,int y,int level,Point2d ll,Point2d ur);

	/**
	 * Calculate the bounding box for a given tile in geographic, that is in 
	 * WGS84 longitude/latitude radians.
	 */
	public Mbr geoBoundsForTile(MaplyTileID tileID)
	{
		Mbr mbr = new Mbr(new Point2d(0,0),new Point2d(0,0));
		geoBoundsForTileNative(tileID.x,tileID.y,tileID.level,mbr.ll,mbr.ur);
		return mbr;
	}
	protected native void geoBoundsForTileNative(int x,int y,int level,Point2d ll,Point2d ur);
	
	// Called by the native side when it's time to load a tile
	void loadTile(int x,int y,int level)
	{
		if (!valid)
			return;

//		Log.i("QuadPagingLayer","Load tile: " + level + "(" + x + "," + y + ")");
		MaplyTileID tileID = new MaplyTileID(x,y,level);
		
		LoadedTile thisTile = findLoadedTile(tileID);
		// We were asked the load the tile twice, which is odd.  Punt.
		if (thisTile != null)
			return;
		
		// Keep track of the new tile
		thisTile = new LoadedTile(tileID);
		thisTile.isLoading = true;
		thisTile.didLoad = false;
		thisTile.enable = false;
		addLoadedTile(thisTile);
		
		if (tileID.level >= pagingDelegate.minZoom())
			pagingDelegate.startFetchForTile(this, tileID);
		else
			tileDidLoad(tileID);
	}
	
	// Called by the native side when it's time to unload a tile
	void unloadTile(int x,int y,int level)
	{
		if (!valid)
			return;

//		Log.i("QuadPagingLayer","Unload tile: " + level + "(" + x + "," + y + ")");		
		
		MaplyTileID tileID = new MaplyTileID(x,y,level);
		LoadedTile tile = findLoadedTile(tileID);
		// Guess it wasn't loaded.  Punt.
		if (tile == null)
			return;
		
		removeLoadedTile(tileID);
		tile.clear(maplyControl);
		
		// Check the parent
		if (tileID.level>= pagingDelegate.minZoom() && !singleLevelLoading)
		{
			runTileUpdate(parentTile(tileID));
		}
	}

	// Note: Missing wakeUp
	
	// Tile that we've loaded or are in the processing of loading
	class LoadedTile implements Comparable<LoadedTile>
	{
		public MaplyTileID ident = null;
	    /// Set if this tile is in the process of loading
		public boolean isLoading = false;	    
	    /// Set if this tile successfully loaded
		public boolean didLoad = false;
	    /// Keep track of whether the visible objects are enabled
		public boolean enable = false;
	    /// If set, our children our enabled, but not us.
		public boolean childrenEnable = false;
		
		ArrayList<ComponentObject> compObjs = new ArrayList<ComponentObject>();
		
		LoadedTile(MaplyTileID inIdent)
		{
			ident = inIdent;
		}
		
		public void addToCompObjs(List<ComponentObject> newCompObjs)
		{
			compObjs.addAll(newCompObjs);
		}
				
		// Clear component objects out of maply control
		public void clear(MaplyBaseController maplyControl)
		{
			maplyControl.removeObjects(compObjs,MaplyBaseController.ThreadMode.ThreadCurrent);
			compObjs = new ArrayList<ComponentObject>();
		}
		
		@Override
		public int compareTo(LoadedTile that) 
		{
			return ident.compareTo(that.ident);
		}
	}
	
	// Loaded tiles.  Be sure to use synchronized before messing with them
	Map<MaplyTileID,LoadedTile> loadedTiles = new HashMap<MaplyTileID,LoadedTile>();
	
	// Look for a loaded tile
	LoadedTile findLoadedTile(MaplyTileID tileID)
	{
		synchronized(loadedTiles)
		{
			if (!valid)
				return null;
			return loadedTiles.get(tileID);
		}
	}
	
	// Add a loaded tile.  We're assuming it's not already there
	void addLoadedTile(LoadedTile tile)
	{
		synchronized(loadedTiles)
		{
			if (!valid)
				return;

			loadedTiles.put(tile.ident, tile);
		}
	}
	
	// Remove a loaded tile
	void removeLoadedTile(MaplyTileID tileID)
	{
		synchronized(loadedTiles)
		{
			if (!valid)
				return;

			loadedTiles.remove(tileID);
		}
	}

	/**
	 * When the paging object has loaded a component object, it tells us
	 * about it here.
	 * 
	 * @param compObj The component object handed back by the Maply Controller.
	 * @param tileID The tile we've loaded data for.
	 */
	public void addData(ComponentObject compObj,MaplyTileID tileID)
	{
		if (!valid)
			return;

		ArrayList<ComponentObject> compObjs = new ArrayList<ComponentObject>();
		compObjs.add(compObj);
		addData(compObjs,tileID);
	}

	/**
	 * When the paging object has loaded several component objects, it
	 * tells us about them here.
	 * 
	 * @param compObjs The component objects handed back by the Maply Controller.
	 * @param tileID The tile we've loaded data for.
	 */
	public void addData(List<ComponentObject> compObjs,MaplyTileID tileID)
	{
		if (!valid)
			return;

		LoadedTile tile = findLoadedTile(tileID);
		// We're no longer interested in the tile, so punt
		if (tile == null)
		{
			maplyControl.removeObjects(compObjs,MaplyBaseController.ThreadMode.ThreadCurrent);
			return;
		}
		tile.addToCompObjs(compObjs);
	}
	
	// Calculate the parent tile ID
	MaplyTileID parentTile(MaplyTileID child)
	{
		if (child.level == 0)
			return new MaplyTileID(0,0,0);
		return new MaplyTileID(child.x/2,child.y/2,child.level-1);
	}

	/**
	 * The paging object calls this once it has created all the visual
	 * data associated with the tile.  The paging layer will then
	 * do some calculation and figure out what parents and children to
	 * turn on and off to accommodate the new tile.
	 * <p>
	 * Call this on any thread.
	 * 
	 * @param tileID The tile that successfully loaded.
	 */
	public void tileDidLoad(final MaplyTileID tileID)
	{
		// This needs to be done on the layer's thread
		if (layerThread.getLooper() != Looper.myLooper())
		{
			layerThread.addTask(new Runnable() {
				@Override
				public void run() {
					tileDidLoad(tileID);
				}
			},true);

			return;
		}

		if (!valid)
			return;

		LoadedTile tile = findLoadedTile(tileID);
		if (tile != null)
		{
			tile.isLoading = false;
			tile.didLoad = true;
		}
		if (singleLevelLoading)
		{
			if (tile != null)
				maplyControl.enableObjects(tile.compObjs,MaplyBaseController.ThreadMode.ThreadCurrent);
		} else
			runTileUpdate(parentTile(tileID));
		
		// Call the native code back on the layer thread
		layerThread.addTask(new Runnable()
		{
			@Override public void run()
			{
				nativeTileDidLoad(tileID.x,tileID.y,tileID.level);	
				scheduleEvalStep();
			}
		});
		
//		Log.d("Maply","Tile " + tileID.toString() + " did load");
	}

	/**
	 * When a tile fails to load successfully, the paging object
	 * calls this method to let the paging layer know.
	 * <p>
	 * Call this on any thread.
	 * 
	 * @param tileID The tile that did not load successfully.
	 */
	public void tileFailedToLoad(final MaplyTileID tileID)
	{
		// This needs to be done on the layer's thread
		if (layerThread.getLooper() != Looper.myLooper())
		{
			layerThread.addTask(new Runnable() {
				@Override
				public void run() {
					tileFailedToLoad(tileID);
				}
			});

			return;
		}

		if (!valid)
			return;

		LoadedTile tile = findLoadedTile(tileID);
		if (tile != null)
		{
			tile.clear(maplyControl);
			removeLoadedTile(tileID);
		}
		if (!singleLevelLoading)
			runTileUpdate(parentTile(tileID));
		
		// Call the native code back on the layer thread
		layerThread.addTask(new Runnable()
		{
			@Override public void run()
			{
				nativeTileDidNotLoad(tileID.x,tileID.y,tileID.level);				
				scheduleEvalStep();
			}
		});
		
//		Log.d("Maply","Tile " + tileID.toString() + " failed to load");
	}
	
	// Evaluate whether this tile should be on and all the children
	// Assuming we have the the loadedTiles lock here
	void evaluate(LoadedTile tile,boolean enable,ArrayList<ComponentObject> toEnable,ArrayList<ComponentObject> toDisable)
	{
		if (!valid)
			return;

		LoadedTile children[] = new LoadedTile[4];
		
		int numChild = 0;
		for (int ix=0;ix<2;ix++)
		{
			for (int iy=0;iy<2;iy++)
			{
				MaplyTileID childID = new MaplyTileID(2*tile.ident.x + ix,2*tile.ident.y+iy,tile.ident.level+1);
				LoadedTile found = loadedTiles.get(childID);
				if (found != null && found.didLoad)
					children[numChild++] = found;
			}
		}
				
		if (enable)
		{
			// Enable the children and disable ourselves
			if (numChild == 4)
			{
				for (LoadedTile child: children)
					if (child != null)
						evaluate(child,enable,toEnable,toDisable);
				if (tile.enable)
				{
					tile.enable = false;
					toDisable.addAll(tile.compObjs);
				}
			} else {
				// Disable the children
				for (LoadedTile child: children)
					if (child != null)
						evaluate(child,false,toEnable,toDisable);
				// Enable ourselves
				if (!tile.isLoading && !tile.enable)
				{
					tile.enable = true;
					toEnable.addAll(tile.compObjs);
				}
			}
		} else {
			// Disable children
			for (LoadedTile child: children)
				if (child != null)
					evaluate(child,false,toEnable,toDisable);
			// And ourselves
			if (tile.enable)
			{
				tile.enable = false;
				toDisable.addAll(tile.compObjs);
			}
		}
	}
	
	// When a tile loads or unloads, we need to enable or disable parents and such
	void runTileUpdate(MaplyTileID topTile)
	{
		if (!valid)
			return;

		// Note: Could improve this
		topTile = new MaplyTileID(0,0,0);
		ArrayList<ComponentObject> toEnable = new ArrayList<ComponentObject>();
		ArrayList<ComponentObject> toDisable = new ArrayList<ComponentObject>();
		
		// Ask the tiles to sort themselves out
		synchronized(loadedTiles)
		{
			if (!valid)
				return;

			LoadedTile found = loadedTiles.get(topTile);
			if (found != null)
				evaluate(found,true,toEnable,toDisable);
		}

		if (toEnable.size() > 0)
			maplyControl.enableObjects(toEnable,MaplyBaseController.ThreadMode.ThreadCurrent);
		if (toDisable.size() > 0)
			maplyControl.disableObjects(toDisable,MaplyBaseController.ThreadMode.ThreadCurrent);
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
	
	public native void nativeSetSingleLevelLoading(boolean newVal);
	
	/**
	 * This is the number of pixels we'll want a tile to be before we load it.
	 */
	public native void setImportance(double imp);

	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise(CoordSystem coordSys,PagingInterface pagingDelegate,ChangeSet changes);
	native void dispose();
	private long nativeHandle;

	native void nativeStartLayer(Scene scene,MaplyRenderer renderer,Point2d ll,Point2d ur,int minZoom,int maxZoom);
	native void nativeViewUpdate(ViewState viewState);	
	native boolean nativeEvalStep(ChangeSet changes);
	native boolean nativeRefresh(ChangeSet changes);
	native void nativeTileDidLoad(int x,int y,int level);
	native void nativeTileDidNotLoad(int x,int y,int level);
}
