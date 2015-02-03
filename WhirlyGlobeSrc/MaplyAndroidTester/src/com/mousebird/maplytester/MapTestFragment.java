package com.mousebird.maplytester;

import java.io.File;

import com.mousebird.maply.MapFragment;
import com.mousebird.maply.QuadImageTileLayer;
import com.mousebird.maply.RemoteTileSource;
import com.mousebird.maply.SphericalMercatorCoordSystem;
import com.mousebird.maplytester.ConfigFragment.ConfigFragmentListener;
import com.mousebird.maplytester.ConfigFragment.OptionIdent;

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
		RemoteTileSource tileSource = null;
		
		// Get rid of the existing base layer
		if (baseLayer != null)
		{
			mapFragment.mapControl.removeLayer(baseLayer);
		}
		
		for (ConfigFragment.ConfigEntry entry : config.sections.get(0).entries)
		{
			if (entry.status)
			{
				switch (entry.optionIdent)
				{
				case BlankLayer:
					break;
				case GeographyClass:
					// Note: Put MBTiles in resources
					break;
				case MapboxRegular:
//					cacheDirName = "mapbox_regular";
//					tileSource = new RemoteTileSource("http://a.tiles.mapbox.com/v3/examples.map-zswgei2n/","png",0,22);				
					break;
				case MapboxSatellite:
					cacheDirName = "mapbox_satellite";
					tileSource = new RemoteTileSource("http://a.tiles.mapbox.com/v3/examples.map-zyt2v9k2/","png",0,22);
					break;
				case MapboxTerrain:
//					cacheDirName = "mapbox_terrain";
//					tileSource = new RemoteTileSource("http://a.tiles.mapbox.com/v3/examples.map-zq0f1vuc/","png",0,22);
					break;
				case OSMMapquest:
					cacheDirName = "osm_mapquest";
					tileSource = new RemoteTileSource("http://otile1.mqcdn.com/tiles/1.0.0/osm/","png",0,18);
					break;
				case StamenWatercolor:
					cacheDirName = "stamen_watercolor";
					tileSource = new RemoteTileSource("http://tile.stamen.com/watercolor/","png",0,18);
					break;
				case QuadTest:
					break;
				case QuadTestAnimate:
					break;
				case QuadVectorTest:
					break;
				}
			}
		}

		// New base layer
		if (tileSource != null)
		{
			// Set up the layer
			SphericalMercatorCoordSystem coordSys = new SphericalMercatorCoordSystem();
			baseLayer = new QuadImageTileLayer(mapFragment.mapControl,coordSys,tileSource);
			baseLayer.setSimultaneousFetches(8);

			// Cache directory for tiles
			if (cacheDirName != null)
			{
				File cacheDir = new File(getActivity().getCacheDir(),cacheDirName);
				cacheDir.mkdir();
				tileSource.setCacheDir(cacheDir);
			}
				
			mapFragment.mapControl.addLayer(baseLayer);		
		}
	}
	
}
