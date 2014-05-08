package com.mousebird.maplytester;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;

import com.mousebird.maply.LabelInfo;
import com.mousebird.maply.MaplyController;
import com.mousebird.maply.MarkerInfo;
import com.mousebird.maply.NamedBitmap;
import com.mousebird.maply.Point2d;
import com.mousebird.maply.Point3d;
import com.mousebird.maply.ScreenLabel;
import com.mousebird.maply.ScreenMarker;
import com.mousebird.maply.VectorInfo;
import com.mousebird.maply.VectorObject;

import android.app.Activity;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;

/**
 * Test Maply by loading every country outline we have and tossing in a couple other objects.
 * Works on its own thread.
 */
public class TestCountries 
{
	Activity activity = null;
	MaplyController mapControl = null;
	
	TestCountries(Activity inActivity,MaplyController inMapControl)
	{
		activity = inActivity;
		mapControl = inMapControl;
	}
	
	void start()
	{
//	   	mapControl.setPosition(-122.416667 / 180.0 * 3.1415, 37.783333 / 180.0 * 3.1415, 4.0) ;

//		Point3d testPt = new Point3d(0.f,0.f,0.f);

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

    	        	// Style information for the vectors
    	        	VectorInfo vecInfo = new VectorInfo();
    	        	vecInfo.setColor(0.f, 0.f, 0.f, 1.f);
    	        	vecInfo.setFade(0.5f);

					// Load each of the country files
    	        	for (String country: countries)
    	            {
    	            	String fileName = country.concat(".geojson");
    	            	String json = readGeoJSON(fileName);
        	        	ArrayList<VectorObject> vecObjs = new ArrayList<VectorObject>();
    	            	if (json != null)
    	            	{
    	                	// Create a vector object
    	                	VectorObject vecObj = new VectorObject();
    	                	vecObj.fromGeoJSON(json);
    	                	vecObjs.add(vecObj);
    	            	}
    	            	
    	            	if (vecObjs.size() > 0)
    	            		mapControl.addVectors(vecObjs,vecInfo);
    	            }
    	        	    	        	    	        	
    	        	// Image for a marker
    	        	String bitmapName = "Star.png";
    	        	Bitmap bitmap = readBitmap(bitmapName);
    	        	if (bitmap != null)
    	        	{
    	        		NamedBitmap namedBitmap = new NamedBitmap(bitmapName,bitmap);
    	        		
        	        	// And a marker
        	        	ScreenMarker testMarker = new ScreenMarker();
        	        	testMarker.loc = Point2d.FromDegrees(-122, 37);
        	        	testMarker.size = new Point2d(32,32);
        	        	testMarker.image = namedBitmap;
        	        	ArrayList<ScreenMarker> markers = new ArrayList<ScreenMarker>();
        	        	markers.add(testMarker);
        	        	
        	        	MarkerInfo markerInfo = new MarkerInfo();
        	        	markerInfo.setFade(0.5f);
        	        	
//        	        	mapControl.addScreenMarkers(markers,markerInfo);
    	        	}    	        	
    	        	
    	        	// Draw some text
    	        	ScreenLabel testLabel = new ScreenLabel();
    	        	testLabel.loc = Point2d.FromDegrees(-118, 35);
    	        	testLabel.text = "Test Text";
    	        	ArrayList<ScreenLabel> labels = new ArrayList<ScreenLabel>();
    	        	labels.add(testLabel);
    	        	LabelInfo labelInfo = new LabelInfo();
    	        	labelInfo.setFontSize(24.f);
//    	        	mapControl.addScreenLabels(labels,labelInfo);
    	        	
    	        } catch (Exception e) 
    	        {
    	            e.printStackTrace();
    	        }
    	    }
    	};
    	thread.start();		
	}

    // Read a GeoJSON file.  Return the string or null.
    public String readGeoJSON(String fileName)
    {
    	String str = null;
    	
    	// Load a geoJSON file
    	AssetManager am = activity.getAssets();
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
    
    // Read a Bitmap from a file
    public Bitmap readBitmap(String fileName)
    {
    	// Load a geoJSON file
    	AssetManager am = activity.getAssets();
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
}
