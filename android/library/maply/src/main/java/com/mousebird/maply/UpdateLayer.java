/*
 *  UpdateLayer.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 9/15/20.
 *  Copyright 2011-2020 mousebird consulting
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

import java.lang.ref.WeakReference;

/**
 * This layer will call a delegate as the user moves around, but constrained to distance and time.
 *
 * This layer is responsible for calling a delegate you provide as the user moves their viewpoint around.
 * You'll be called if they move from than a certain amount, but not more often than the minimum time.
 */
public class UpdateLayer extends Layer implements LayerThread.ViewWatcherInterface
{
    /**
     * Fill in the delegate for callbacks
     */
    public interface Delegate {
        /**
         Called when the MaplyUpdateLayer is initialized.

         This is called after things are set up.  You'll be on the layer thread here.
         */
        void start(UpdateLayer layer);

        /**
         Called when the viewer moves.

         You'll be called on the layer thread when the viewer moves more than your moveDist, subject to calls no more frequent than the minTime.
         */
        void viewerMovedTo(UpdateLayer layer, ViewState viewState);

        /**
         Called when the update layer is shutting down.

         Clean up your own data here.
         */
        void teardown(UpdateLayer layer);
    }
    public Delegate delegate = null;

    WeakReference<BaseController> maplyControl = null;

    public UpdateLayer(float moveDist, float minTime, Delegate delegate, BaseController inMaplyControl) {
        maplyControl = new WeakReference<BaseController>(inMaplyControl);

        this.delegate = delegate;
        this.moveDist = moveDist;
        this.minTime = minTime;
    }

    public void startLayer(LayerThread inLayerThread) {
        super.startLayer(inLayerThread);

        delegate.start(this);

        scheduleUpdate();
        if (maplyControl != null) {
            LayerThread layerThread = maplyControl.get().getLayerThread();
            if (layerThread != null)
                maplyControl.get().getLayerThread().addWatcher(this);
        }
    }

    public void shutdown()
    {
        cancelUpdate();
        delegate.teardown(this);
        delegate = null;
    }

    ViewState viewState = null;

    public float moveDist = 0.0f;
    public float minTime = 0.2f;
    public float maxLag = 0.0f;

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
                updateRun = new Runnable()
                {
                    @Override
                    public void run()
                    {
                        runUpdate();
                    }
                };
                if (maplyControl != null && maplyControl.get().getLayerThread() != null) {
                    updateHandle = maplyControl.get().getLayerThread().addDelayedTask(updateRun, 200);
                }
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

        if (delegate != null)
            delegate.viewerMovedTo(this, viewState);
    }

    @Override
    public void viewUpdated(ViewState newViewState) {
        // Check how far we've moved
        double dist = 0.0;
        if (viewState != null) {
            Point3d eye0 = newViewState.getEyePos();
            Point3d eye1 = viewState.getEyePos();

            dist = eye0.subtract(eye1).length();
        }

        if (dist > moveDist) {
            cancelUpdate();
            scheduleUpdate();
        }
        viewState = newViewState;
    }

    @Override
    public float getMinTime() {
        return minTime;
    }

    @Override
    public float getMaxLagTime() {
        return maxLag;
    }
}
