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

/**
 * The Component Manager tracks the various low level IDs for
 * visual geometry that's been added, often as a group.
 */
public class ComponentManager
{
    public ComponentManager()
    {
        initialise();
    }

    public void finalize()
    {
        dispose();
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
    public native void removeComponentObjects(ComponentObject[] compObjs);

    /**
     * Enable/disable the component objects given.
     */
    public native void enableComponentObjects(ComponentObject[] compObj,boolean enable,ChangeSet changes);


    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise();
    native void dispose();
}
