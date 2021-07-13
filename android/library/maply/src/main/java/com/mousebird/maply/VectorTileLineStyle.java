/*  MaplyVectorTileLineStyle.java
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
import java.util.Arrays;

/**
 * The VectorTileStyle base class for styling line features.
 */
public class VectorTileLineStyle extends VectorTileStyle {

    private final boolean useWideVectors;
    private WideVectorInfo wideVectorInfo;
    private VectorInfo vectorInfo;

    public VectorTileLineStyle(String ident,String category,BaseInfo baseInfo,
                               VectorStyleSettings settings,RenderControllerInterface viewC) {
        super(ident,category,viewC);
        if (baseInfo instanceof VectorInfo) {
            useWideVectors = false;
            vectorInfo = (VectorInfo)baseInfo;
        } else if (baseInfo instanceof WideVectorInfo) {
            useWideVectors = true;
            wideVectorInfo = (WideVectorInfo)baseInfo;
        } else {
            throw new IllegalArgumentException();
        }
    }

    public void buildObjects(VectorObject[] objects, VectorTileData tileData, RenderControllerInterface controller) {
        ArrayList<VectorObject> vecObjs = new ArrayList<>(Arrays.asList(objects));

        ComponentObject compObj;
        if (useWideVectors) {
            compObj = controller.addWideVectors(vecObjs, wideVectorInfo, RenderController.ThreadMode.ThreadCurrent);
        }
        else {
            compObj = controller.addVectors(vecObjs, vectorInfo, RenderController.ThreadMode.ThreadCurrent);
        }
        tileData.addComponentObject(compObj);
    }

}
