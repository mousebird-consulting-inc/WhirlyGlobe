/*  LayerThread.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2022 mousebird consulting
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
 */

package com.mousebird.maply;

import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.util.Log;

import androidx.annotation.Nullable;

import com.mousebirdconsulting.whirlyglobemaply.BuildConfig;

import org.jetbrains.annotations.NotNull;

import java.io.IOException;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.util.ArrayList;
import java.util.Locale;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.locks.ReentrantLock;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLSurface;

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
	boolean valid = true;
	final public View view;
	final public Scene scene;
	public RenderController renderer = null;
	final ReentrantLock startLock = new ReentrantLock();
	final ArrayList<Layer> layers = new ArrayList<>();
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
	public interface ViewWatcherInterface
	{
		/**
		 * This method is called when the view updates, but no more often then the minTime().
		 * 
		 * @param viewState The new state for the view associated with the MaplyController.
		 */
		void viewUpdated(ViewState viewState);
		
		/**
		 * This minimum time before unique viewUpdated() calls.  Layers can't handle rapid
		 * changes of the view, typically.  So we pick a period, such as 1/10s that we can
		 * handle and specify that here.  The viewUpdated() calls will come no more often than this.
		 */
		float getMinTime();
		
		/**
		 * How long the layer can go without a viewUpdated() call.
		 */
		float getMaxLagTime();

		/**
		 * Some tiles were just removed
		 */
		void tilesUnloaded(@NotNull TileID[] ids);
	}

	// If set, this is a full layer thread.  If not, it just has the context
	protected boolean viewUpdates;
	
	/**
	 * Construct a layer thread.  You should not be doing this directly.  Layer threads are
	 * controlled by the MaplyController.
	 * 
	 * @param name Name of the layer thread for tracking purposes.
	 * @param inView The view we're using.
	 * @param inScene Scene we're putting things into.
	 * @param inViewUpdates If set, we'll watch for view updates.
	 */
	LayerThread(String name,View inView,Scene inScene,boolean inViewUpdates)
	{
		super(name);
		view = inView;
		scene = inScene;
		viewUpdates = inViewUpdates;

		if (viewUpdates) {
			view.addViewWatcher(this);

			// This starts the thread, then we immediately block waiting for the renderer
			// The renderer is created at a later time and handed to us
			try {
				startLock.lockInterruptibly();
			} catch (InterruptedException ignored) {
				return;
			}
			start();
			addTask(() -> {
				if (isShuttingDown) {
					return;
				}

				// Wait for a renderer to be set
				try {
					startLock.lockInterruptibly();
				} catch (InterruptedException ignored) {
					return;
				}
				try {
					startLock.unlock();
				} catch (IllegalMonitorStateException ignored) {
				}

				if (isShuttingDown) {
					return;
				}

				try {
					final EGL10 egl = (EGL10) EGLContext.getEGL();
					if (context != null && surface != null && renderer != null)
						if (!egl.eglMakeCurrent(renderer.display, surface, surface, context)) {
							Log.d("Maply", "Failed to make current context in layer thread.");
							renderer.dumpFailureInfo("LayerThread Setup");
						}
				} catch (Exception e) {
					Log.e("Maply", "Failed to make current context in layer thread.", e);
				}
			});
		} else {
			start();
		}
	}
	
	// Note: Why isn't this in EGL10?
	private static final int EGL_CONTEXT_CLIENT_VERSION = 0x3098;

	private static final int[] glAttribList = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL10.EGL_NONE
	};
	private static final int[] glSurfaceAttrs = {
		EGL10.EGL_WIDTH, 32,
		EGL10.EGL_HEIGHT, 32,
		EGL10.EGL_NONE
	};

	// Setting the renderer kicks off activity
	void setRenderer(RenderController inRenderer)
	{
		if (isShuttingDown || inRenderer == null) {
			return;
		}

		if (inRenderer.display == null) {
			// This happens if you set up a loader before a surface has been created.
			Log.e("Maply", "Renderer not configured");
			return;
		}

		renderer = inRenderer;
		
		final EGL10 egl = (EGL10) EGLContext.getEGL();
		try {
			context = egl.eglCreateContext(renderer.display, renderer.config, renderer.context, glAttribList);
			if (checkGLError(egl, "eglCreateContext") || context == null) {
				return;
			}
			surface = egl.eglCreatePbufferSurface(renderer.display, renderer.config, glSurfaceAttrs);
			if (checkGLError(egl, "eglCreatePbufferSurface") || surface == null) {
				final EGLContext c = context;
				context = null;
				egl.eglDestroyContext(renderer.display, c);
				return;
			}
		} catch (Exception e) {
			Log.e("Maply", "Failed to create EGL context for layer thread: " +
					Integer.toHexString(egl.eglGetError()), e);
			if (context != null) {
				try {
					egl.eglDestroyContext(renderer.display, context);
				} catch (Exception ignored) {
				}
				context = null;
			}
			return;
		}

		if (viewUpdates) {
			// This will release the very first task which sets the right context
			Handler handler = new Handler(Looper.getMainLooper());
			handler.post(() -> {
				try {
					startLock.unlock();
				} catch (IllegalMonitorStateException ignored) {
					return;
				}
				if (!isShuttingDown) {
					viewUpdated(view);
				}
			});
		} else {
			addTask(() -> {
				if (isShuttingDown) {
					return;
				}
				try {
					final EGL10 egl1 = (EGL10) EGLContext.getEGL();
					if (context != null && surface != null && renderer != null) {
						if (!egl1.eglMakeCurrent(renderer.display, surface, surface, context)) {
							Log.e("Maply", "Failed to make EGL context in layer thread: " +
									Integer.toHexString(egl1.eglGetError()));
							renderer.dumpFailureInfo("LayerThread setRenderer");
						}
					} else {
						Log.e("Maply", "Unable to make EGL context in layer thread");
					}
				} catch (Exception e) {
					Log.e("Maply", "Failed to make EGL context in layer thread.", e);
				}
			});
		}
	}

	public static boolean checkGLError(String where) {
		return checkGLError((EGL10)EGLContext.getEGL(), where);
	}
	public static boolean checkGLError(EGL10 egl, String where) {
		final int err = egl.eglGetError();
		if (err != EGL10.EGL_SUCCESS) {
			Log.w("Maply", String.format("OpenGLES error %x%s%s",
					err, (where != null) ? " in " : "", where),
					BuildConfig.DEBUG ? new Throwable() : null);
			return true;
		}
		return false;
	}

	// Used to shut down cleaning without cutting off outstanding work threads
	public boolean isShuttingDown = false;
	private final AtomicInteger numActiveWorkers = new AtomicInteger(0);

	/**
	 * Something is requesting a lock on shutting down while working.
	 *
	 * Prefer <c>startOfWorkWrapper</c> and a try-with-resource construct.
	 *
	 * @return True if the work was started and can proceed, and <c>endOfWork</c> *must* be called.
	 *         False if the work should be canceled, and <c>endOfWork</c> must *not* be called.
	 */
	public boolean startOfWork()
	{
		if (isShuttingDown) {
			return false;
		}

		numActiveWorkers.incrementAndGet();
		return true;
	}

	// End of an external work block.  Safe to shut down.
	public void endOfWork()
	{
		if (numActiveWorkers.decrementAndGet() < 0) {
			// If you see this, it probably means you called `startOfWork` and didn't check the
			// result.  If it returns false, you must not do the work and not call `endOfWork`.
			Log.e("Maply", "Unbalanced endOfWork");
		}
	}

	@Nullable
	public WorkWrapper startOfWorkWrapper() {
		return startOfWork() ? new WorkWrapper() : null;
	}

	public class WorkWrapper implements AutoCloseable {
		public WorkWrapper() {
		}
		public void close() {
			LayerThread.this.endOfWork();
			if (checkWorkTimes) {
				final double e = elapsed();
				if (e > warnWorkTimeLimit) {
					Log.w("Maply", "Work region took " + e +
							((trackWorkStacks && e > stackWorkTimeLimit) ? ": " + getStackTrace() : ""));
				}
			}
		}
		public double elapsed() { return (System.nanoTime() - t0) / 1.0e9; }

		private String getStackTrace() {
			try (StringWriter sw = new StringWriter()) {
				try (PrintWriter pw = new PrintWriter(sw)) {
					throwable.printStackTrace(pw);
					return sw.toString();
				}
			} catch (IOException ex) {
				return "Failed to get stack trace: " + ex.getMessage();
			}
		}

		private final long t0 = System.nanoTime();
		private final Throwable throwable = trackWorkStacks ? new Throwable("for stack") : null;
		private static final boolean checkWorkTimes = false;
		private static final boolean trackWorkStacks = false;
		private static final double warnWorkTimeLimit = 0.5;
		private static final double stackWorkTimeLimit = 1.0;
	}

	private String getStackSummary() {
		try {
			final StackTraceElement[] trace = getStackTrace();
			final StringBuilder sb = new StringBuilder();
			for (int i = 0; i < Math.min(trace.length, 3); ++i) {
				final StackTraceElement e = trace[i];
				sb.append((i > 0) ? " / " : "")
						.append(e.getClassName())
						.append(".")
						.append(e.getMethodName())
						.append(" (")
						.append(e.getFileName())
						.append(":")
						.append(e.getLineNumber());
			}
			return (sb.length() > 0) ? sb.toString() : "(no trace)";
		} catch (Exception ignored) {
			return "(no trace)";
		}
	}

	// Called on the main thread *after* the thread has quit safely
	void shutdown()
	{
//		Log.d("Maply", "LayerThread.shutdown()");

		synchronized (this) {
			if (!isShuttingDown) {
				isShuttingDown = true;
			} else {
				return;
			}
		}

		// We can't run from the thread itself.
		// (maybe we could allow this if `numActiveWorkers` is already zero...)
		final Handler handler = new Handler(getLooper());
		if (handler.getLooper().getThread() == Thread.currentThread()) {
			return;
		}

		// Create with zero permits, must be released before the first acquire can occur
		final Semaphore endLock = new Semaphore(0, true);
		final Semaphore beginLock = new Semaphore(0);

		// If we're shut down before a renderer is set, the the lambda in
		// the constructor will never be unblocked, so wake it up now.
		try {
			if (startLock.isLocked()) {
				startLock.unlock();
			}
		} catch (IllegalMonitorStateException ignored) {
			return;
		}

		// stop accepting new work
		valid = false;

		// Signal the layers, giving them a chance to stop ongoing operations
		synchronized (layers) {
			for (final Layer layer : layers) {
				layer.preShutdown();
			}
		}

		// Wait for anything outstanding to finish before we shut down
		try {
			final long t0 = System.nanoTime();
			int delay = 1;
			while (true) {
				final int count = numActiveWorkers.get();
				if (count <= 0) {
					break;
				}
				if (!isAlive()) {
					Log.w("Maply", "Layer thread with outstanding work was killed");
					break;
				}
				final double elapsed = (System.nanoTime() - t0) / 1.0e9;
				if (elapsed > 2) {
					// If you see this, it means a thread didn't respond to cancellation quickly.
					Log.w("Maply", String.format(Locale.getDefault(),
							"LayerThread timed out waiting for '%s' (%d/%d) after %f s (%s)",
							getName(), getThreadId(), getId(), elapsed, getStackSummary()));
					break;
				}
				// Use exponential backoff to wait for work to end
				//noinspection BusyWait
				sleep(delay);
				if (delay < 64) {
					delay *= 4;
				}
			}
		} catch (InterruptedException ignored) {
			// we took too long
			Log.w("Maply", "LayerThread interrupted while waiting for workers");
		} catch (Exception exp) {
			// Not sure why this would ever happen
			Log.w("Maply", "LayerThread exception while waiting for workers", exp);
		}

		// Run the shutdowns on the thread itself
		final Runnable threadWork = () -> {
			try {
				beginLock.release();

				shutdownLayers();

				// Stop any pending updates
				final Handler trailHandle = trailingHandle;
				final Runnable trailRun = trailingRun;
				if (trailHandle != null && trailRun != null) {
					trailHandle.removeCallbacks(trailRun);
					trailingHandle = null;
					trailingRun = null;
				}

				if (renderer != null && renderer.display != null) {
					final EGL10 egl = (EGL10) EGLContext.getEGL();
					egl.eglMakeCurrent(renderer.display, egl.EGL_NO_SURFACE, egl.EGL_NO_SURFACE, egl.EGL_NO_CONTEXT);
				}
			} catch (Exception ex) {
				Log.w("Maply", "LayerThread shutdown error", ex);
			} finally {
				endLock.release();
			}

			try {
				quit();
			} catch (Exception ex) {
				Log.e("Maply", "Exception from HandlerThread.quit", ex);
			}
		};

		if (!handler.post(threadWork)) {
			// The shutdown work didn't run, so pretend it ran.
			if (renderer != null) {
				if (!layers.isEmpty()) {
					shutdownLayers();
				}
				beginLock.release();
				endLock.release();
			}
		}

		// Block until the queue drains
		if (renderer != null) {
			try {
				// Wait for the shutdown task to start.  If this takes a while, it's not our fault,
				// something else is going on, so we don't want it to count against our timeout.
				if (!beginLock.tryAcquire(2000, TimeUnit.MILLISECONDS)) {
					Log.w("Maply", "LayerThread didn't start stopping within 2s");
				}
				if (!endLock.tryAcquire(500, TimeUnit.MILLISECONDS)) {
					Log.w("Maply", "LayerThread didn't stop within 500ms");

					// If the thread is blocked, wake it up and try again
					try {
						interrupt();
					} catch (SecurityException ignored) {
					}
					if (!endLock.tryAcquire(100, TimeUnit.MILLISECONDS)) {
						Log.w("Maply", "LayerThread didn't stop after interrupt");
					}
				}
			} catch (Exception ex) {
				Log.w("Maply", "LayerThread error waiting for shutdown", ex);
			}
		}

//		Log.d("Maply", "LayerThread.shutdown() done waiting");

		final EGL10 egl = (EGL10) EGLContext.getEGL();
		if (surface != null) {
			if (surface != EGL10.EGL_NO_SURFACE)
				egl.eglDestroySurface(renderer.display, surface);
			surface = null;
		}
		if (context != null) {
			egl.eglDestroyContext(renderer.display,context);
			context = null;
		}

		renderer = null;
		context = null;
		surface = null;
	}
	
	// Add a layer.  These just run in our thread and do their own thing
	public void addLayer(final Layer layer)
	{
		// Do the actual work on the layer thread
		final LayerThread theLayerThread = this;
		addTask(() -> {
			synchronized (layers) {
				layers.add(layer);
			}
			layer.startLayer(theLayerThread);
		});
	}
	
	// Remove a layer.
	public void removeLayer(final Layer layer)
	{
		if (layer == null)
			return;

		addTask(() -> {
			layer.shutdown();
			synchronized (layers) {
				layers.remove(layer);
			}
		});
	}

	private void shutdownLayers()
	{
		final ArrayList<Layer> layersToRemove;
		synchronized (layers) {
			layersToRemove = new ArrayList<>(layers);
			layers.clear();
		}
		for (final Layer layer : layersToRemove) {
			// Don't let an error in one layer's shutdown prevent the others from being called
			try {
				layer.shutdown();
			} catch (Exception ex) {
				Log.w("Maply", "Layer shutdown error", ex);
			}
		}
	}

	private ChangeSet changes = new ChangeSet();
	private Object changeLock = new Object();
	private Handler changeHandler = null;

	/**
	 * Add a set of change requests to the scene.
	 * 
	 * @param newChanges Change requests to process.
	 */
	public void addChanges(ChangeSet newChanges)
	{
		if (changes == null || newChanges == null)
			return;

		if (isShuttingDown)
			return;

		final LayerThread layerThread = this;

		synchronized (changeLock)
		{
			changes.merge(newChanges);
			newChanges.dispose();

			if (changeHandler != null) {
				// already scheduled
				return;
			}

			// Schedule a merge with the scene
			changeHandler = addTask(() -> {
				if (isShuttingDown)
					return;

				// Do a pre-scene flush callback on the layers
				synchronized (layers) {
					for (Layer layer : layers) {
						layer.preSceneFlush(layerThread);
					}
				}

				// Now merge in the changes
				final ChangeSet localChanges;
				synchronized (changeLock) {
					changeHandler = null;
					localChanges = changes;
					changes = new ChangeSet();
				}
				if (localChanges != null) {
					if (scene != null) {
						localChanges.process(renderer, scene);
					}
					localChanges.dispose();
				}
			},true);
		}
	}

	/**
	 * Add a Runnable to our queue.  This will be executed at some point in the future.
	 * 
	 * @param run Runnable to run
	 * @return The Handler if you want to cancel this at some point in the future.
	 */
	public Handler addTask(Runnable run) {
		return addTask(run,false);
	}
	
	/**
	 * Add a Runnable to the queue, but only execute after the given amount of time.
	 * 
	 * @param run Runnable to add to the queue
	 * @param time time Number of milliseconds to wait before running.
	 * @return The Handler if you want to cancel this at some point in the future.
	 */
	public Handler addDelayedTask(Runnable run,long time) {
		return addDelayedTask(run, time, true);
	}

	/**
	 * Add a Runnable to the queue, but only execute after the given amount of time.
	 *
	 * @param run Runnable to add to the queue
	 * @param time time Number of milliseconds to wait before running.
	 * @param unitOfWork If true, the runnable will be bracketed with
	 *                   <c>startOfWork</c> and <c>endOfWork</c> calls
	 * @return The Handler if you want to cancel this at some point in the future.
	 */
	public Handler addDelayedTask(Runnable run,long time,boolean unitOfWork) {
		if (valid && run != null) {
			Handler handler = new Handler(getLooper());
			handler.postDelayed(unitOfWork ? () -> runWorkRunnable(run, true) : run, time);
			return handler;
		}
		return null;
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
	public Handler addTask(Runnable run,boolean wait) {
		return addTask(run,wait,true);
	}

	/**
	 * Add a Runnable to this thread's queue.  It will be executed at some point in the future.
	 *
	 * @param run Runnable to run
	 * @param wait If true we'll always put the Runnable in the queue.  If false we'll see
	 *             if we're already on the layer thread and just execute the runnable instead.
	 * @param unitOfWork If true, the runnable will be bracketed with
	 *                   <c>startOfWork</c> and <c>endOfWork</c> calls
	 * @return Returns a Handler if the work was scheduled or completed,
	 *         null if it could not be scheduled or threw an exception.
	 */
	public Handler addTask(Runnable run,boolean wait,boolean unitOfWork) {
		if (valid && run != null) {
			Handler handler = new Handler(getLooper());
			if (!wait && Looper.myLooper() == handler.getLooper()) {
				if (runWorkRunnable(run, false)) {
					return handler;
				}
			} else if (handler.post(unitOfWork ? () -> runWorkRunnable(run, true) : run)) {
				return handler;
			}
		}
		return null;
	}

	/**
	 * Run the given runnable, wrapping it in a startOfWork/endOfWork region.
	 *
	 * @param work The work to do
	 * @param trap Trap any exceptions
	 */
	private boolean runWorkRunnable(Runnable work, boolean trap) {
		if (!startOfWork()) {
			return false;
		}
		try {
			work.run();
			return true;
		} catch (Exception ex) {
			if (trap) {
				Log.e("Maply", "Exception in LayerThread task", ex);
				return false;
			}
			throw ex;
		} finally {
			endOfWork();
		}
	}

	// Used to track a view watcher
	static class ViewWatcher
	{
		public ViewWatcherInterface watcher;
		public float minTime;
		public float maxLagTime;
		
		public ViewWatcher(ViewWatcherInterface inWatcher)
		{
			watcher = inWatcher;
			minTime = watcher.getMinTime();
			maxLagTime = watcher.getMaxLagTime();
		}
	}
	
	final ArrayList<ViewWatcher> watchers = new ArrayList<>();

	/**
	 * Add an object that we'd like to track changes to the view as
	 * the user moves around.  This is typically called by a Layer
	 * in the startLayer() call.
	 * 
	 * @param watcher Watcher to add to the list.
	 */
	public void addWatcher(final ViewWatcherInterface watcher)
	{
		// Access to the watchers collection is synchronized by always being on the layer thread
		addTask(() -> {
			watchers.add(new ViewWatcher(watcher));

			// Make sure an update gets through the system for this layer
			// Note: Fix this
			final ViewState viewState = currentViewState;
			if (viewState != null) {
				updateWatchers(viewState, System.currentTimeMillis());
			}
		});
	}

	/**
	 * Remove a view watcher that was added previously.  That object will
	 * no longer get view updates.
	 */
	public void removeWatcher(final ViewWatcherInterface watcher)
	{
		// Access to the watchers collection is synchronized by always being on the layer thread
		addTask(() -> {
			for (final ViewWatcher theWatcher : watchers) {
				if (theWatcher.watcher == watcher) {
					watchers.remove(theWatcher);
					break;
				}
			}
		});
	}
	
	ViewState currentViewState = null;
	boolean viewUpdateScheduled = false;
	long viewUpdateLastCalled = 0;
	
	// Update the watchers themselves
	void updateWatchers(final ViewState viewState,long now) {
		// Kick off a view update to the watchers on the layer thread
		final LayerThread theLayerThread = this;
		synchronized (this) {
			if (now > viewUpdateLastCalled) {
				viewUpdateLastCalled = now;
			}
			if (!viewUpdateScheduled) {
				viewUpdateScheduled = true;
				addTask(() -> {
					if (!valid) {
						return;
					}
					synchronized (theLayerThread) {
						viewUpdateScheduled = false;
						currentViewState = viewState;
					}
					for (ViewWatcher watcher : watchers) {
						watcher.watcher.viewUpdated(currentViewState);
					}
				},true);
			}
		}
	}
	
	Handler trailingHandle = null;
	Runnable trailingRun = null;

	// Schedule a lagging update (e.g. not too often, but no less than 100ms)
	void scheduleLateUpdate(long delay)
	{
		synchronized (this)
		{
			if (!valid)
				return;

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
					if (valid) {
						final RenderController theRenderer = renderer;
						if (view != null && theRenderer != null) {
							final ViewState viewState = view.makeViewState(theRenderer);
							if (viewState != null) {
								updateWatchers(viewState, System.currentTimeMillis());
							}
						}
					}

					synchronized (this) {
						trailingHandle = null;
						trailingRun = null;
					}
				}
			};
			
			trailingHandle = addDelayedTask(trailingRun,delay);
		}
	}

	// Note: Hardwired to 1/10 second.  Lame.
	public static long UpdatePeriod = 100;
	
	// Called when the view updates its information
	public void viewUpdated(View view)
	{
		if (view == null || renderer == null)
			return;

		final ViewState viewState = view.makeViewState(renderer);
		if (viewState != null) {
			final long now = System.currentTimeMillis();
			final long timeUntil = now - viewUpdateLastCalled;
			if (timeUntil >= UpdatePeriod) {
				updateWatchers(viewState, now);
			} else {
				scheduleLateUpdate(UpdatePeriod - timeUntil);
			}
		}
	}
}
