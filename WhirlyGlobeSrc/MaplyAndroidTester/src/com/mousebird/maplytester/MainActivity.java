package com.mousebird.maplytester;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import com.mousebird.maply.MaplyController;
import com.mousebird.maplytester.TestRemoteImageTiles;

import android.os.*;
import android.app.*;
//import android.util.Log;
//import android.widget.Toast;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ListView;
import android.widget.SimpleAdapter;

/**
 * The MainActivity is a test activity for the Maply library.
 * For a real app, you'll be wanting to make one of these of your own.
 * 
 * @author sjg
 *
 */
public class MainActivity extends Activity 
{
	// Handles drawing, interaction, and so forth for Maply
	MaplyController mapControl = null;
	
	List<Map<String, String>> optionsList = new ArrayList<Map<String,String>>();

	// Main constructor
	public MainActivity()
	{
		System.loadLibrary("Maply");

		optionsList.add(createEntry("entry", "OSM Paging"));
		optionsList.add(createEntry("entry", "Satellite Basemap"));
		optionsList.add(createEntry("entry", "Countries"));		
		optionsList.add(createEntry("entry", "Test Vector Pager"));
		optionsList.add(createEntry("entry", "Test Image Pager"));
	}
	
	// Create an entry for the list view
	private HashMap<String,String> createEntry(String key,String name)
	{
		HashMap<String,String> entry = new HashMap<String, String>();
		entry.put(key,name);
		
		return entry;
	}
	
	// Demo types for the user to choose
	public enum DemoType { OSMPaging, SatelliteBasemap, Countries, TestVectorPager, TestImagePager };
	
	@Override
	protected void onCreate(Bundle savedInstState)
	{
		super.onCreate(savedInstState);
		
		startListView();
	}
	
	// Fire up the list view for user selection
	void startListView()
	{
		setContentView(R.layout.activity_main);
				
		final MainActivity mainActivity = this;
		
		// Set up the list view
		ListView lv = (ListView) findViewById(R.id.listView);
		SimpleAdapter simpleAdpt = new SimpleAdapter(this, optionsList, android.R.layout.simple_list_item_1, new String[] {"entry"}, new int[] {android.R.id.text1});
		lv.setAdapter(simpleAdpt);
		lv.setOnItemClickListener(new AdapterView.OnItemClickListener() {
			public void onItemClick(AdapterView<?> parentAdapter, View view, int position,long id)
			{
				mainActivity.startDemo(DemoType.values()[position]);
			}
		});		
	}
	
	void startDemo(DemoType type)
	{
    	// Create the Maply Controller
    	mapControl = new MaplyController(this);
    	View renderView = mapControl.getContentView();
    	if (renderView != null)
    		this.setContentView(renderView);

		switch (type)
		{
		case OSMPaging:
		{
	    	TestRemoteOSM test = new TestRemoteOSM(this,mapControl);
	    	test.start();
		}
			break;
		case SatelliteBasemap:
		{
	    	TestRemoteImageTiles test = new TestRemoteImageTiles(this,mapControl);
	    	test.start();
		}
			break;
		case Countries:
		{
	    	TestCountries test = new TestCountries(this,mapControl);
	    	test.start();
		}
			break;
		case TestVectorPager:
		{
			TestVectorPager test = new TestVectorPager(this,mapControl);
			test.start();
		}
			break;
		case TestImagePager:
		{
			TestImagePagerSource test = new TestImagePagerSource(this,mapControl);
			test.start();
		}
			break;
		}				
	}	
	
	@Override
	public void onBackPressed()
	{
		// Tear down the display
		if (mapControl != null)
		{
			mapControl.shutdown();
			mapControl = null;
		}
		// Pop back to the list view
		startListView();
	}
}
