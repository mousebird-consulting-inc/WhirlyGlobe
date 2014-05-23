package com.mousebird.maply;

import android.os.Handler;

import com.mousebird.maply.LayerThread.ViewWatcher;

/**
 * The layout layer runs every so often to layout text and marker objects
 * on the screen.
 * 
 * @author sjg
 *
 */
class LayoutLayer extends Layer implements LayerThread.ViewWatcherInterface
{
	MaplyController maplyControl = null;
	LayoutManager layoutManager = null;

	LayoutLayer(MaplyController inMaplyControl,LayoutManager inLayoutManager)
	{
		maplyControl = inMaplyControl;
		layoutManager = inLayoutManager;
	}
	
	public void startLayer(LayerThread inLayerThread)
	{
		super.startLayer(inLayerThread);

		scheduleUpdate();
		maplyControl.layerThread.addWatcher(this);
	}
	
	public void shutdown()
	{
		cancelUpdate();
	}
	
	ViewState viewState = null;

	// Called when the view state changes
	@Override
	public void viewUpdated(ViewState newViewState) 
	{		
		// This pushes back the update, which is what we want
		// We'd prefer to update 0.2s after the user stops moving
		if (viewState == null || viewState.isEqual(newViewState))
		{			
			viewState = newViewState;
			cancelUpdate();
			scheduleUpdate();
		}
	}
	
	// Make a Runnable that repeatedly runs itself
	Runnable makeRepeatingTask()
	{
		return new Runnable()
		{
			@Override
			public void run()
			{
				runUpdate();
				scheduleUpdate();
			}
		};
	}
	
	// Set if we've got an update in the queue
	Runnable updateRun = null;
	Handler updateHandle = null;
	
	// Schedule an update if there isn't one already
	void scheduleUpdate()
	{
		synchronized(this)
		{
			if (updateHandle == null)
			{
				updateRun = makeRepeatingTask();
				updateHandle = maplyControl.layerThread.addDelayedTask(updateRun,0.2f);
			}
		}
	}
	
	// Cancel an update if there's one scheduled
	void cancelUpdate()
	{
		synchronized(this)
		{
			if (updateHandle != null)
			{
				updateHandle.removeCallbacks(updateRun);
				updateHandle = null;
				updateRun = null;
			}
		}
	}
	
	// Actually run the layout update
	void runUpdate()
	{
		updateHandle = null;
		updateRun = null;
		// Note: Should wait until the user stops moving
		ChangeSet changes = new ChangeSet();
		layoutManager.updateLayout(viewState, changes);
		maplyControl.mapScene.addChanges(changes);		
	}
	
	@Override
	public float getMinTime() 
	{
		// Update every 1/10s
		return 0.2f;
	}

	@Override
	public float getMaxLagTime() 
	{
		// Want an update no less often than this
		// Note: What?
		return 4.0f;
	}
}
