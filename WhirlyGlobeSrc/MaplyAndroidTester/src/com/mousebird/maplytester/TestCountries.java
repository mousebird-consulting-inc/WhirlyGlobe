/*
 *  TestCountries.java
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

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;

import com.mousebird.maply.LabelInfo;
import com.mousebird.maply.MapController;
import com.mousebird.maply.Point2d;
import com.mousebird.maply.ScreenLabel;
import com.mousebird.maply.VectorInfo;
import com.mousebird.maply.VectorObject;

import android.app.Activity;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Typeface;

/**
 * Test Maply by loading every country outline we have and tossing in a couple other objects.
 * Works on its own thread.
 */
public class TestCountries 
{
	Activity activity = null;
	MapController mapControl = null;
	
	TestCountries(Activity inActivity,MapController inMapControl)
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

    	        	// Style information for the labels
    	        	LabelInfo labelInfo = new LabelInfo();
    	        	labelInfo.setTextColor(0.f, 0.f, 0.f, 1.f);
    	        	labelInfo.setBackgroundColor(0.f, 0.f, 0.f, 0.f);
    	        	labelInfo.setTypeface(Typeface.defaultFromStyle(Typeface.BOLD));
    	        	ArrayList<ScreenLabel> labels = new ArrayList<ScreenLabel>();

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
    	                	
    	                	// Also do a label in the middle
    	                	String name = vecObj.getAttributes().getString("ADMIN");
    	                	Point2d ll = new Point2d();
    	                	Point2d ur = new Point2d();
    	                	Point2d center = vecObj.largestLoopCenter(ll, ur);
    	                	if (name != null && center != null)
    	                	{
    	                		ScreenLabel label = new ScreenLabel();
    	                		label.loc = center;
    	                		label.text = name;
    	                		labels.add(label);
    	                	}
    	            	}
    	            	
    	            	if (vecObjs.size() > 0)
    	            		mapControl.addVectors(vecObjs,vecInfo,MapController.ThreadMode.ThreadAny);
    	            }
    	        	    	        	    	        	    	        	
    	        	mapControl.addScreenLabels(labels,labelInfo,MapController.ThreadMode.ThreadAny);
    	        	
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
