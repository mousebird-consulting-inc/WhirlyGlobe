/*
 *  QuadTrackerPointReturn.java
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
 *  Copyright 2011-2015 mousebird consulting
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

public class QuadTrackerPointReturn {

    private double screenU, screenV;
    private MaplyTileID tileID;

    private int padding;

    private double locX, locY;
    private double tileU, tileV;

    public double getScreenU() {
        return screenU;
    }

    public void setScreenU(double screenU) {
        this.screenU = screenU;
    }

    public double getScreenV() {
        return screenV;
    }

    public void setScreenV(double screenV) {
        this.screenV = screenV;
    }

    public MaplyTileID getTileID() {
        return tileID;
    }

    public void setTileID(MaplyTileID tileID) {
        this.tileID = tileID;
    }

    public int getPadding() {
        return padding;
    }

    public void setPadding(int padding) {
        this.padding = padding;
    }

    public double getLocX() {
        return locX;
    }

    public void setLocX(double locX) {
        this.locX = locX;
    }

    public double getLocY() {
        return locY;
    }

    public void setLocY(double locY) {
        this.locY = locY;
    }

    public double getTileU() {
        return tileU;
    }

    public void setTileU(double tileU) {
        this.tileU = tileU;
    }

    public double getTileV() {
        return tileV;
    }

    public void setTileV(double tileV) {
        this.tileV = tileV;
    }
}
