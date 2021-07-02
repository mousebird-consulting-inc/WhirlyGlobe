/*  MaplyClusterInfo.java
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
 *  Copyright 2011-2021 mousebird consulting
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

/**
 * Information about the group of objects to cluster.
 * <p>
 * This object is passed in when the developer needs to make an image for a group of objects.
 */
public class ClusterInfo {

    /**
     * Number of objects being clustered
     */
    public final int numObjects;

    /**
     * Unique IDs of the objects being clustered
     */
    public final String[] uniqueIDs;

    public ClusterInfo(int numObjects, String[] uniqueIDs) {
        this.numObjects = numObjects;
        this.uniqueIDs = uniqueIDs;
    }
}
