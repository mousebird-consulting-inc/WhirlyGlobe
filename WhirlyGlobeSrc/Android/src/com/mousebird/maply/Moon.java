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
import java.util.TimeZone;


public class Moon {


    private double illuminatedFraction;
    private double phase;
    private double moonLon, moonLat;

    public Moon(Calendar date) {
        initialise(date.get(Calendar.YEAR), date.get(Calendar.MONTH), date.get(Calendar.DAY_OF_MONTH), date.get(Calendar.HOUR), date.get(Calendar.MINUTE), date.get(Calendar.SECOND));

        //Position of the moon in equatorial
        double [] data;
        data = this.getPositionOfMoon();
        if (data != null) {
            this.moonLon = data[0];
            this.moonLat = data[1];
        }
        //Calcule IlluminatedFraction
        data =  this.getIlluminatedFractionAndPhaseNative();
        if (data != null) {
            this.illuminatedFraction = data[0];
            this.phase = data[1];
        }
    }

    private native double[] getIlluminatedFractionAndPhaseNative();

    private native double[] getPositionOfMoon();

    public double getIlluminatedFraction() {
        return illuminatedFraction;
    }

    public void setIlluminatedFraction(double illuminatedFraction) {
        this.illuminatedFraction = illuminatedFraction;
    }

    public double getPhase() {
        return phase;
    }

    public void setPhase(double phase) {
        this.phase = phase;
    }

    public Point2d asCoordinate(){
        return new Point2d(this.moonLon, this.moonLat);
    }

    public Point3d asPosition() {
        return new Point3d(this.moonLon, this.moonLat, 5.0);
    }

    static {
        nativeInit();
    }

    private static native void nativeInit();
    native void initialise(int year, int month, int day, int hour, int minutes, int second);
    native void dispose();
    private long nativeHandle;
}
