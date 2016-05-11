/*
 *  Moon.java
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
 *  Copyright 2011-2016 mousebird consulting
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

import java.util.Calendar;


/**
 * Utility for calculating moon position.
 * This is a utility class that figures out where the moon is at a given data
 * and provides the position.
 */
public class Moon {

    private Moon() {
    }

    /**
     * Initialize with a date.
     * <br>
     * The moon position will correspond to that.
     * @param date Must be after 2000.
     */
    public Moon(Calendar date) {
        initialise(date.get(Calendar.YEAR), date.get(Calendar.MONTH), date.get(Calendar.DAY_OF_MONTH), date.get(Calendar.HOUR), date.get(Calendar.MINUTE), date.get(Calendar.SECOND));
    }

    /**
     * @return Illuminated fraction of the moon
     */
    public native double[] getIlluminatedFractionAndPhaseNative();

    /**
     * @return Phase of the moon.
     */
    private native double[] getPositionOfMoon();

    /**
     * @return Location on the globe where the moon would land if it fell straight down.  Ouch.
     */
    public Point2d asCoordinate(){
        double[] pos = this.getPositionOfMoon();
        return new Point2d(pos[0], pos[1]);
    }

    /**
     * @return the location above the globe in lon/lat/distance.  Yay geocentric!
     */
    public Point3d asPosition() {
        double[] pos = this.getPositionOfMoon();
        return new Point3d(pos[0], pos[1], 5.0);
    }

    static {
        nativeInit();
    }

    private static native void nativeInit();
    native void initialise(int year, int month, int day, int hour, int minutes, int second);
    native void dispose();
    private long nativeHandle;
}
