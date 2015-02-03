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
	public MaplyController mapControl = null;
	
	public MapFragment()
	{
	}
	
	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState)
	{
    	// Create the Maply Controller
    	mapControl = new MaplyController(this.getActivity());
    	View renderView = mapControl.getContentView();
    	// Report stats every 300 frames
//    	mapControl.setPerfInterval(300);
		    	
    	return renderView;
	}

}
