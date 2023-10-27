/*  QIFBatchOps.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/28/19.
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
import androidx.annotation.Nullable;
import org.jetbrains.annotations.NotNull;
import java.util.ArrayList;

/**
 * For QuadSamplingLayer and associated logic we handle the actual
 * data fetching on the Java side.  This object keeps track of
 * fetches that need to be started and cancelled.
 */
class QIFBatchOps
{
    ArrayList<TileFetchRequest> toCancel = new ArrayList<>();
    ArrayList<TileFetchRequest> toStart = new ArrayList<>();

    @SuppressWarnings("unused")		// Referenced by JNI
    protected QIFBatchOps() { }

    /**
     * Add a fetch request to cancel.
     */
    void addToCancel(@NotNull TileFetchRequest request) {
        toCancel.add(request);
    }

    /**
     * Add a fetch request to start.
     */
    void addToStart(@NotNull TileFetchRequest request) {
        toStart.add(request);
    }

    /**
     * Process the outstanding starts and cancels we gathered.
     */
    void process(@Nullable TileFetcher fetcher) {
        // Just run the logic ourselves
        if (fetcher == null) {
            // Don't do anything for cancel
            for (TileFetchRequest request: toStart) {
                request.callback.success(request, null);
            }
            return;
        }

        if (!toCancel.isEmpty()) {
            fetcher.cancelTileFetches(toCancel.toArray(new TileFetchRequest[0]));
            toCancel = null;
        }
        if (!toStart.isEmpty()) {
            fetcher.startTileFetches(toStart.toArray(new TileFetchRequest[0]));
            toStart = null;
        }

        // These are single use, so clear out the C++ side
        dispose();
    }

    // Get the tiles to be removed from the native object
    public native @Nullable TileID[] getDeletes();

    public void finalize() {
        dispose();
    }
    native void dispose();

    static {
        nativeInit();
    }
    private static native void nativeInit();

    @SuppressWarnings("unused")		// Referenced by JNI
    native void initialise();

    @SuppressWarnings("unused")		// Referenced by JNI
    private long nativeHandle;
}
