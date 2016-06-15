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


import java.util.ArrayList;

/**
 * Fill in this protocol to provide images when individual markers/labels are clustered.
 * <p>
 * This is the protocol for marker/label clustering.  You must fill this in and register the cluster
 */
public class ClusterGenerator
{
    public MaplyBaseController baseController = null;
    ArrayList<MaplyTexture> oldTextures,currentTextures;

    /**
     * Called at the start of clustering.
     * <p>
     * Called right before we start generating clusters.  Do you setup here if need be.
     */
    public void startClusterGroup()
    {
        if (oldTextures != null) {
            baseController.removeTextures(oldTextures, MaplyBaseController.ThreadMode.ThreadCurrent);
            oldTextures = null;
        }

        oldTextures = currentTextures;
        currentTextures = new ArrayList<MaplyTexture>();
    }

    /**
     * Generate a cluster group for a given collection of markers.
     * <p>
     * Generate an image and size to represent the number of marker/labels we're consolidating.
     * @param clusterInfo
     * @return a cluster group for a given collection of markers.
     */
    public ClusterGroup makeClusterGroup(ClusterInfo clusterInfo)
    {
        return null;
    }

    // The C++ code calls this to get a Bitmap then we call makeClusterGroup
    public long makeClusterGroupJNI(int num)
    {
        ClusterInfo clusterInfo = new ClusterInfo(num);
        ClusterGroup newGroup = makeClusterGroup(clusterInfo);

        currentTextures.add(newGroup.tex);

        return newGroup.tex.texID;
    }

    /**
     * Called at the end of clustering.
     * <p>
     * If you were doing optimization (for image reuse, say) clean it up here.
     */
    public void endClusterGroup()
    {
    }

    /**
     * The Cluster number is referenced by screen markers.  We group all the markers that
     * share a cluster number together.
     * @return the cluster number we're covering
     */
    public int clusterNumber()
    {
        return 0;
    }

    /**
     * The size of the cluster that will be created.
     * <p>
     * This is the biggest cluster you're likely to create.  We use it to figure overlaps between clusters.
     * @return The size of the cluster that will be created.
     */
    public Point2d clusterLayoutSize()
    {
        return new Point2d(32.0,32.0);
    }

    /**
     * Set this if you want cluster to be user selectable.  On by default.
     * @return
     */
    public boolean selectable()
    {
        return true;
    }

    /**
     * How long to animate markers the join and leave a cluster
     * @return time in seconds
     */
    public double markerAnimationTime()
    {
        return 1.0;
    }

    /**
     * The shader to use for moving objects around
     * <p>
     * If you're doing animation from point to cluster you need to provide a suitable shader.
     * @return
     */
//    public Shader motionShader()
//    {
//    }

}
