/*
 *  QIFFrameAsset.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/29/19.
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

/**
 * Android version of the frame asset keeps the FetchRequest around.
 * This is always created from the C++ side
 */
class QIFFrameAsset
{
    QIFFrameAsset() { initialise(); }

    // The tile fetch request, if active
    TileFetchRequest request = null;

    // Clear out any active fetches for this frame
    // Called by the C++ side
    public void cancelFetch(QIFBatchOps batchOps) {
        if (request != null && batchOps != null)
            batchOps.addToCancel(request);
        request = null;
    }

    // Called by the C++ side after a request successfully loads
    public void clearRequest() {
        request = null;
    }

    // Update the priority and importance for a tile fetch
    // Probably because the user moved around and a tile takes a different amount of screen space
    // Called by the c++ side
    public void updateFetch(QuadLoaderBase loader, int newPriority,double newImportance)
    {
        if (loader == null || loader.tileFetcher == null)
            return;
        loader.tileFetcher.updateTileFetch(request,newPriority,(float)newImportance);
    }

    // Prepare this frame asset to be deleted
    public void clearFrameAsset(QuadLoaderBase loader,QIFBatchOps batchOps)
    {
        loader.clearFrameAsset(this);
        cancelFetch(batchOps);

        dispose();
    }

    // Return the priority assigned on the C++ side
    public native int getPriority();

    public void finalize()
    {
        dispose();
    }
    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise();
    native void dispose();
    private long nativeHandle;
}
