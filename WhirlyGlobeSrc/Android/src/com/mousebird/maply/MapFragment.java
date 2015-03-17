package com.mousebird.maply;

import android.app.Fragment;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

/**
 * The MapFragment implements a 2D map with gestures and such.
 */
public class MapFragment extends Fragment 
{
	// Handles drawing, interaction, and so forth for Maply
	public MapController mapControl = null;
	
	public MapFragment()
	{
	}
	
	// Create the map controller
	@Override 
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		
    	// Create the Map Controller
    	mapControl = new MapController(this.getActivity());
	}

	// Return the render view, to be hooked into the hierarchy
	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState)
	{
		super.onCreateView(inflater,container,savedInstanceState);
		
    	View renderView = mapControl.getContentView();
		    	
    	return renderView;
	}

	// Note: Turn off the rendering loop and layer thread?
	@Override
	public void onPause()
	{
		super.onPause();
	}
	
	@Override
	public void onDestroy()
	{
		super.onDestroy();
		
		mapControl.shutdown();
		mapControl = null;
	}
}
