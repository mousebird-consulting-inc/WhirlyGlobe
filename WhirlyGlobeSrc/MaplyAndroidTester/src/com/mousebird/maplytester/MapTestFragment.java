package com.mousebird.maplytester;

import java.io.File;

import com.mousebird.maply.MapFragment;
import com.mousebird.maply.QuadImageTileLayer;
import com.mousebird.maply.RemoteTileSource;
import com.mousebird.maply.SphericalMercatorCoordSystem;
import com.mousebird.maplytester.ConfigFragment.ConfigFragmentListener;

import android.app.Fragment;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

/**
 * This fragment manages the map (globe), adding and removing content as the user requests.
 * 
 */
public class MapTestFragment extends Fragment implements ConfigFragmentListener
{
	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState)
	{
		View view = inflater.inflate(R.layout.fragment_map, container, false);
		
		ConfigFragment config = (ConfigFragment) getFragmentManager().findFragmentById(R.id.configlistview);
		config.setConfigFragmentListener(this);
		
		return view;
	}
	
	QuadImageTileLayer baseLayer = null;

	// Called when the user changes what is selected
	@Override
	public void userChangedSelections(ConfigFragment config) 
	{
		MapFragment mapFragment = (MapFragment) getFragmentManager().findFragmentById(R.id.mapview);
		
		String cacheDirName = null;
		String remoteSource = null;
		
		// Get rid of the existing base layer
//		if (baseLayer != null)
//		{
//			mapFragment.mapControl.removeLayer(baseLayer);
//		}

		// Note: Create a new base layer
		
		// Cache directory for tiles
		File cacheDir = new File(getActivity().getCacheDir(),"testbasemap");
		cacheDir.mkdir();
		
		SphericalMercatorCoordSystem coordSys = new SphericalMercatorCoordSystem();
		RemoteTileSource tileSource = new RemoteTileSource("http://a.tiles.mapbox.com/v3/examples.map-zyt2v9k2/","png",0,22);
		tileSource.setCacheDir(cacheDir);
//		tileSource.delegate = this;
		QuadImageTileLayer layer = new QuadImageTileLayer(mapFragment.mapControl,coordSys,tileSource);
		layer.setSimultaneousFetches(8);
		mapFragment.mapControl.addLayer(layer);		
	}
	
}
