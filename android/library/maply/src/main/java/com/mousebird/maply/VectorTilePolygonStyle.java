/*  MaplyVectorTilePolygonStyle.java
 *  WhirlyGlobeLib
 *
 *  Created by Ranen Ghosh on 3/27/17.
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

import java.util.ArrayList;

/**
 * The VectorTileStyle base class for styling polygon features.
 */
public class VectorTilePolygonStyle extends VectorTileStyle {

    private final VectorInfo vectorInfo;


    public VectorTilePolygonStyle(String ident,String category,VectorInfo vectorInfo,
                                  VectorStyleSettings settings,RenderControllerInterface viewC) {
        super(ident,category,viewC);
        this.vectorInfo = vectorInfo;
    }

    static double ClipGridSize = 2.0/180.0*Math.PI;

    public void buildObjects(VectorObject[] objects, VectorTileData tileData, RenderControllerInterface controller) {

        boolean globeMode = (controller instanceof GlobeController);
        ArrayList<VectorObject> vectors = new ArrayList<>();

        for (VectorObject vecObj : objects) {

            VectorObject tessObj = null;
            Point2d center = vecObj.centroid();

            if (globeMode && center != null)
            {
                // We adjust the grid clipping size based on the latitude
                // This helps a lot near the poles.  Otherwise we're way oversampling
                float thisClipGridLon = (float)ClipGridSize;
                if (Math.abs(center.getY()) > 60.0/180.0 * Math.PI)
                    thisClipGridLon *= 4.0;
                else if (Math.abs(center.getY()) > 45.0/180.0 * Math.PI)
                    thisClipGridLon *= 2.0;

                vecObj.getAttributes().setDouble("veccenterx",center.getX());
                vecObj.getAttributes().setDouble("veccentery",center.getY());

                // We clip the vector to a grid and then tesselate the results
                // This forms the vector closer to the globe, make it look nicer
                VectorObject clipped = vecObj.clipToGrid(new Point2d(thisClipGridLon, ClipGridSize));
                if (clipped != null)
                    tessObj = clipped.tesselate();
            }

            if (tessObj == null)
                tessObj = vecObj.tesselate();

            if (tessObj != null)
                vectors.add(tessObj);

        }


        ComponentObject compObj = controller.addVectors(vectors, vectorInfo, RenderController.ThreadMode.ThreadCurrent);
        tileData.addComponentObject(compObj);
    }

}
