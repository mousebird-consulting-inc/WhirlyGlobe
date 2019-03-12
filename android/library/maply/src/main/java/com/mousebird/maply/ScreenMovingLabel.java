/*
 *  ScreenLabel.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2014 mousebird consulting
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
 * The Moving Screen Label has an end location and duration.
 * It will move from one point to another over that period.
 */
public class ScreenMovingLabel extends ScreenLabel {
    /**
     * The final location in geographic (WGS84) radians.  x is longitude, y is latitude.
     */
    public Point2d endLoc = null;

    /**
     * How long it takes to animate from the start loc to the endLoc
     */
    public double duration = 0.0;
}
