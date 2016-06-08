/*
 *  MaplyClusterGenerator.java
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


/**
 * Fill in this protocol to provide images when individual markers/labels are clustered.
 * <p>
 * This is the protocol for marker/label clustering.  You must fill this in and register the cluster
 */
public interface MaplyClusterGenerator {

    /**
     * Called at the start of clustering.
     * <p>
     * Called right before we start generating clusters.  Do you setup here if need be.
     */
    public void startClusterGroup();

    /**
     * Generate a cluster group for a given collection of markers.
     * <p>
     * Generate an image and size to represent the number of marker/labels we're consolidating.
     * @param clusterInfo
     * @return a cluster group for a given collection of markers.
     */
    public MaplyClusterGroup makeClusterGroup(MaplyClusterInfo clusterInfo);

    /**
     * Called at the end of clustering.
     * <p>
     * If you were doing optimization (for image reuse, say) clean it up here.
     */
    public void endClusterGroup();

    /**
     *
     * @return the cluster number we're covering
     */
    public int clusterNumber();

    /**
     * The size of the cluster that will be created.
     * <p>
     * This is the biggest cluster you're likely to create.  We use it to figure overlaps between clusters.
     * @return The size of the cluster that will be created.
     */
    public Point2d clusterLayoutSize();

    /**
     * Set this if you want cluster to be user selectable.  On by default.
     * @return
     */
    public boolean selectable();

    /**
     * How long to animate markers the join and leave a cluster
     * @return time in seconds
     */
    public double markerAnimationTime();

    /**
     * The shader to use for moving objects around
     * <p>
     * If you're doing animation from point to cluster you need to provide a suitable shader.
     * @return
     */
    public Shader motionShader();
}
