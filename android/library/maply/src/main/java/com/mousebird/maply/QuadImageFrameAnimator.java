/*
 *  QuadImageFrameAnimator.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/8/19.
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
import java.util.Calendar;

/**
 * Quad Image FrameAnimation runs through the frames in a Quad Image Frame loader over time.
 * <br>
 * Set this up with a QuadImageFrameLoader and it'll run through the available frames from start
 * to finish.  At the end it will snap back to the beginning.
 */
public class QuadImageFrameAnimator implements ActiveObject
{
    WeakReference<BaseController> control = null;
    WeakReference<QuadImageFrameLoader> loader = null;
    int numFrames = 0;

    protected double startTime = 0.0;

    /**
     * Animate the current image on the given image frame loader.
     */
    public QuadImageFrameAnimator(QuadImageFrameLoader inLoader,BaseController inControl)
    {
        control = new WeakReference<BaseController>(inControl);
        loader = new WeakReference<QuadImageFrameLoader>(inLoader);
        startTime = Calendar.getInstance().getTimeInMillis() / 1000.0;
        numFrames = inLoader.getNumFrames();

        control.get().addActiveObject(this);
    }

    /**
     * How long to animate from start to finish.
     */
    public double period = 10.0;

    /**
     * How long to pause at the end of the sequence before starting back
     */
    public double pauseLength = 0.0;

    /**
     * Remove the animator and stop animating
     */
    public void shutdown() {
        if (control.get() == null)
            return;
        control.get().removeActiveObject(this);
    }

    /** ----- Active Object methods ------- */

    // Have to do the position update in the setCurrentImage so we're
    // not messing with the rendering loop
    public boolean hasChanges()
    {
        if (loader.get() == null)
            return false;

        double now = Calendar.getInstance().getTimeInMillis() / 1000.0;
        double totalPeriod = period + pauseLength;
        double when = (now-startTime) % totalPeriod;
        // Snap it to the end for a while
        if (when >= period)
            loader.get().setCurrentImage(numFrames-1);
        else {
            double where = when/period * (numFrames-1);
            loader.get().setCurrentImage(where);
        }

        return false;
    }

    // Don't need to do anything here
    public void activeUpdate()
    {

    }
}
