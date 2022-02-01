/*
 *  ComponentObject.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/18/19.
 *  Copyright 2011-2022 mousebird consulting
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

import android.util.Log;

import com.mousebirdconsulting.whirlyglobemaply.BuildConfig;

import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Vector;
/**
 * The Component Manager tracks the various low level IDs for
 * visual geometry that's been added, often as a group.
 */
public class ComponentManager
{
    @SuppressWarnings("unused")     // Needed for JNI JavaClassInfo
    private ComponentManager() { }

    public ComponentManager(Scene scene) {
        initialise(scene);
    }

    public ComponentObject makeComponentObject() {
        return new ComponentObject();
    }

    /**
     * Hand a component object over to be managed.
     */
    public native void addComponentObject(ComponentObject compObj,ChangeSet changes);

    /**
     * Return true if the given component object still exists.
     */
    public native boolean hasComponentObject(long compID);

    /**
     * Remove the list of component objects from display
     */
    private native void removeComponentObjectsNative(ComponentObject[] compObjs,
                                                     ChangeSet changes,
                                                     boolean disposeAfterRemoval);

    public void removeComponentObjects(ComponentObject[] compObjs,ChangeSet changes,boolean disposeAfterRemoval) {
        // This calls `objectsRemoved` with the removed IDs to clean up the selection maps
        removeComponentObjectsNative(compObjs,changes,disposeAfterRemoval);

        if (disposeAfterRemoval) {
            for (ComponentObject compObj : compObjs) {
                compObj.dispose();
            }
        }
    }

    /**
     * Enable/disable the component objects given.
     */
    public native void enableComponentObjects(ComponentObject[] compObj,boolean enable,ChangeSet changes);

    /**
     * Enable/disable a single component object.
     */
    public void enableComponentObject(ComponentObject compObj,boolean enable,ChangeSet changes) {
        enableComponentObjects(new ComponentObject[] { compObj },enable,changes);
    }

    final Map<Long, Object> selectionMap = new HashMap<>();
    final Map<Long, ComponentObject> compObjMap = new HashMap<>();

    // Add selectable objects to the list
    public void addSelectableObject(long selectID,Object selObj,ComponentObject compObj) {
        synchronized(selectionMap) {
            compObj.addSelectID(selectID);
            selectionMap.put(selectID,selObj);
            compObjMap.put(compObj.getID(),compObj);
        }
    }

    public void remapSelectableObjects(SelectedObject[] selManObjs) {
        // Remap the objects
        synchronized(selectionMap) {
            for (SelectedObject selObj : selManObjs) {
                final long selectID = selObj.getSelectID();
                selObj.selObj = selectionMap.get(selectID);
            }
        }
    }

    public Object findObjectForSelectID(long selectID) {
        // Look for the object
        synchronized(selectionMap) {
            return selectionMap.get(selectID);
        }
    }

    // Called by the C++ side to let us know when objects are removed by the C++ side
    // This happens commonly with vector tiles
    public void objectsRemoved(long[] objIDs, boolean disposeAfterRemoval) {
        synchronized (selectionMap) {
            for (long objID: objIDs) {
                ComponentObject compObj = compObjMap.get(objID);
                if (compObj != null) {
                    removeSelectableObjectNoLock(objID, compObj.getSelectIDs(), disposeAfterRemoval);
                }
            }
        }
    }

    private void removeSelectableObjectNoLock(long objID, long[] selectIDs, boolean disposeAfterRemoval) {
        for (long selectID : selectIDs) {
            Object selObj = selectionMap.get(selectID);
            if (selObj != null) {
                if (disposeAfterRemoval) {
                    // todo: We should fix this for the other object types
                    if (selObj.getClass() == VectorObject.class) {
                        VectorObject vecObj = (VectorObject)selObj;
                        vecObj.dispose();
                    } else if (BuildConfig.DEBUG) {
                        Log.v("Maply", selObj.getClass().getSimpleName() + " not disposed");
                    }
                }
                selectionMap.remove(selectID);
            }
        }
        compObjMap.remove(objID);
    }

    public SelectedObject[] findVectors(Point2d geoPt, double maxDist, ViewState viewState,
                                               Point2d  frameSize) {
        return findVectors(geoPt, maxDist, viewState, frameSize, 0);
    }

    public native SelectedObject[] findVectors(Point2d geoPt, double maxDist, ViewState viewState,
                                               Point2d  frameSize, int limit);

    /**
     * Set the representation for a set of unique features
     */
    public native void setRepresentation(String repName, String fallbackName, String[] uuids, ChangeSet changes);

    static {
        nativeInit();
    }
    private static native void nativeInit();
    private native void initialise(Scene scene);
    public void finalize() {
        dispose();
    }
    native void dispose();
    @SuppressWarnings("unused")
    private long nativeHandle;
}
