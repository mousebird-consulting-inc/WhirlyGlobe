/*
 *  MaplyVectorTileStyle.java
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


import java.lang.ref.WeakReference;
import java.util.HashMap;

public abstract class VectorTileStyle implements VectorStyle {

    protected boolean geomAdditive;
    protected String uuid;
    protected WeakReference<MaplyBaseController> viewC;
    protected boolean selectable;

    public String getUuid() {
        return uuid;
    }

    public boolean geomIsAdditive() {
        return geomAdditive;
    }

    public VectorTileStyle(MaplyBaseController viewC) {
        this.viewC = new WeakReference<MaplyBaseController>(viewC);
        uuid = " " + Math.random() * 1000000 + Math.random() * 10000;

//        if (((String)styleEntry.get("tilegeom")).equals("add"))
//            geomAdditive = true;
//
//        selectable = ((Boolean)styleEntry.get("?")).booleanValue();
    }


}
