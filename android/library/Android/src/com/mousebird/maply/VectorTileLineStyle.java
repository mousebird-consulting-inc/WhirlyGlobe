/*
 *  MaplyVectorTileLineStyle.java
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
import java.util.Vector;


public class VectorTileLineStyle extends VectorTileStyle {

    private boolean useWideVectors;
    private WideVectorInfo wideVectorInfo;
    private VectorInfo vectorInfo;

    public VectorTileLineStyle(BaseInfo baseInfo, VectorStyleSettings settings, MaplyBaseController viewC) {
        super(viewC);
        if (baseInfo instanceof VectorInfo) {
            useWideVectors = false;
            vectorInfo = (VectorInfo)baseInfo;
        } else if (baseInfo instanceof WideVectorInfo) {
            useWideVectors = true;
            wideVectorInfo = (WideVectorInfo)baseInfo;
        } else
            throw new IllegalArgumentException();

    }


    public ComponentObject[] buildObjects(List<VectorObject> objects, MaplyTileID tileID, MaplyBaseController controller) {
        ComponentObject compObj = null;
        if (useWideVectors)
            compObj = controller.addWideVectors(objects, wideVectorInfo, MaplyBaseController.ThreadMode.ThreadCurrent);
        else
            compObj = controller.addVectors(objects, vectorInfo, MaplyBaseController.ThreadMode.ThreadCurrent);
        if (compObj != null)
            return new ComponentObject[]{compObj};
        return null;
    }

}
