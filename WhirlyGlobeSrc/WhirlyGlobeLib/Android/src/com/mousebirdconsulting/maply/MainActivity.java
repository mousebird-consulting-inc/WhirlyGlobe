package com.mousebirdconsulting.maply;

import java.io.*;
import java.util.*;

import android.os.*;
import android.app.*;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.view.Menu;
//import android.widget.Toast;

public class MainActivity extends Activity 
{
	// Handles drawing, interaction, and so forth for Maply
	MaplyController mapControl;
	
    @Override
    protected void onCreate(Bundle savedInstanceState) 
    {    	
		System.loadLibrary("Maply");
    	
		// Wait for the debugger to catch up
    	try {
			Thread.sleep(5000);
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
    	
    	// Create the Maply Controller
    	mapControl = new MaplyController(this);
    	mapControl.setPosition(-122.416667 / 180.0 * 3.1415, 37.783333 / 180.0 * 3.1415, 4.0) ;
    	
    	// Go load vector files (on another thread, please)
    	Thread thread = new Thread()
    	{
        	String[] countries = {"ABW", "AFG", "AGO", "AIA", "ALA", "ALB", "AND", "ARE", "ARG", "ARM", "ASM", "ATA", "ATF", "ATG", "AUS", "AUT",
       	         "AZE", "BDI", "BEL", "BEN", "BES", "BFA", "BGD", "BGR", "BHR", "BHS", "BIH", "BLM", "BLR", "BLZ", "BMU", "BOL",
       	         "BRA", "BRB", "BRN", "BTN", "BVT", "BWA", "CAF", "CAN", "CCK", "CHE", "CHL", "CHN", "CIV", "CMR", "COD", "COG",
       	         "COK", "COL", "COM", "CPV", "CRI", "CUB", "CUW", "CXR", "CYM", "CYP", "CZE", "DEU", "DJI", "DMA", "DNK", "DOM",
       	         "DZA", "ECU", "EGY", "ERI", "ESH", "ESP", "EST", "ETH", "FIN", "FJI", "FLK", "FRA", "FRO", "FSM", "GAB", "GBR",
       	         "GEO", "GGY", "GHA", "GIB", "GIN", "GLP", "GMB", "GNB", "GNQ", "GRC", "GRD", "GRL", "GTM", "GUF", "GUM", "GUY",
       	         "HKG", "HMD", "HND", "HRV", "HTI", "HUN", "IDN", "IMN", "IND", "IOT", "IRL", "IRN", "IRQ", "ISL", "ISR", "ITA",
       	         "JAM", "JEY", "JOR", "JPN", "KAZ", "KEN", "KGZ", "KHM", "KIR", "KNA", "KOR", "KWT", "LAO", "LBN", "LBR", "LBY",
       	         "LCA", "LIE", "LKA", "LSO", "LTU", "LUX", "LVA", "MAC", "MAF", "MAR", "MCO", "MDA", "MDG", "MDV", "MEX", "MHL",
       	         "MKD", "MLI", "MLT", "MMR", "MNE", "MNG", "MNP", "MOZ", "MRT", "MSR", "MTQ", "MUS", "MWI", "MYS", "MYT", "NAM",
       	         "NCL", "NER", "NFK", "NGA", "NIC", "NIU", "NLD", "NOR", "NPL", "NRU", "NZL", "OMN", "PAK", "PAN", "PCN", "PER",
       	         "PHL", "PLW", "PNG", "POL", "PRI", "PRK", "PRT", "PRY", "PSE", "PYF", "QAT", "REU", "ROU", "RUS", "RWA", "SAU",
       	         "SDN", "SEN", "SGP", "SGS", "SHN", "SJM", "SLB", "SLE", "SLV", "SMR", "SOM", "SPM", "SRB", "SSD", "STP", "SUR",
       	         "SVK", "SVN", "SWE", "SWZ", "SXM", "SYC", "SYR", "TCA", "TCD", "TGO", "THA", "TJK", "TKL", "TKM", "TLS", "TON",
       	         "TTO", "TUN", "TUR", "TUV", "TWN", "TZA", "UGA", "UKR", "UMI", "URY", "USA", "UZB", "VAT", "VCT", "VEN", "VGB",
       	         "VIR", "VNM", "VUT", "WLF", "WSM", "YEM", "ZAF", "ZMB", "ZWE"};

        	@Override
    	    public void run() 
        	{
    	        try 
    	        {
    	        	ArrayList<VectorObject> vecObjs = new ArrayList<VectorObject>();

    	        	// Load each of the country files
    	        	for (String country: countries)
    	            {
    	            	String fileName = country.concat(".geojson");
    	            	String json = readGeoJSON(fileName);
    	            	if (json != null)
    	            	{
    	                	// Create a vector object
    	                	VectorObject vecObj = new VectorObject();
    	                	vecObj.fromGeoJSON(json);
    	                	vecObjs.add(vecObj);
    	            	}
    	            }
    	        	
    	        	// Style information for the vectors
    	        	VectorInfo vecInfo = new VectorInfo();
    	        	vecInfo.setColor(1.f, 0.f, 0.f, 1.f);
    	        	
    	        	// Add the vectors all at once
    	        	mapControl.addVectors(vecObjs,vecInfo);
    	        	
    	        	// Image for a marker
    	        	String bitmapName = "Star.png";
    	        	Bitmap bitmap = readBitmap(bitmapName);
    	        	if (bitmap != null)
    	        	{
    	        		NamedBitmap namedBitmap = new NamedBitmap(bitmapName,bitmap);
    	        		
        	        	// And a marker
        	        	ScreenMarker testMarker = new ScreenMarker();
        	        	testMarker.loc = Point2d.FromDegrees(-122, 37);
        	        	testMarker.color = Color.argb(255, 255, 0, 255);
        	        	testMarker.size = new Point2d(24,24);
        	        	testMarker.image = namedBitmap;
        	        	ArrayList<ScreenMarker> markers = new ArrayList<ScreenMarker>();
        	        	markers.add(testMarker);
        	        	
        	        	MarkerInfo markerInfo = new MarkerInfo();
        	        	
        	        	mapControl.addScreenMarkers(markers,markerInfo);
    	        	}    	        	
    	        	
    	        	// Draw some text
    	        	ScreenLabel testLabel = new ScreenLabel();
    	        	testLabel.loc = Point2d.FromDegrees(-118, 35);
    	        	testLabel.text = "Test Text";
    	        	ArrayList<ScreenLabel> labels = new ArrayList<ScreenLabel>();
    	        	labels.add(testLabel);
    	        	LabelInfo labelInfo = new LabelInfo();
    	        	labelInfo.fontSize = 24.f;
    	        	mapControl.addScreenLabels(labels,labelInfo);
    	        	
    	        } catch (Exception e) 
    	        {
    	            e.printStackTrace();
    	        }
    	    }
    	};
    	thread.start();
    	
        super.onCreate(savedInstanceState);
    }
    
    // Read a Bitmap from a file
    public Bitmap readBitmap(String fileName)
    {
    	// Load a geoJSON file
    	AssetManager am = getAssets();
    	try {
			InputStream is = am.open(fileName);
			if (is != null)
				return BitmapFactory.decodeStream(is);
		} catch (IOException e) {
			// TODO Auto-generated catch block
			return null;
		}    	
    	
    	return null;
    }
    
    // Read a GeoJSON file.  Return the string or null.
    public String readGeoJSON(String fileName)
    {
    	String str = null;
    	
    	// Load a geoJSON file
    	AssetManager am = getAssets();
    	try {
			InputStream is = am.open(fileName);
			if (is != null)
			{
				StringBuilder buf = new StringBuilder();
				BufferedReader in = new BufferedReader(new InputStreamReader(is));
				
				String strl = null;
				while ((strl=in.readLine()) != null) {
					buf.append(strl);
				}
				in.close();
				str = buf.toString();
			}
		} catch (IOException e) {
			return null;
		}
    	
    	if (str == null || str.isEmpty())
    		return null;
    	
    	return str;
    }


    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
//        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    } 
}
