package com.mousebirdconsulting.maplytester;

import java.io.*;

import android.os.*;
import android.app.*;
import android.content.Context;
import android.content.pm.*;
import android.content.res.AssetManager;
import android.view.Menu;
import android.widget.Toast;
import android.opengl.*;
import com.mousebirdconsulting.maply.*;

public class MainActivity extends Activity {

	private GLSurfaceView glSurfaceView;
	private RendererWrapper rendererWrapper;
	private Handler mHandler = new Handler();
	VectorObject vecObj = null;
	
    @Override
    protected void onCreate(Bundle savedInstanceState) 
    {    	
		// Wait for the debugger to catch up
    	try {
			Thread.sleep(5000);
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

    	// Note: This is a little dumb
    	MaplyInitialize.Init();
    	
    	// Load a vector file
    	String canJSON = readGeoJSON("USA.geojson");
    	if (canJSON != null)
    	{
        	// Create a vector object
        	vecObj = new VectorObject();
        	vecObj.fromGeoJSON(canJSON);
    	}
    	
        super.onCreate(savedInstanceState);
        
        ActivityManager activityManager = (ActivityManager) getSystemService(Context.ACTIVITY_SERVICE);
        ConfigurationInfo configurationInfo = activityManager.getDeviceConfigurationInfo();
        
        final boolean supportsEs2 = configurationInfo.reqGlEsVersion > 0x20000 || isProbablyEmulator();
        if (supportsEs2)
        {
        	glSurfaceView = new GLSurfaceView(this);
        	
        	if (isProbablyEmulator())
        	{
        		glSurfaceView.setEGLConfigChooser(8, 8, 8, 8, 16, 0);
        	}
        	
        	glSurfaceView.setEGLContextClientVersion(2);
        	rendererWrapper = new RendererWrapper();
        	glSurfaceView.setRenderer(rendererWrapper);
        	setContentView(glSurfaceView);        
        	
        	// Wait for everything to be set up before we kick off the vector add
        	mHandler.postDelayed(new Runnable() 
        		{
        			public void run() {
        	        	// Add the vector file
        	        	if (vecObj != null)
        	        	{
        	        		rendererWrapper.vecManager.addVector(vecObj);
        	        		
        	        		rendererWrapper.mapView.setLoc(-122.416667 / 180.0 * 3.1415, 37.783333 / 180.0 * 3.1415, 4.0) ;
        	        	}
        			}
        		},1000);
        
        } else {
        	Toast.makeText(this,  "This devices does not support OpenGL ES 2.0.", Toast.LENGTH_LONG).show();
        	return;
        }
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
			// TODO Auto-generated catch block
			return null;
		}
    	
    	if (str == null || str.isEmpty())
    		return null;
    	
    	return str;
    }


    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }
 
    private boolean isProbablyEmulator() {
        return Build.VERSION.SDK_INT >= Build.VERSION_CODES.ICE_CREAM_SANDWICH_MR1
                && (Build.FINGERPRINT.startsWith("generic")
                        || Build.FINGERPRINT.startsWith("unknown")
                        || Build.MODEL.contains("google_sdk")
                        || Build.MODEL.contains("Emulator")
                        || Build.MODEL.contains("Android SDK built for x86"));
    }        
}
