/*
 *  QuadSamplingLayer.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/28/19.
 *  Copyright 2011-2019 mousebird consulting
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

import java.lang.ref.WeakReference;
import java.util.ArrayList;

/**
 * The Quad Sampling Layer runs a quad tree which determines what
 *  tiles to load.  We hook up other things to this to actually do
 *  the loading.
 */
class QuadSamplingLayer extends Layer implements LayerThread.ViewWatcherInterface
{
    WeakReference<BaseController> control;
    SamplingParams params;

    QuadSamplingLayer(BaseController inControl,SamplingParams inParams) {
        control = new WeakReference<BaseController>(inControl);
        params = inParams;

        initialise(params);
    }

    // Used to hook
    interface ClientInterface {
        void samplingLayerConnect(QuadSamplingLayer layer);
        void samplingLayerDisconnect(QuadSamplingLayer layer);
    }

    /**
     * Add a client to this sampling layer.
     * It'll do its hooking up on the C++ side.
     */
    public void addClient(ClientInterface user)
    {
        user.samplingLayerConnect(this);
        clients.add(user);

        // TODO: Push a client notification if the client is added after we're running
    }

    ArrayList<ClientInterface> clients = new ArrayList<ClientInterface>();

    /**
     * Client will stop getting updates from this sampling layer.
     */
    public void removeClient(ClientInterface user)
    {
        if (!clients.contains(user))
            return;
        user.samplingLayerDisconnect(this);
        clients.remove(user);
    }

    /** --- Layer methods --- */

    public void startLayer(LayerThread inLayerThread)
    {
        layerThread = inLayerThread;

        layerThread.addWatcher(this);

        startNative(params,control.get().scene,control.get().renderControl);
    }

    public void preSceneFlush(LayerThread layerThread)
    {
        ChangeSet changes = new ChangeSet();
        preSceneFlushNative(changes);
        layerThread.addChanges(changes);
    }

    public void shutdown()
    {
        ChangeSet changes = new ChangeSet();
        shutdownNative(changes);
        layerThread.addChanges(changes);
    }

    /** --- View Updated Methods --- **/
    public void viewUpdated(ViewState viewState)
    {
        ChangeSet changes = new ChangeSet();
        if (viewUpdatedNative(viewState,changes)) {
            // TODO: Schedule a delayed update because the controller is sitting on some later changes
        }
        layerThread.addChanges(changes);
    }

    // Called noe more often than 1/10 of a second
    public float getMinTime()
    {
        return 0.1f;
    }

    // Lags no more than 4s (if a user is continuously moving around, basically)
    public float getMaxLagTime()
    {
        return 4.0f;
    }

    // Number of clients attached to this sampling layer
    native int getNumClients();

    private native boolean viewUpdatedNative(ViewState viewState,ChangeSet changes);
    private native void startNative(SamplingParams params,Scene scene,RenderController render);
    private native void preSceneFlushNative(ChangeSet changes);
    private native void shutdownNative(ChangeSet changes);

    public void finalize()
    {
        dispose();
    }
    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise(SamplingParams params);
    native void dispose();
    private long nativeHandle;
}
