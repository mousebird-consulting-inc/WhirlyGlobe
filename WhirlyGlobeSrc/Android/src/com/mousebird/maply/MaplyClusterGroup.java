/*
 *  MaplyClusterGroup.java
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
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

import android.graphics.Bitmap;


/**
 * Visual representation for a group of markers.
 */
public class MaplyClusterGroup {

    /**
     * The image to use for the group
     */
    public Bitmap image;

    public Point2d getSize() {
        return size;
    }

    /**
     * Screen size to use for the resulting marker
     */
    public Point2d size = new Point2d();

    public MaplyClusterGroup(Bitmap image, Point2d size) {
        this.image = image;
        this.size = size;
    }

    public MaplyClusterGroup() {
        this.image = null;
        this.size = new Point2d();
    }
}
