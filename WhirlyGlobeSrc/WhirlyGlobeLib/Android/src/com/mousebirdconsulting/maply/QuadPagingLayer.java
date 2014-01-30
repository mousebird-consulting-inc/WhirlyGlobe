package com.mousebirdconsulting.maply;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import android.os.Handler;
import android.os.Looper;
//import android.util.Log;

public class QuadPagingLayer extends Layer implements LayerThread.ViewWatcherInterface
{
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
		public int minZoom();
		public int maxZoom();
		public void startFetchForTile(QuadPagingLayer layer,MaplyTileID tileID);
	}
	
	MaplyController maplyControl = null;
	CoordSystem coordSys = null;
	PagingInterface pagingDelegate = null;
	
	QuadPagingLayer(MaplyController inMaplyControl,CoordSystem inCoordSys,PagingInterface inDelegate)
	{
		maplyControl = inMaplyControl;
		coordSys = inCoordSys;
		pagingDelegate = inDelegate;
		initialise(coordSys,pagingDelegate);
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
	}
	
	// Called when the layer shuts down.  Won't be run again.
	public void shutdown()
	{
		cancelEvalStep();
		nativeShutdown();
		super.shutdown();
	}

	// View state was updated (filtered through our schedule)
	@Override
	public void viewUpdated(ViewState viewState) 
	{
		nativeViewUpdate(viewState);

		scheduleEvalStep();
	}
	
	// Cancel the current evalStep
	void cancelEvalStep()
	{
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
		evalStepHandle = null;
		evalStepRun = null;
		
		// Note: Check that the renderer is set up and such.
		
		boolean didSomething = nativeEvalStep();
		if (didSomething)
			scheduleEvalStep();
	}
	
	// Caller wants us to reload everything
	public void refresh()
	{
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
		
		boolean doEvalStep = nativeRefresh();
		if (doEvalStep)
			scheduleEvalStep();
	}

	// Calculate the bounding box for a given tile in WGS84 geographic (lon/lat radians)
	Mbr geoBoundsForTile(MaplyTileID tileID)
	{
		Mbr mbr = new Mbr(new Point2d(0,0),new Point2d(0,0));
		geoBoundsForTileNative(tileID.x,tileID.y,tileID.level,mbr.ll,mbr.ur);
		return mbr;
	}
	public native void geoBoundsForTileNative(int x,int y,int level,Point2d ll,Point2d ur);
	
	// Called by the native side when it's time to load a tile
	public void loadTile(int x,int y,int level)
	{
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
	public void unloadTile(int x,int y,int level)
	{
//		Log.i("QuadPagingLayer","Unload tile: " + level + "(" + x + "," + y + ")");		
		
		MaplyTileID tileID = new MaplyTileID(x,y,level);
		LoadedTile tile = findLoadedTile(tileID);
		// Guess it wasn't loaded.  Punt.
		if (tile == null)
			return;
		
		removeLoadedTile(tileID);
		tile.clear(maplyControl);
		
		// Check the parent
		if (tileID.level>= pagingDelegate.minZoom())
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
		public void clear(MaplyController maplyControl)
		{
			maplyControl.removeObjects(compObjs);
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
			return loadedTiles.get(tileID);
		}
	}
	
	// Add a loaded tile.  We're assuming it's not already there
	void addLoadedTile(LoadedTile tile)
	{
		synchronized(loadedTiles)
		{
			loadedTiles.put(tile.ident, tile);
		}
	}
	
	// Remove a loaded tile
	void removeLoadedTile(MaplyTileID tileID)
	{
		synchronized(loadedTiles)
		{
			loadedTiles.remove(tileID);
		}
	}

	// Called by the paging delegate to hand us data for a given tile
	public void addData(List<ComponentObject> compObjs,MaplyTileID tileID)
	{
		LoadedTile tile = findLoadedTile(tileID);
		// We're no longer interested in the tile, so punt
		if (tile == null)
		{
			maplyControl.removeObjects(compObjs);
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
	
	// Called by the paging delegate to indicate a tile loaded
	public void tileDidLoad(final MaplyTileID tileID)
	{
		LoadedTile tile = findLoadedTile(tileID);
		if (tile != null)
		{
			tile.isLoading = false;
			tile.didLoad = true;
		}
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
	}
	
	// Called by the paging delegate to indicate a failed tile load
	public void tileFailedToLoad(final MaplyTileID tileID)
	{
		LoadedTile tile = findLoadedTile(tileID);
		if (tile != null)
		{
			tile.clear(maplyControl);
			removeLoadedTile(tileID);
		}
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
	}
	
	// Evaluate whether this tile should be on and all the children
	// Assuming we have the the loadedTiles lock here
	void evaluate(LoadedTile tile,boolean enable,ArrayList<ComponentObject> toEnable,ArrayList<ComponentObject> toDisable)
	{
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
		// Note: Could improve this
		topTile = new MaplyTileID(0,0,0);
		ArrayList<ComponentObject> toEnable = new ArrayList<ComponentObject>();
		ArrayList<ComponentObject> toDisable = new ArrayList<ComponentObject>();
		
		// Ask the tiles to sort themselves out
		synchronized(loadedTiles)
		{
			LoadedTile found = loadedTiles.get(topTile);
			if (found != null)
				evaluate(found,true,toEnable,toDisable);
		}
		
		maplyControl.enableObjects(toEnable);
		maplyControl.disableObjects(toDisable);
	}
	
	public native void initialise(CoordSystem coordSys,PagingInterface pagingDelegate);
	public native void dispose();
	private long nativeHandle;

	native void nativeStartLayer(MapScene scene,MaplyRenderer renderer,Point2d ll,Point2d ur,int minZoom,int maxZoom);
	native void nativeShutdown();
	native void nativeViewUpdate(ViewState viewState);	
	native boolean nativeEvalStep();
	native boolean nativeRefresh();
	native void nativeTileDidLoad(int x,int y,int level);
	native void nativeTileDidNotLoad(int x,int y,int level);
}
