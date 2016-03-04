package com.mousebirdconsulting.autotester.Framework;


import android.app.Activity;
import android.os.AsyncTask;
import android.view.View;

import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MapController;
import com.mousebirdconsulting.autotester.ConfigOptions;
import com.mousebirdconsulting.autotester.R;

public class MaplyTestCase extends AsyncTask<Void, View, Void> {

	public interface MaplyTestCaseListener {
		void onFinish(MaplyTestResult resultMap, MaplyTestResult resultGlobe);

		void onExecute(View view);
	}

	protected String testName;
	protected boolean selected;
	protected int icon = R.drawable.ic_action_selectall;
	protected ConfigOptions.TestType options;
	protected Activity activity;
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

	@Override
	protected void onPreExecute() {
		if (options == ConfigOptions.TestType.BothTest || options == ConfigOptions.TestType.GlobeTest) {
			globeController = new GlobeController(activity);
		}
		if (options == ConfigOptions.TestType.BothTest || options == ConfigOptions.TestType.MapTest) {
			mapController = new MapController(activity);
		}
	}

	@Override
	protected Void doInBackground(Void... params) {
		if (options == ConfigOptions.TestType.BothTest || options == ConfigOptions.TestType.MapTest) {
			runMapTest();
		}
		if (options == ConfigOptions.TestType.BothTest || options == ConfigOptions.TestType.GlobeTest) {
			runGlobeTest();
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

	public void runGlobeTest() {
		//create and prepare the controller
		try {
			publishProgress(globeController.getContentView());
			if (setUpWithGlobe(globeController)) {
				if (ConfigOptions.getViewSetting(activity.getApplicationContext()) == ConfigOptions.ViewMapOption.ViewMap) {
					Thread.sleep(delay * 1000);
				}
				globeResult = new MaplyTestResult(testName + " Globe Test");
			}
		} catch (Exception ex) {
			globeResult = new MaplyTestResult(testName + " Globe Test", ex);
		} finally {
			tearDownWithGlobe(globeController);
		}
	}

	public void runMapTest() {
		try {

			publishProgress(mapController.getContentView());
			if (setUpWithMap(mapController)) {
				if (ConfigOptions.getViewSetting(activity.getApplicationContext()) == ConfigOptions.ViewMapOption.ViewMap) {
					Thread.sleep(delay * 1000);
				}
				mapResult = new MaplyTestResult(testName + " Map Test");
			}
		} catch (Exception ex) {
			mapResult = new MaplyTestResult(testName + " Map Test", ex);
		} finally {
			tearDownWithMap(mapController);
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
