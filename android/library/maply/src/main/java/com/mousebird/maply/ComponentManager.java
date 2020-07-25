/*
 *  ComponentObject.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/18/19.
 *  Copyright 2011-2019 mousebird consulting
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
import java.util.Map;
import java.util.Vector;
/**
 * The Component Manager tracks the various low level IDs for
 * visual geometry that's been added, often as a group.
 */
public class ComponentManager
{
    private ComponentManager() { }

    public ComponentManager(Scene scene)
    {
        initialise(scene);
    }

    public void finalize()
    {
        dispose();
    }

    public ComponentObject makeComponentObject()
    {
        return new ComponentObject();
    }

    /**
     * Hand a component object over to be managed.
     */
    public native void addComponentObject(ComponentObject compObj);

    /**
     * Return true if the given component object still exists.
     */
    public native boolean hasComponentObject(long compID);

    /**
     * Remove the list of component objects from display
     */
    public native void removeComponentObjectsNative(ComponentObject[] compObjs,ChangeSet changes);

    public void removeComponentObjects(ComponentObject[] compObjs,ChangeSet changes,boolean disposeAfterRemoval)
    {
        for (ComponentObject compObj : compObjs)
        {
            removeSelectableObjects(compObj,disposeAfterRemoval);
        }

        removeComponentObjectsNative(compObjs,changes);
        for (ComponentObject compObj : compObjs)
            compObj.dispose();
    }

    /**
     * Enable/disable the component objects given.
     */
    public native void enableComponentObjects(ComponentObject[] compObj,boolean enable,ChangeSet changes);

    /**
     * Enable/disable a single component object.
     */
    public void enableComponentObject(ComponentObject compObj,boolean enable,ChangeSet changes)
    {
        ComponentObject[] compObjs = new ComponentObject[1];
        compObjs[0] = compObj;
        enableComponentObjects(compObjs,enable,changes);
    }

    // Remove selectable objects
    private void removeSelectableObjects(ComponentObject compObj,boolean disposeAfterRemoval)
    {
        long[] selectIDs = compObj.getSelectIDs();
        if (selectIDs != null)
        {
            synchronized(selectionMap)
            {
                for (long selectID : selectIDs) {
                    Object selObj = selectionMap.get(selectID);
                    if (selObj != null)
                    {
                        if (disposeAfterRemoval)
                        {
                            // Note: We should fix this for the other object types
                            if (selObj.getClass() == VectorObject.class)
                            {
                                VectorObject vecObj = (VectorObject)selObj;
                                vecObj.dispose();
                            }
                        }
                        selectionMap.remove(selectID);
                    }
                }
            }
        }
    }

    Map<Long, Object> selectionMap = new HashMap<Long, Object>();

    // Add selectable objects to the list
    public void addSelectableObject(long selectID,Object selObj,ComponentObject compObj)
    {
        synchronized(selectionMap)
        {
            compObj.addSelectID(selectID);
            selectionMap.put(selectID,selObj);
        }
    }

    public void remapSelectableObjects(SelectedObject[] selManObjs)
    {
        // Remap the objects
        synchronized(selectionMap) {
            for (SelectedObject selObj : selManObjs) {
                long selectID = selObj.getSelectID();
                selObj.selObj = selectionMap.get(selectID);
            }
        }
    }

    public Object findObjectForSelectID(long selectID)
    {
        // Look for the object
        Object selObj = null;
        synchronized(selectionMap)
        {
            selObj = selectionMap.get(selectID);
        }

        return selObj;
    }

    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise(Scene scene);
    native void dispose();
    private long nativeHandle;
}
