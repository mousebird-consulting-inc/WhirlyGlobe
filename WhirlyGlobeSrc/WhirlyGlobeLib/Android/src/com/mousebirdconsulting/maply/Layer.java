package com.mousebirdconsulting.maply;

public class Layer
{
	LayerThread layerThread = null;
	
	public void startLayer(LayerThread inLayerThread)
	{
		layerThread = inLayerThread;
	}
	
	public void shutdown()
	{
	}
}
