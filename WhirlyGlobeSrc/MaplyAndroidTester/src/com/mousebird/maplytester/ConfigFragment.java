package com.mousebird.maplytester;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import android.app.ListFragment;
import android.content.Context;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ListView;

/**
 * This fragment handles configuration, letting the user turn features on and off.
 *
 */
public class ConfigFragment extends ListFragment 
{
	// Whether an entry is available in 2D (map), 3D (globe) or both
	public enum Available
	{
		Map, Globe, None, All
	};
	
	// Identifiers for base layers
	public enum OptionIdent
	{
		BlankLayer,GeographyClass,MapboxRegular,MapboxSatellite,MapboxTerrain,OSMMapquest,StamenWatercolor,QuadTest,QuadTestAnimate,QuadVectorTest
	};
			
	// Single entry for 
	class ConfigEntry
	{
		public ConfigEntry(String inTitle,OptionIdent inOptionIdent,Available inAvail)
		{
			title = inTitle;
			avail = inAvail;
			optionIdent = inOptionIdent;
			status = false;
		}
		
		@Override public String toString()
		{
			return title;
		}
		
		public String title;
		OptionIdent optionIdent;
		public Available avail;
		public boolean status;  // Turned on or off
	}
	
	// Section grouping various things together
	class ConfigSection
	{
		public ConfigSection(String inTitle,boolean inSingle)
		{
			title = inTitle;
			single = inSingle;
		}
		
		public String title;
		public boolean single;
		public ArrayList<ConfigEntry> entries = new ArrayList<ConfigEntry>();
	}

	/**
	 * Implement this listener to get configuration changes
	 */
	public interface ConfigFragmentListener
	{
		public void userChangedSelections(ConfigFragment config);
	}
	ConfigFragmentListener configListen = null;
	
	/**
	 * Set the config fragment listener if you want configuration changes.
	 */
	public void setConfigFragmentListener(ConfigFragmentListener newOne)
	{
		configListen = newOne;
	}
	
	public ArrayList<ConfigSection> sections = new ArrayList<ConfigSection>();
	
	// Converts the entries into display-able stuff
	private class ConfigArrayAdapter extends ArrayAdapter<ConfigEntry>
	{
		HashMap<ConfigEntry, Integer> mIdMap = new HashMap<ConfigEntry, Integer>();
		
		public ConfigArrayAdapter(Context context, int textViewResourceId,List<ConfigEntry> entries)
		{
			super(context, textViewResourceId, entries);
			for (int ii=0;ii<entries.size();ii++)
				mIdMap.put(entries.get(ii), ii);
		}
		
		@Override
		public long getItemId(int position)
		{
			ConfigEntry entry = getItem(position);
			return mIdMap.get(entry);
		}
		
		@Override
		public boolean hasStableIds()
		{
			return true;
		}
	}
	
	// Set up the sections and entries
	public ConfigFragment()
	{
		// ,,,,OSMMapquest,StamenWatercolor,QuadTest,QuadTestAnimate,QuadVectorTest
		ConfigSection baseSection = new ConfigSection("Base Layers",true);
		baseSection.entries.add(new ConfigEntry("Blank",OptionIdent.BlankLayer,Available.All));
		baseSection.entries.add(new ConfigEntry("Geography Class (local)",OptionIdent.GeographyClass,Available.None));
		baseSection.entries.add(new ConfigEntry("Mapbox Regular (remote)",OptionIdent.MapboxRegular,Available.All));
		baseSection.entries.add(new ConfigEntry("Mapbox Satellite (remote)",OptionIdent.MapboxSatellite,Available.All));
		baseSection.entries.add(new ConfigEntry("Mapbox Terrain (remote)",OptionIdent.MapboxTerrain,Available.All));
		baseSection.entries.add(new ConfigEntry("OpenStreetMap - Mapquest (remote)",OptionIdent.OSMMapquest,Available.None));
		baseSection.entries.add(new ConfigEntry("Stamen Watercolor (remote)",OptionIdent.StamenWatercolor,Available.None));
		baseSection.entries.add(new ConfigEntry("Quad Test Layer",OptionIdent.QuadTest,Available.All));
		baseSection.entries.add(new ConfigEntry("Quad Test Layer - Animated (remote)",OptionIdent.QuadTestAnimate,Available.None));
		baseSection.entries.add(new ConfigEntry("Quad Vector Test Layer",OptionIdent.QuadVectorTest,Available.All));
		sections.add(baseSection);
		
//		ConfigSection objectSection = new ConfigSection("Maply Objects",false);
//		objectSection.entries.add(new ConfigEntry("Countries",Available.All));
//		objectSection.entries.add(new ConfigEntry("Labels - 2D",Available.None));
//		objectSection.entries.add(new ConfigEntry("Labels - 3D",Available.None));
//		objectSection.entries.add(new ConfigEntry("Lofted Polygons",Available.None));
//		objectSection.entries.add(new ConfigEntry("Lon/Lat Lines",Available.None));
//		objectSection.entries.add(new ConfigEntry("Markers - 2D",Available.None));
//		objectSection.entries.add(new ConfigEntry("Markers - 3D",Available.None));
//		objectSection.entries.add(new ConfigEntry("Mega Markers",Available.None));
//		objectSection.entries.add(new ConfigEntry("Models",Available.None));
//		objectSection.entries.add(new ConfigEntry("SF Roads",Available.None));
//		objectSection.entries.add(new ConfigEntry("Shapes: Arrows",Available.None));
//		objectSection.entries.add(new ConfigEntry("Shapes: Cylinders",Available.None));
//		objectSection.entries.add(new ConfigEntry("Shapes: Great Circles",Available.None));
//		objectSection.entries.add(new ConfigEntry("Shapes: Sphers",Available.None));
//		objectSection.entries.add(new ConfigEntry("Stickers",Available.None));
//		sections.add(objectSection);
	}
	
	@Override public View onCreateView(LayoutInflater inflater, ViewGroup container,Bundle savedInstance)
	{
		View rootView = inflater.inflate(R.layout.configlistview, container, false);
		
		// Note: Just showing the first list for now
		ConfigArrayAdapter adapter = new ConfigArrayAdapter(getActivity(),android.R.layout.simple_list_item_1,sections.get(0).entries);
		setListAdapter(adapter);
		return rootView;
	}
		
	// User selected something
	@Override public void onListItemClick (ListView l, View v, int position, long id)
	{
		// Note: Just doing the first section for now
		ConfigSection section = sections.get(0);
		if (section.single)
		{
			for (ConfigEntry entry: section.entries)
				entry.status = false;
		}
		ConfigEntry entry = section.entries.get(position);
		entry.status = true;
		
		if (configListen != null)
			configListen.userChangedSelections(this);
	}
}
