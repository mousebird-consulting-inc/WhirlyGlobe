/*
 *  Layer.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2014 mousebird consulting
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
 * The Layer subclass is used by the LayerThread to track Maply
 * objects that need to be updated on a regular basis.  The various
 * layer subclasses are accessible to toolkit users.
 * 
 * @author sjg
 *
 */
public class Layer
{
	public LayerThread layerThread = null;

	/**
	 * Once your layer is created and handed to the LayerThread, it needs to
	 * actually be started on that layer.  This call indicates the layer has
	 * been started on the thread and can hook itself into the system, generating
	 * geometry or registering for view changes and such.
	 * 
	 * @param inLayerThread
	 */
	public void startLayer(LayerThread inLayerThread)
	{
		layerThread = inLayerThread;
	}

	/**
	 * Called on the layer thread right before we flush changes out to the scene.
	 * You can be guaranteed that no changes you make appear before this is called.
	 * So it's nice for syncing up display state with the main thread.
	 */
	public void preSceneFlush(LayerThread layerThread)
	{
	}

	/**
	 * This method is called when a layer is to be removed.  The layer should
	 * clean up any objects it may have created.
	 * <p>
	 * If the MaplyController is shut down, you may not get this call and instead
	 * may simply be deleted.
	 */
	public void shutdown()
	{
	}
}
