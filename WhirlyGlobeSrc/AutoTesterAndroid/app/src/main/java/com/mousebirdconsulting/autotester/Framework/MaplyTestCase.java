package com.mousebirdconsulting.autotester.Framework;


import android.app.Activity;
import android.graphics.Color;
import android.os.AsyncTask;
import android.view.View;

import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MapController;
import com.mousebird.maply.MaplyBaseController;
import com.mousebirdconsulting.autotester.ConfigOptions;
import com.mousebirdconsulting.autotester.R;

public class MaplyTestCase extends AsyncTask<Void, View, Void> {

	public interface MaplyTestCaseListener {
		void onStart(View view);

		void onExecute(View view);

		void onFinish(MaplyTestResult resultMap, MaplyTestResult resultGlobe);
	}

	protected String testName;
	protected boolean selected;
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

	public MaplyTestCase(Activity activity) {
		super();

		this.activity = activity;
	}

	// Change this to set transparent backgrounds
	int clearColor = Color.BLACK;

	// Set if we successfully set up the controller
	boolean success = false;

	// Create the test case and start it
	public void start()
	{
		if (options == ConfigOptions.TestType.BothTest || options == ConfigOptions.TestType.GlobeTest) {
			GlobeController.Settings settings = new GlobeController.Settings();
			// Note: Turn this off for testing GLTextureView
//			settings.useSurfaceView = false;
			settings.clearColor = clearColor;
			globeController = new GlobeController(activity,settings);
			controller = globeController;
		}
		if (options == ConfigOptions.TestType.BothTest || options == ConfigOptions.TestType.MapTest) {
			MapController.Settings settings = new MapController.Settings();
			settings.clearColor = clearColor;
			mapController = new MapController(activity,settings);
			controller = mapController;
		}
		success = true;

		controller.addPostSurfaceRunnable(new Runnable() {
			@Override
			public void run() {
				if (mapController != null) {
					try {
						setUpWithMap(mapController);
					} catch (Exception ex) {
						mapResult = new MaplyTestResult(testName + " Map Test");
						success = false;
					}
				}
				if (globeController != null) {
					try {
						setUpWithGlobe(globeController);
					} catch (Exception ex) {
						globeResult = new MaplyTestResult(testName + " Globe Test", ex);
						success = false;
					}
				}
				execute();
			}
		});

		listener.onStart(controller.getContentView());
	}

	@Override
	protected void onPreExecute() {
	}

	@Override
	protected Void doInBackground(Void... params) {
		if (success)
		{
			publishProgress(controller.getContentView());
			if (ConfigOptions.getViewSetting(activity.getApplicationContext()) == ConfigOptions.ViewMapOption.ViewMap) {
				try {
					Thread.sleep(delay * 1000);
				}
				catch (Exception ex) {
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

	@Override
	protected void onProgressUpdate(View... values) {
		if (listener != null) {
			listener.onExecute(values[0]);
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

	public boolean isSelected() {
		return selected;
	}

	public void setSelected(boolean value) {
		selected = value;
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

}
