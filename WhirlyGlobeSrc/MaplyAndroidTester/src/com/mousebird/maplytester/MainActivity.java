/*
 *  MainActivity.java
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

package com.mousebird.maplytester;

import com.mousebird.maply.MapFragment;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import android.os.*;
import android.app.*;
import android.view.LayoutInflater;
//import android.util.Log;
//import android.widget.Toast;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ListView;
import android.widget.SimpleAdapter;

/**
 * The MainActivity is a test activity for the Maply library.
 * For a real app, you'll be wanting to make one of these of your own.
 *
 */
public class MainActivity extends Activity 
{
	// Main constructor
	public MainActivity()
	{
		System.loadLibrary("Maply");
	}
		
	// Demo types for the user to choose
	public enum DemoType { Globe3D, GlobeElev3D, Map3D, Map2D };
	
	@Override
	protected void onCreate(Bundle savedInstState)
	{
		super.onCreate(savedInstState);
		
		setContentView(R.layout.activity_main);
		LocalListFragment lf = (LocalListFragment) getFragmentManager().findFragmentById(R.id.listFragment);
		lf.mainActivity = this;
	}
	
	// Simple list fragment
	static public class LocalListFragment extends ListFragment
	{
		public List<Map<String, String>> optionsList = new ArrayList<Map<String,String>>();
		public MainActivity mainActivity = null;
		
		// Create an entry for the list view
		private HashMap<String,String> createEntry(String key,String name)
		{
			HashMap<String,String> entry = new HashMap<String, String>();
			entry.put(key,name);
			
			return entry;
		}

		public LocalListFragment()
		{
			optionsList.add(createEntry("entry", "Globe (3D)"));
			optionsList.add(createEntry("entry", "Globe w/ Elevation (3D)"));
			optionsList.add(createEntry("entry", "Map (3D)"));		
			optionsList.add(createEntry("entry", "Map (2D)"));
		}
	
		@Override
		public View onCreateView(LayoutInflater inflater, ViewGroup container,
	            Bundle savedInstanceState)
		{
			SimpleAdapter simpleAdpt = new SimpleAdapter(getActivity(), optionsList, android.R.layout.simple_list_item_1, new String[] {"entry"}, new int[] {android.R.id.text1});
			setListAdapter(simpleAdpt);	
			
			return super.onCreateView(inflater, container, savedInstanceState);
		}
		
		@Override
		public void onListItemClick (ListView l, View v, int position, long id)
		{
			mainActivity.startDemo(DemoType.values()[position]);
		}
	}
		
	Fragment mapFragment = null;
	
	// Kick off the map (globe)
	void startDemo(DemoType type)
	{
		FragmentManager fragManage = getFragmentManager();
				
		switch (type)
		{
		case Globe3D:
		{
		}
			break;
		case GlobeElev3D:
		{
		}
			break;
		case Map3D:
		{
		}
			break;
		case Map2D:
		{
			mapFragment = new MapTestFragment();
		}
			break;
		}
		
		if (mapFragment != null)
			fragManage.beginTransaction()
				.hide(fragManage.findFragmentById(R.id.listFragment))
				.add(R.id.main_container,mapFragment)
				.commit();
	}	
	
	@Override
	public void onBackPressed()
	{
		FragmentManager fragManage = getFragmentManager();
		fragManage.beginTransaction()
		.remove(mapFragment)
		.show(fragManage.findFragmentById(R.id.listFragment))
		.commit();
		
		mapFragment = null;
	}
}
