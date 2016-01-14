/*
 *  LayerThread.java
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
import java.util.concurrent.Semaphore;
import java.util.concurrent.locks.ReentrantLock;

import javax.microedition.khronos.egl.*;

import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.util.Log;

/**
 * The layer thread runs tasks we want off the UI thread, but still need
 * some control over.  The layer thread has some of its own state, including
 * ChangeSets and similar objects.
 * <p>
 * When you call addTask on the MaplyController, the Runnable probably ends up here.
 * 
 * @author sjg
 *
 */
public class LayerThread extends HandlerThread implements View.ViewWatcher
{
	View view = null;
	Scene scene = null;
	MaplyRenderer renderer = null;
	ReentrantLock startLock = new ReentrantLock();
	ArrayList<Layer> layers = new ArrayList<Layer>();
	// A unique context for this thread
	EGLContext context = null;
	EGLSurface surface = null;

	/**
	 * Objects that want to be called when the view updates its position
	 * fill out this interface and register with the layer thread.  These
	 * are probably layers.
	 * 
	 * @author sjg
	 *
	 */
	interface ViewWatcherInterface
	{
		/**
		 * This method is called when the view updates, but no more often then the minTime().
		 * 
		 * @param viewState The new state for the view associated with the MaplyController.
		 */
		public void viewUpdated(ViewState viewState);
		
		/**
		 * This minimum time before unique viewUpdated() calls.  Layers can't handle rapid
		 * changes of the view, typically.  So we pick a period, such as 1/10s that we can
		 * handle and specify that here.  The viewUpdated() calls will come no more often than this.
		 */
		public float getMinTime();
		
		/**
		 * How long the layer can go without a viewUpdated() call.
		 */
		public float getMaxLagTime();
	}
	
	/**
	 * Construct a layer thread.  You should not be doing this directly.  Layer threads are
	 * controlled by the MaplyController.
	 * 
	 * @param name Name of the layer thread for tracking purposes.
	 * @param inView The view we're using.
	 * @param inScene Scene we're putting things into.
	 */
	LayerThread(String name,View inView,Scene inScene) 
	{
		super(name);
		view = inView;
		scene = inScene;

		view.addViewWatcher(this);
		
		// This starts the thread, then we immediately block waiting for the renderer
		// The renderer is created at a later time and handed to us
		startLock.lock();
		start();
		addTask(new Runnable()
		{
			@Override
			public void run()
			{
				startLock.lock();
				startLock.unlock();

				try
				{
					EGL10 egl = (EGL10) EGLContext.getEGL();
					if (!egl.eglMakeCurrent(renderer.display, surface, surface, context))
					    Log.i("Maply","Failed to make current context in layer thread.");
				}
				catch (Exception e)
				{
					Log.i("Maply","Failed to make current context in layer thread.");
				}
			}
		});
	}
	
	// Note: Why isn't this in EGL10?
	private static final int EGL_CONTEXT_CLIENT_VERSION = 0x3098;
	
	// Setting the renderer kicks off activity
	void setRenderer(MaplyRenderer inRenderer)
	{
		renderer = inRenderer;
		
		EGL10 egl = (EGL10) EGLContext.getEGL();
		int[] attrib_list = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL10.EGL_NONE };
		context = egl.eglCreateContext(renderer.display,renderer.config,renderer.context, attrib_list);
		int[] surface_attrs =
			{
			    EGL10.EGL_WIDTH, 32,
			    EGL10.EGL_HEIGHT, 32,
//			    EGL10.EGL_COLORSPACE, GL10.GL_RGB,
//			    EGL10.EGL_TEXTURE_FORMAT, EGL_TEXTURE_RGB,
//			    EGL10.EGL_TEXTURE_TARGET, EGL_TEXTURE_2D,
//			    EGL10.EGL_LARGEST_PBUFFER, GL10.GL_TRUE,
			    EGL10.EGL_NONE
			};
		surface = egl.eglCreatePbufferSurface(renderer.display, renderer.config, surface_attrs);
		
