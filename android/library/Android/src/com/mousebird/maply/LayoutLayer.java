/*
 *  LayoutLayer.java
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

import android.os.Handler;

/**
 * The layout layer runs every so often to layout text and marker objects
 * on the screen.  You don't need to create one of these, MaplyController does that.
 * 
 */
class LayoutLayer extends Layer implements LayerThread.ViewWatcherInterface
{
	MaplyBaseController maplyControl = null;
	LayoutManager layoutManager = null;

	LayoutLayer(MaplyBaseController inMaplyControl,LayoutManager inLayoutManager)
	{
		maplyControl = inMaplyControl;
		layoutManager = inLayoutManager;
	}
	
	public void startLayer(LayerThread inLayerThread)
	{
		super.startLayer(inLayerThread);

		scheduleUpdate();
		maplyControl.getLayerThread().addWatcher(this);
	}
	
	public void shutdown()
	{
		cancelUpdate();

		layoutManager.clearClusterGenerators();
	}
	
	ViewState viewState = null;
	ViewState lastViewState = null;

	// Called when the view state changes
	@Override
	public void viewUpdated(ViewState newViewState) 
	{		
		// This pushes back the update, which is what we want
		// We'd prefer to update 0.2s after the user stops moving
		// Note: Should do a deeper compare on the view states
		if (viewState == null || viewState != newViewState)
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
				updateHandle = maplyControl.getLayerThread().addDelayedTask(updateRun,200);
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

		// Note: Should do a deeper compare on the view states
		if (layoutManager.hasChanges() || viewState != lastViewState) {
			// Note: Should wait until the user stops moving
			ChangeSet changes = new ChangeSet();
			layoutManager.updateLayout(viewState, changes);
			maplyControl.scene.addChanges(changes);

			lastViewState = viewState;
		}
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

	public void addClusterGenerator(ClusterGenerator generator) {
		synchronized (this) {
			Point2d clusterSize = generator.clusterLayoutSize();
			this.layoutManager.addClusterGenerator(generator, generator.clusterNumber(),generator.selectable(),clusterSize.getX(),clusterSize.getY());
		}
	}
}
