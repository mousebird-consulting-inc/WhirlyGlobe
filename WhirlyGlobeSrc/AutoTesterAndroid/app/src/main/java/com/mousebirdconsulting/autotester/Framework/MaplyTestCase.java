package com.mousebirdconsulting.autotester.Framework;


import android.app.Activity;
import android.graphics.Color;
import android.os.AsyncTask;
import android.util.Log;
import android.view.View;

import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MapController;
import com.mousebird.maply.MaplyBaseController;
import com.mousebirdconsulting.autotester.ConfigOptions;
import com.mousebirdconsulting.autotester.R;

import java.io.DataInputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.net.URL;
import java.util.ArrayList;

public class MaplyTestCase extends AsyncTask<Void, View, Void> {

	public enum TestExecutionImplementation {
		Globe, Map, Both, None;
	}

	public interface MaplyTestCaseListener {
		void onStart(View view);

		void onExecute(View view);

		void onFinish(MaplyTestResult resultMap, MaplyTestResult resultGlobe);
	}

	protected String testName;
	protected int icon = R.drawable.ic_action_selectall;
	protected ConfigOptions.TestType options;
	protected Activity activity;
	protected MaplyBaseController controller;
	protected GlobeController globeController;
	protected MapController mapController;
	protected Integer delay = 3;
	protected MaplyTestResult globeResult;
	protected MaplyTestResult mapResult;
	protected MaplyTestCaseListener listener;
	protected TestExecutionImplementation implementation = TestExecutionImplementation.None;
	protected ArrayList<String> remoteResources = new ArrayList<>();

	public MaplyTestCase(Activity activity) {
		super();
		this.activity = activity;
	}

	public boolean areResourcesDownloaded(){
		for (int ii = 0; ii < remoteResources.size(); ii++){
			File file = new File(ConfigOptions.getCacheDir(activity) + "/" + getFileName(remoteResources.get(ii)));
			if (!file.exists()){
				return false;
			}
		}
		return true;
	}

	private String getFileName(String url){
		String[] tokens = url.split("/");
		return tokens[tokens.length-1];
	}

	public void downloadResources() {
		for (String resourcePath : this.remoteResources) {
			downloadFromServer(resourcePath);
		}
	}

	private void downloadFromServer(String url) {
		String filename = ConfigOptions.getCacheDir(activity).getPath() + "/" + getFileName(url);
		Log.e("Download", "Downloading " + url);

		try {
			FileOutputStream fos = new FileOutputStream(filename);
			DataInputStream dis = new DataInputStream(new URL(url).openStream());
			try {
				byte[] buffer = new byte[1024];
				int length;

				while ((length = dis.read(buffer)) > 0) {
					fos.write(buffer, 0, length);
				}
			} finally {
				try {
					dis.close();
					fos.close();
				} catch (IOException e) {
					Log.e("Download", "Error downloading " + url, e);
					ConfigOptions.setTestState(getActivity(), getTestName(), ConfigOptions.TestState.Error);
					deleteFile(getFileName(url));
				}
			}
			Log.e("Download", "Download stored in " + filename);
		} catch (Exception e) {
			Log.e("Download", "Error downloading " + url, e);
			ConfigOptions.setTestState(getActivity(), getTestName(), ConfigOptions.TestState.Error);
			deleteFile(getFileName(url));
		}
	}

	private void deleteFile(String name){

		File file = new File(ConfigOptions.getCacheDir(activity) + "/" + name);
		file.delete();
	}
	// Change this to set transparent backgrounds
	int clearColor = Color.BLACK;

	// Set if we successfully set up the controller
	boolean success = false;

	// Create the test case and start it
	public void start()
	{
		if (ConfigOptions.getTestState(getActivity(), getTestName()) == ConfigOptions.TestState.Error || ConfigOptions.getTestState(getActivity(), getTestName()) == ConfigOptions.TestState.Downloading)
			return;

		if (options.isGlobe()) {
			GlobeController.Settings settings = new GlobeController.Settings();
			// Note: Turn this off for testing GLTextureView
			settings.useSurfaceView = false;
			settings.clearColor = clearColor;
			globeController = new GlobeController(activity,settings);
			controller = globeController;
		}
		if (options.isMap()) {
			MapController.Settings settings = new MapController.Settings();
			settings.clearColor = clearColor;
			mapController = new MapController(activity,settings);
			controller = mapController;
		}
		success = true;
		listener.onStart(controller.getContentView());
		if (ConfigOptions.getViewSetting(getActivity().getApplicationContext()) == ConfigOptions.ViewMapOption.ViewMap){
			controller.addPostSurfaceRunnable(new Runnable() {
				@Override
				public void run() {
					implementationTest();
				}
			});
		}
		else{
			implementationTest();
		}
	}

	private void implementationTest() {
		if (mapController != null) {
			try {
				if (setUpWithMap(mapController)) {
					mapResult = new MaplyTestResult(testName + " Map Test");
				}
			} catch (Exception ex) {
				mapResult = new MaplyTestResult(testName + " Map Test", ex);
				success = false;
			}
		}
		if (globeController != null) {
			try {
				if (setUpWithGlobe(globeController)) {
					globeResult = new MaplyTestResult(testName + " Globe Test");
				}
			} catch (Exception ex) {
				globeResult = new MaplyTestResult(testName + " Globe Test", ex);
				success = false;
			}
		}
		execute();
	}

	@Override
	protected void onPreExecute() {
	}

	@Override
	protected Void doInBackground(Void... params) {
		if (success) {
			publishProgress(controller.getContentView());
			if (ConfigOptions.getExecutionMode(activity.getApplicationContext()) == ConfigOptions.ExecutionMode.Interactive) {
				while (true) {
					try {
						Thread.sleep(500);
					} catch (InterruptedException e) {
					}
				}
			}
			if (ConfigOptions.getViewSetting(activity.getApplicationContext()) == ConfigOptions.ViewMapOption.ViewMap) {
				if (this.mapController != null && listener != null) {
					try {
						Thread.sleep(delay * 1000);
					}
					catch (InterruptedException ex) {
					}
				}
				if (this.globeController != null && listener != null) {
					if (options == ConfigOptions.TestType.BothTest) {
						activity.runOnUiThread(new Runnable() {
							@Override
							public void run() {
								listener.onExecute(globeController.getContentView());
							}
						});
					}
					try {
						Thread.sleep(delay * 1000);
					}
					catch (InterruptedException ex) {
					}
				}
			}
		}

		return null;
	}

	@Override
	protected void onPostExecute(Void aVoid) {
		if (listener != null) {
			listener.onFinish(this.mapResult, this.globeResult);
		}
	}

	public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
		return false;
	}

	public void tearDownWithGlobe(GlobeController globeVC) {
	}

	public boolean setUpWithMap(MapController mapVC) throws Exception {
		return false;
	}

	public void tearDownWithMap(MapController mapVC) {
	}

	public void setListener(MaplyTestCaseListener resultTestListener) {
		this.listener = resultTestListener;
	}

	public void setOptions(ConfigOptions.TestType value) {
		options = value;
	}

	public int getIcon() {
		return icon;
	}

	public void setIcon(int value) {
		icon = value;
	}

	public Activity getActivity() {
		return activity;
	}

	public void setActivity(Activity value) {
		activity = value;
	}

	public String getTestName() {
		return testName;
	}

	public void setTestName(String value) {
		testName = value;
	}

	public void setDelay(int value) {
		delay = value;
	}

	public int getDelay() {
		return delay;
	}

	public TestExecutionImplementation getImplementation() {
		return implementation;
	}

}