		// This will release the very first task which sets the right context
		Handler handler = new Handler(Looper.getMainLooper());
		handler.post(new Runnable()
		{
			@Override
			public void run()
			{
				startLock.unlock();				
				viewUpdated(view);
			}
		});
	}
	
	// Called on the main thread *after* the thread has quit safely
	void shutdown()
	{
		// Run the shutdowns on the thread itself
		addTask(new Runnable()
		{
			@Override
			public void run()
			{
				for (final Layer layer : layers)
				{
					layer.shutdown();
				}
			}
		},true);

		final Semaphore endLock = new Semaphore(0,true);
		addTask(new Runnable()
		{
			@Override
			public void run()
			{
				try
				{
					quit();
				}
				catch (Exception e)
				{

				}
				endLock.release();
			}
		},true);

		// Block until the queue drains
		try {
			endLock.acquire();
		}
		catch( Exception e)
		{
		}

		// Note: Is this blocking?
		layers = null;
		view = null;
		scene = null;
		renderer = null;
		context = null;
		surface = null;
	}
	
	// Add a layer.  These just run in our thread and do their own thing
	public void addLayer(final Layer layer)
	{
		// Do the actual work on the layer thread
		final LayerThread theLayerThread = this;
		addTask(new Runnable()
		{
			@Override
			public void run()
			{
				layers.add(layer);
				layer.startLayer(theLayerThread);
			}
		});
	}
	
	// Remove a layer.  Do this on the layer thread
	public void removeLayer(final Layer layer)
	{
		addTask(new Runnable()
		{
			@Override
			public void run()
			{
				layer.shutdown();
				layers.remove(layer);
			}
		});
	}
	
	// Note: Need a removeLayer()
	
	ChangeSet changes = new ChangeSet();
	Handler changeHandler = null;

	/**
	 * Add a set of change requests to the scene.
	 * 
	 * @param changes Change requests to process.
	 */
	void addChanges(ChangeSet newChanges)
	{
		if (changes == null || newChanges == null)
			return;

		synchronized(changes)
		{
			changes.merge(newChanges);
			// Schedule a merge with the scene
			if (changeHandler == null)
			{
				changeHandler = addTask(new Runnable()
				{
					@Override
					public void run()
					{
						synchronized (changes) {
							changeHandler = null;
							changes.process(scene);
						}
					}
				},true);
			}
		}
	}

	/**
	 * Add a Runnable to our queue.  This will be executed at some point in the future.
	 * 
	 * @param run Runnable to run
	 * @return The Handler if you want to cancel this at some point in the future.
	 */
	Handler addTask(Runnable run)
	{
		return addTask(run,false);
	}
	
	/**
	 * Add a Runnable to the queue, but only execute after the given amount of time.
	 * 
	 * @param run Runnable to add to the queue
	 * @time time Number of milliseconds to wait before running.
	 * @return The Handler if you want to cancel this at some point in the future.
	 */
	Handler addDelayedTask(Runnable run,long time)
	{
		Handler handler = new Handler(getLooper());
		handler.postDelayed(run, time);
		return handler;
	}

	/**
	 * Add a Runnable to this thread's queue.  It will be executed at some point in the future.
	 * 
	 * @param run Runnable to run
	 * @param wait If true we'll always put the Runnable in the queue.  If false we'll see
	 * if we're already on the layer thread and just execute the runnable instead.
	 * 
	 * @return Returns a Handler if you want to cancel the task later.  Returns null if
	 * we were on the layer thread and no Handler was needed.
	 */
	Handler addTask(Runnable run,boolean wait)
	{
		if (!wait && Looper.myLooper() == getLooper())
			run.run();
		else {
			Handler handler = new Handler(getLooper());
			handler.post(run);
			return handler;		
		}
		
		return null;
	}

	// Used to track a view watcher
	class ViewWatcher
	{
		public ViewWatcherInterface watcher = null;
		public float minTime = 0.1f;
		public float maxLagTime = 10.f;
		
		ViewWatcher(ViewWatcherInterface inWatcher)
		{
			watcher = inWatcher;
			minTime = watcher.getMinTime();
			maxLagTime = watcher.getMaxLagTime();
		}
	}
	
	ArrayList<ViewWatcher> watchers = new ArrayList<ViewWatcher>();

	/**
	 * Add an object that we'd like to track changes to the view as
	 * the user moves around.  This is typically called by a Layer
	 * in the startLayer() call.
	 * 
	 * @param watcher Watcher to add to the list.
	 */
	public void addWatcher(final ViewWatcherInterface watcher)
	{
		// Let's do this on the layer thread.  Because.
		addTask(new Runnable()
		{
			@Override
			public void run()
			{
				watchers.add(new ViewWatcher(watcher));	
				
				// Make sure an update gets through the system for this layer
				// Note: Fix this
				addTask(new Runnable()
				{
					@Override
					public void run()
					{
						// Make sure the watcher gets a callback
						if (currentViewState != null)
							updateWatchers(currentViewState,System.currentTimeMillis());						
					}
				},true);
			}
		});
	}

	/**
	 * Remove a view watcher that was added previously.  That object will
	 * no longer get view updates.
	 */
	public void removeWatcher(final ViewWatcherInterface watcher)
	{
		// Let's do this on the layer thread.  Because.
		addTask(new Runnable()
		{
			@Override
			public void run()
			{
				ViewWatcher found = null;
				for (ViewWatcher theWatcher: watchers)
					if (theWatcher.watcher == watcher)
					{
						found = theWatcher;
						break;
					}
				if (found != null)
					watchers.remove(found);
			}
		});
	}
	
	ViewState currentViewState = null;
	boolean viewUpdateScheduled = false;
	long viewUpdateLastCalled = 0;
	
	// Update the watchers themselves
	void updateWatchers(final ViewState viewState,long now)
	{
		viewUpdateLastCalled = now;
		// Kick off a view update to the watchers on the layer thread
		final LayerThread theLayerThread = this;
		synchronized(this)
		{
			if (!viewUpdateScheduled)
			{
				viewUpdateScheduled = true;
				addTask(new Runnable()
				{
					@Override
					public void run()
					{
						synchronized(theLayerThread)
						{
							viewUpdateScheduled = false;
						}
						currentViewState = viewState;
						for (ViewWatcher watcher : watchers)
						{
							watcher.watcher.viewUpdated(currentViewState);
						}
					}
				},true);
			}
		}		
	}
	
	Handler trailingHandle = null;
	Runnable trailingRun = null;
	// Schedule a lagging update (e.g. not too often, but no less than 100ms
	void scheduleLateUpdate(long delay)
	{
		synchronized(this)
		{
			if (trailingHandle != null)
			{
				trailingHandle.removeCallbacks(trailingRun);
				trailingHandle = null;
				trailingRun = null;
			}

			trailingRun = new Runnable()
			{
				@Override
				public void run()
				{
					final ViewState viewState = view.makeViewState(renderer);
					long now = System.currentTimeMillis();
					updateWatchers(viewState,now);
					
					trailingHandle = null;
					trailingRun = null;
				}				
			};
			
			trailingHandle = addDelayedTask(trailingRun,delay);
		}
	}
	
	static long UpdatePeriod = 100;
	
	// Called when the view updates its information
	public void viewUpdated(View view)
	{
		if (view == null || renderer == null)
			return;

		final ViewState viewState = view.makeViewState(renderer);

		long now = System.currentTimeMillis();

		// Note: Hardwired to 1/10 second.  Lame.
		long timeUntil = now - viewUpdateLastCalled;
		if (timeUntil > UpdatePeriod)
		{
			updateWatchers(viewState,now);
		} else {
			// Note: Technically, should do the difference here
			scheduleLateUpdate(UpdatePeriod);
		}
	}
}
