package com.mousebirdconsulting.autotester.Framework;


import android.app.Activity;
import android.os.AsyncTask;
import android.view.View;

import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MapController;
import com.mousebirdconsulting.autotester.ConfigOptions;
import com.mousebirdconsulting.autotester.R;

public class MaplyTestCase extends AsyncTask<Void, Void, Void> {

	public interface MaplyTestCaseListener {
		void onFinish(MaplyTestResult resultMap, MaplyTestResult resultGlobe);

		void onExecute(View view);
	}

	private String testName;
	private boolean selected;
	private int icon = R.drawable.ic_action_selectall;
	private ConfigOptions.TestType options;
	private Activity activity;
	private GlobeController globeController;
	private MapController mapController;
	private Integer delay = 3;
	private MaplyTestResult globeResult;
	private MaplyTestResult mapResult;
	private MaplyTestCaseListener listener;
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

	public synchronized void runGlobeTest() {
		//create and prepare the controller
		try {
			if (listener != null) {
				activity.runOnUiThread(new Runnable() {
					@Override
					public void run() {
						listener.onExecute(globeController.getContentView());
					}
				});
			}

			if (setUpWithGlobe(globeController)) {
				if (ConfigOptions.getViewSetting(activity.getApplicationContext()) == ConfigOptions.ViewMapOption.ViewMap) {
					wait(delay * 1000);
				}
				globeResult = new MaplyTestResult(testName + " Globe Test");
			}
		} catch (Exception ex) {
			globeResult = new MaplyTestResult(testName + " Globe Test", ex);
		} finally {
			tearDownWithGlobe(globeController);
		}
	}

	public synchronized void runMapTest() {
		try {
			if (listener != null) {
				activity.runOnUiThread(new Runnable() {
					@Override
					public void run() {
						listener.onExecute(mapController.getContentView());
					}
				});
			}

			if (setUpWithMap(mapController)) {
				if (ConfigOptions.getViewSetting(activity.getApplicationContext()) == ConfigOptions.ViewMapOption.ViewMap) {
					wait(delay * 1000);
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

}
