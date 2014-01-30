package com.mousebirdconsulting.maply;

import java.util.ArrayList;
import java.util.concurrent.locks.ReentrantLock;

import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;

// The layer thread runs tasks we don't want on the UI thread
public class LayerThread extends HandlerThread implements MapView.ViewWatcher
{
	MapView view = null;
	MapScene scene = null;
	MaplyRenderer renderer = null;
	ReentrantLock startLock = new ReentrantLock();
	ArrayList<Layer> layers = new ArrayList<Layer>();
	
	/* Objects that want to be called back when the view changes
	 * implement this interface.
	 */
	interface ViewWatcherInterface
	{
		public void viewUpdated(ViewState viewState);
		public float getMinTime();
		public float getMaxLagTime();
	}
	
	public LayerThread(String name,MapView inView,MapScene inScene) 
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
			}
		});
	}

	// Setting the renderer kicks off activity
	public void setRenderer(MaplyRenderer inRenderer)
	{
		renderer = inRenderer;
		// We're probably being called on the rendering thread here
		Handler handler = new Handler(Looper.getMainLooper());
		handler.post(new Runnable()
		{
			@Override
			public void run()
			{
				startLock.unlock();				
			}
		});
	}
	
	// Add a layer.  These just run in our thread and do their own thing
	void addLayer(final Layer layer)
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
	
	// Note: Need a removeLayer()
	
	Handler addTask(Runnable run)
	{
		return addTask(run,false);
	}

	// Add a task, which will get executed on this thread at some point
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

	// Add an object that once periodic view change updates
	public void addWatcher(final ViewWatcherInterface watcher)
	{
		// Let's do this on the layer thread.  Because.
		addTask(new Runnable()
		{
			@Override
			public void run()
			{
				watchers.add(new ViewWatcher(watcher));				
			}
		});
	}
	
	// Remove an object that no longer wants periodic view change updates
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
	
	// Called when the view updates its information
	public void viewUpdated(MapView view)
	{
		final ViewState viewState = new ViewState(view,renderer);

		long now = System.currentTimeMillis();

		if (now - viewUpdateLastCalled > 100)
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
					});
				}
			}
		}
	}
}
