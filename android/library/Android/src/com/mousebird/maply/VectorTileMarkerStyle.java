/*
 *  MaplyVectorTileMarkerStyle.java
 *  WhirlyGlobeLib
 *
 *  Created by Ranen Ghosh on 3/27/17.
 *  Copyright 2011-2017 mousebird consulting
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


import java.util.HashMap;
import java.util.List;
import java.util.ArrayList;
import com.mousebird.maply.MarkerInfo;
import android.graphics.Bitmap;


/**
 * The VectorTileStyle base class for styling markers.
 */
public class VectorTileMarkerStyle extends VectorTileStyle {

    private MarkerInfo markerInfo;
    private Bitmap bitmap;

    public VectorTileMarkerStyle(MarkerInfo markerInfo, Bitmap bitmap, VectorStyleSettings settings, MaplyBaseController viewC) {
        super(viewC);

        this.markerInfo = markerInfo;
        this.bitmap = bitmap;


    }

    public ComponentObject[] buildObjects(List<VectorObject> objects, MaplyTileID tileID, MaplyBaseController controller) {

        ArrayList<ScreenMarker> markers = new ArrayList<ScreenMarker>();
        for (VectorObject vector : objects) {
            Point2d centroid = vector.centroid();
            if (centroid != null) {
                ScreenMarker marker = new ScreenMarker();
                marker.image = bitmap;
                marker.loc = centroid;
                marker.size = new Point2d(32, 32);
                marker.selectable = true;
                markers.add(marker);
            }
        }

        ComponentObject compObj = controller.addScreenMarkers(markers, markerInfo, MaplyBaseController.ThreadMode.ThreadCurrent);
        if (compObj != null) {
            return new ComponentObject[]{compObj};
        }
        return null;
    }

}
