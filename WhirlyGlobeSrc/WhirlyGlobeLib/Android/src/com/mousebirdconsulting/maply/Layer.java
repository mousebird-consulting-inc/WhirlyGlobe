package com.mousebirdconsulting.maply;

/**
 * The Layer subclass is used by the LayerThread to track Maply
 * objects that need to be updated on a regular basis.  The various
 * layer subclasses are accessible to toolkit users.
 * 
 * @author sjg
 *
 */
class Layer
{
	LayerThread layerThread = null;

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
