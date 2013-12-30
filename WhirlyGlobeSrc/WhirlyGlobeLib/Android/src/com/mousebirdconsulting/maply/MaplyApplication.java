package com.mousebirdconsulting.maply;

import android.app.Application;

public class MaplyApplication extends Application 
{
	public void onCreate()
	{
		System.loadLibrary("Maply");
	}
}
