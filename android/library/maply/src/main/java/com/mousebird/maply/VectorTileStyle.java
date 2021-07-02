/*  MaplyVectorTileStyle.java
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


import java.lang.ref.WeakReference;
import java.util.concurrent.atomic.AtomicLong;

/**
 *
 * VectorTileStyle is the base class for full-featured vector geometry styles.
 * This builds Maply objects for vector data, displaying the vectors on the globe or map.
 * @see VectorStyle
 */
public abstract class VectorTileStyle implements VectorStyle {

    @Override
    public long getUuid() { return uuid; }

    @Override
    public boolean geomIsAdditive() { return geomAdditive; }

    @Override
    public String getCategory() { return category; }

    @Override
    public String getIdent() { return ident; }


    public VectorTileStyle(String ident,String category,RenderControllerInterface viewC) {
        this.ident = ident;
        this.category = category;
        this.viewC = new WeakReference<>(viewC);

//        if (((String)styleEntry.get("tilegeom")).equals("add"))
//            geomAdditive = true;
//
//        selectable = ((Boolean)styleEntry.get("?")).booleanValue();
    }

    protected boolean geomAdditive;
    protected boolean selectable;
    protected String ident;
    protected String category;

    final protected WeakReference<RenderControllerInterface> viewC;

    final protected long uuid = uuidCount.incrementAndGet();

    static final AtomicLong uuidCount = new AtomicLong(0);
}
