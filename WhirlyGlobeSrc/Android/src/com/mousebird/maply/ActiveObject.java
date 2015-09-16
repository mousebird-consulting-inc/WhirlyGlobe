/*
 *  AngleAxis.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/20/15.
 *  Copyright 2011-2015 mousebird consulting
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
 * Active Objects are called right before the render on the render thread.
 * You can update objects on this thread, but be aware you're on the
 * render thread and act accordingly.  Also don't take very long.
 */
public interface ActiveObject {
    /**
     * Called right before the render on the render thread
     */
    public void activeUpdate();
}
