package com.mousebird.maplytester;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import android.content.Context;
import android.graphics.Color;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;

/**
 * This fragment handles configuration, letting the user turn features on and off.
 *
 */
public class ConfigView extends ListView
{
	// Whether an entry is available in 2D (map), 3D (globe) or both
	public enum Available
	{
		Map, Globe, None, All
	};
	
	// Identifiers for base layers
	public enum OptionIdent
	{
		BlankLayer,GeographyClass,MapboxRegular,MapboxSatellite,OSMMapquest,StamenWatercolor,QuadTest,QuadTestAnimate,QuadVectorTest,ForecastIO,PerfOutput
	};
	
	// Order of sections
	public enum Sections
	{
		BaseSection,OverlaySection,InternalSection
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
	public interface ConfigViewListener
	{
		public void userChangedSelections(ConfigView config);
	}
	ConfigViewListener configListen = null;
	
	/**
	 * Set the config fragment listener if you want configuration changes.
	 */
	public void setConfigViewListener(ConfigViewListener newOne)
	{
		configListen = newOne;
	}
	
	public ArrayList<ConfigSection> sections = new ArrayList<ConfigSection>();
	
	// Converts the entries into display-able stuff
	private class ConfigArrayAdapter extends ArrayAdapter<ConfigEntry>
	{
		ArrayList<ConfigSection> sections;
		
		public ConfigArrayAdapter(Context context, int textViewResourceId, ArrayList<ConfigSection> inSections)
		{
			super(context, textViewResourceId);
			sections = inSections;
		}
				
		@Override
		public int getCount()
		{
			int num = 0;
			for (ConfigSection section : sections)
				num += section.entries.size() + 1;
			return num;
		}
		
		@Override
		public boolean hasStableIds()
		{
			return true;
		}

		// Return from a position query
		class ConfigReturn
		{
			public String header = null;
			public ConfigSection section = null;
			public ConfigEntry entry = null;
		}
		
		public ConfigReturn getEntry(int position)
		{
			ConfigSection section = null;
			ConfigEntry entry = null;
			String header = null;
			int num = position;
			for (ConfigSection theSection : sections)
			{
				section = theSection;
				if (num == 0)
				{
					header = theSection.title;
					break;
				} else {
					if (num > theSection.entries.size())
						num -= theSection.entries.size()+1;
					else {
						entry = theSection.entries.get(num-1);
						break;
					}
				}
			}
			
			ConfigReturn configEntry = new ConfigReturn();
			configEntry.entry = entry;
			configEntry.section = section;
			configEntry.header = header;

			return configEntry;
		}
				
		@Override
		public View getView (int position, View convertView, ViewGroup parent)
		{
			ConfigReturn configEntry = getEntry(position);
			
			TextView textView = new TextView(parent.getContext());
			textView.setTextSize(24);
			textView.setHeight(64);
			if (configEntry.header != null)
			{
				textView.setText(configEntry.header);
				textView.setTextColor(Color.BLACK);
			} else {
				if (configEntry.entry.status)
					textView.setBackgroundColor(Color.LTGRAY);
				textView.setTextColor(Color.DKGRAY);
				textView.setText(configEntry.entry.toString());
			}

			return textView;
		}
	}
	
	// Set up the sections and entries
	public ConfigView(Context ctx)
	{
		super(ctx);
		setChoiceMode(ListView.CHOICE_MODE_MULTIPLE);
		setDividerHeight(1);
		setBackgroundColor(0xFFFFFFFF);
		
		ConfigSection baseSection = new ConfigSection("Base Layers",true);
		baseSection.entries.add(new ConfigEntry("Blank",OptionIdent.BlankLayer,Available.All));
//		baseSection.entries.add(new ConfigEntry("Geography Class (local)",OptionIdent.GeographyClass,Available.None));
		ConfigEntry mapboxSat = new ConfigEntry("Mapbox Satellite (remote)",OptionIdent.MapboxSatellite,Available.All);
		mapboxSat.status = true;
		baseSection.entries.add(mapboxSat);
		baseSection.entries.add(new ConfigEntry("OpenStreetMap - Mapquest (remote)",OptionIdent.OSMMapquest,Available.None));
		baseSection.entries.add(new ConfigEntry("Stamen Watercolor (remote)",OptionIdent.StamenWatercolor,Available.None));
		baseSection.entries.add(new ConfigEntry("Quad Test Layer",OptionIdent.QuadTest,Available.All));
//		baseSection.entries.add(new ConfigEntry("Quad Test Layer - Animated (remote)",OptionIdent.QuadTestAnimate,Available.None));
		baseSection.entries.add(new ConfigEntry("Quad Vector Test Layer",OptionIdent.QuadVectorTest,Available.All));
		sections.add(baseSection);
		
		ConfigSection overlaySection = new ConfigSection("Overlay Layers",false);
		overlaySection.entries.add(new ConfigEntry("Forecast.IO Weather",OptionIdent.ForecastIO,Available.All));
		sections.add(overlaySection);
		
		ConfigSection intSection = new ConfigSection("Internal", false);
		intSection.entries.add(new ConfigEntry("Performance Output",OptionIdent.PerfOutput,Available.All));
		sections.add(intSection);
		
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
//		objectSection.entries.add(new ConfigEntry("Shapes: Spheres",Available.None));
//		objectSection.entries.add(new ConfigEntry("Stickers",Available.None));
//		sections.add(objectSection);
		
		// Note: Just showing the first list for now
		final ConfigArrayAdapter adapter = new ConfigArrayAdapter(ctx,android.R.layout.simple_list_item_1,sections);
		setAdapter(adapter);
		final ConfigView theConfigView = this;
		setOnItemClickListener(new android.widget.AdapterView.OnItemClickListener() {
			@Override
			public void onItemClick(AdapterView<?> arg0, View arg1, int position,
					long arg3) 
			{
				ConfigArrayAdapter.ConfigReturn configEntry = adapter.getEntry(position);
				if (configEntry.entry != null)
				{
					if (configEntry.section.single)
					{
						for (ConfigEntry entry: configEntry.section.entries)
							entry.status = false;						
						configEntry.entry.status = true;
					} else {
						configEntry.entry.status = !configEntry.entry.status;
					}
				}
				
				if (configListen != null)
					configListen.userChangedSelections(theConfigView);		
				
				adapter.notifyDataSetChanged();
			}
		});	
	}		
}
