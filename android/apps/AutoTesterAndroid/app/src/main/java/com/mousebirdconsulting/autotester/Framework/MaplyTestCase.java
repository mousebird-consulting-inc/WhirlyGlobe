package com.mousebirdconsulting.autotester.Framework;

import android.app.Activity;
import android.content.Context;
import android.content.ContextWrapper;
import android.content.res.Resources;
import android.graphics.Color;
import android.os.AsyncTask;
import android.os.Handler;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Display;
import android.view.View;
import android.view.WindowManager;

import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MapController;
import com.mousebird.maply.BaseController;
import com.mousebird.maply.Mbr;
import com.mousebird.maply.Point2d;
import com.mousebird.maply.Point3d;
import com.mousebird.maply.SelectedObject;
import com.mousebirdconsulting.autotester.ConfigOptions;
import com.mousebirdconsulting.autotester.R;

import java.io.InputStream;
import java.io.OutputStream;
import java.lang.ref.WeakReference;
import java.io.DataInputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.net.URL;
import java.util.ArrayList;

public class MaplyTestCase extends AsyncTask<Void, View, Void> implements GlobeController.GestureDelegate, MapController.GestureDelegate {

	public enum TestExecutionImplementation { Globe, Map, Both, None }

	public interface MaplyTestCaseListener {
		void onStart(View view);

		void onExecute(View view);

		void onFinish(MaplyTestResult resultMap, MaplyTestResult resultGlobe);
	}

	protected String testName;
	protected int icon = R.drawable.ic_action_selectall;
	protected ConfigOptions.TestType options;
	protected WeakReference<Activity> weakActivity;
	protected BaseController controller;
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
		weakActivity = new WeakReference<>(activity);
	}

	public MaplyTestCase(Activity activity, String testName) {
		this(activity, testName, TestExecutionImplementation.Both);
	}

	public MaplyTestCase(Activity activity, String testName, TestExecutionImplementation impl) {
		this(activity);
		this.testName = testName;
		this.implementation = impl;
	}

	public boolean areResourcesDownloaded(){
		for (int ii = 0; ii < remoteResources.size(); ii++){
			File file = new File(ConfigOptions.getCacheDir(getActivity()) + "/" + getFileName(remoteResources.get(ii)));
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
		String filename = ConfigOptions.getCacheDir(getActivity()).getPath() + "/" + getFileName(url);
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

	private Boolean deleteFile(String name){

		File file = new File(ConfigOptions.getCacheDir(getActivity()) + "/" + name);
		return file.delete();
	}
	// Change this to set transparent backgrounds
	int clearColor = Color.BLACK;

	// Set if we successfully set up the controller
	boolean success = false;

	// Build the globe controller for use later
	// This can be overridden if we're doing something tricky
	protected GlobeController makeGlobeController()
	{
		GlobeController.Settings settings = new GlobeController.Settings();
		// Note: Turn this off for testing GLTextureView
//		settings.useSurfaceView = false;
		settings.clearColor = clearColor;
//		settings.width = 512;
//		settings.height = 512;
		GlobeController globeControl = new GlobeController(getActivity(),settings);
		globeControl.gestureDelegate = this;

		return globeControl;
	}

	// Build the map controller for use later
	// This can be overridden if we're doing something tricky
	protected MapController makeMapController()
	{
		MapController.Settings settings = new MapController.Settings();
		settings.clearColor = clearColor;
//		settings.width = 512;
//		settings.height = 512;
		MapController mapControl = new MapController(getActivity(),settings);
		mapControl.gestureDelegate = this;

		return mapControl;
	}

	/**
	 * Find the display DPI
	 */
	public Point2d getDisplayDensity() {
		Activity activity = getActivity();
		WindowManager windowManager = (activity != null) ? activity.getWindowManager() : null;
		Display display = (windowManager != null) ? windowManager.getDefaultDisplay() : null;
		if (display != null) {
			DisplayMetrics dm = new DisplayMetrics();
			display.getRealMetrics(dm);
			return new Point2d(dm.xdpi, dm.ydpi);
		}
		Resources resources = (activity != null) ? activity.getResources() : null;
		if (resources != null) {
			DisplayMetrics dm = resources.getDisplayMetrics();
			if (dm != null) {
				return new Point2d(dm.xdpi, dm.ydpi);
			}
		}
		return null;
	}

	int numRuns = 0;
	// If set we'll run startup/shutdown tests
	boolean multiTest = false;
	long runDelay = 2000;

	// Run start/shutdown in a loop
	public void start()
	{
		if (multiTest) {
			startControl();

			Handler handler = new Handler();
			handler.postDelayed(() -> {
				shutdown();

				Handler handler1 = new Handler();
				handler1.postDelayed(this::start, runDelay);
			}, runDelay);
		} else {
			startControl();
		}
	}


	// Create the test case and start it
	public void startControl()
	{
		numRuns++;
		if (multiTest)
			Log.d("Maply","Run " + numRuns + " times.");

		if (ConfigOptions.getTestState(getActivity(), getTestName()) == ConfigOptions.TestState.Error || ConfigOptions.getTestState(getActivity(), getTestName()) == ConfigOptions.TestState.Downloading)
			return;

		if (options.isGlobe()) {
			globeController = makeGlobeController();
			controller = globeController;
		}
		if (options.isMap()) {
			mapController = makeMapController();
			controller = mapController;
		}
		success = true;
		listener.onStart(controller.getContentView());
		controller.addPostSurfaceRunnable(this::implementationTest);
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

		if (!multiTest)
			execute();
	}

	@Override
	protected void onPreExecute() {
	}

	@Override
	protected Void doInBackground(Void... params) {
		if (success) {
			publishProgress(controller.getContentView());
			if (ConfigOptions.getExecutionMode(getActivity().getApplicationContext()) == ConfigOptions.ExecutionMode.Interactive) {
				//noinspection InfiniteLoopStatement
				while (true) {
					try {
						//noinspection BusyWait
						Thread.sleep(500);
					} catch (InterruptedException ignored) {
					}
				}
			}
			if (ConfigOptions.getViewSetting(getActivity().getApplicationContext()) == ConfigOptions.ViewMapOption.ViewMap) {
				if (this.mapController != null && listener != null) {
					try {
						Thread.sleep(delay * 1000);
					}
					catch (InterruptedException ignored) {
					}
				}
				if (this.globeController != null && listener != null) {
					if (options == ConfigOptions.TestType.BothTest) {
						getActivity().runOnUiThread(() -> listener.onExecute(globeController.getContentView()));
					}
					try {
						Thread.sleep(delay * 1000);
					}
					catch (InterruptedException ignored) {
					}
				}
			}
		}

		return null;
	}

	public void shutdown()
	{
		if (globeController != null)
			globeController.shutdown();
		globeController = null;
		if (mapController != null)
			mapController.shutdown();
		mapController = null;
		controller = null;
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
		return weakActivity.get();
	}

	public void setActivity(Activity value) {
		weakActivity = new WeakReference<>(value);
	}

	public String getTestName() {
		return testName;
	}

	public void setTestName(String value) {
		testName = value;
	}

	public final void setDelay(int value) {
		delay = value;
	}

	public int getDelay() { return delay; }

	public TestExecutionImplementation getImplementation() {
		return implementation;
	}

	/** Implementation of Globe Gesture Delegate
     */

	public void userDidSelect(GlobeController globeControl, SelectedObject[] selObjs, Point2d loc, Point2d screenLoc)
	{
		Log.d("Maply","Selected " + selObjs.length + " objects");
	}

	public void userDidTap(GlobeController globeControl,Point2d loc,Point2d screenLoc)
	{
		Log.d("Maply",  String.format("User tapped at screen(%f,%f) = geo(%f,%f)",
				screenLoc.getX(), screenLoc.getY(), loc.getX()*180/Math.PI, loc.getY()*180/Math.PI));
		//Point2d newScreenPt = globeControl.screenPointFromGeo(loc);
		//Point2d newGeo = globeControl.geoPointFromScreen(newScreenPt);
		Mbr mbr = globeControl.getCurrentViewGeo();
		if (mbr != null) {
			Log.d("Maply", "User is looking at bounding box: " + mbr);
		}
	}

	public void userDidTapOutside(GlobeController globeControl,Point2d screenLoc)
	{
		Log.d("Maply","User tapped outside globe.");
	}

	public void userDidLongPress(GlobeController globeControl, SelectedObject[] selObjs, Point2d loc, Point2d screenLoc)
	{}

	public void globeDidStartMoving(GlobeController globeControl, boolean userMotion)
	{}

	public void globeDidStopMoving(GlobeController globeControl, Point3d[] corners, boolean userMotion)
	{}

	public void globeDidMove(GlobeController globeControl,Point3d[] corners, boolean userMotion)
	{}

	public void mapDidStartMoving(MapController mapControl, boolean userMotion)
	{}

	public void mapDidStopMoving(MapController mapControl, Point3d[] corners, boolean userMotion)
	{

	}

	public void mapDidMove(MapController mapControl,Point3d[] corners, boolean userMotion)
	{

	}

	/** Implementation of Map Gesture Delegate
	 */

	public void userDidSelect(MapController mapControl,SelectedObject[] selObjs,Point2d loc,Point2d screenLoc)
	{
		Log.d("Maply","Selected object");
	}

	public void userDidTap(MapController mapControl,Point2d loc,Point2d screenLoc)
	{
		Log.d("Maply",  String.format("User tapped at screen(%f,%f) = geo(%f,%f)",
				screenLoc.getX(), screenLoc.getY(), loc.getX()*180/Math.PI, loc.getY()*180/Math.PI));
		//Point2d newScreenPt = mapControl.screenPointFromGeo(loc);
		//Point2d newGeo = mapControl.geoPointFromScreen(newScreenPt);
		Mbr mbr = mapControl.getCurrentViewGeo();
		if (mbr != null) {
			Log.d("Maply", "User is looking at bounding box: " + mbr);
		}
	}

	public void userDidLongPress(MapController mapController, SelectedObject[] selObjs, Point2d loc, Point2d screenLoc)
	{}

	/**
	 * Write a resource file to a local directory where it can be modified or opened for random-access.
	 * @param assetFile The name of the asset file
	 * @param localDirName The directory in which to place the file, created if necessary
	 * @param localFileName The name of the file to write
	 * @return A File representing the resulting file, or null
	 */
	protected File copyAssetFile(String assetFile, String localDirName, String localFileName) {
		return copyAssetFile(getActivity(), assetFile, localDirName, localFileName);
	}

	/**
	 * Write a resource file to a local directory where it can be modified or opened for random-access.
	 * @param assetFile The name of the asset file
	 * @param localDirName The directory in which to place the file, created if necessary
	 * @param localFileName The name of the file to write
	 * @return A File representing the resulting file, or null
	 */
	protected static File copyAssetFile(Activity activity, String assetFile, String localDirName, String localFileName) {
		if (activity == null) {
			return null;
		}

		ContextWrapper wrapper = new ContextWrapper(activity);
		File localDir =  wrapper.getDir(localDirName, Context.MODE_PRIVATE);
		if (localDir == null) {
			return null;
		}

		File outFile = new File(localDir, localFileName);
		if (outFile.exists()) {
			return outFile;
		}

		try (InputStream is = activity.getAssets().open(assetFile);
			 OutputStream os = new FileOutputStream(outFile)) {
			byte[] mBuffer = new byte[1024];
			for (int length; (length = is.read(mBuffer)) > 0; ) {
				os.write(mBuffer, 0, length);
			}
			return outFile;
		} catch (Exception ex) {
			Log.w("Maply", "Failed to extract asset " + assetFile);
			return null;
		}

	}

	protected static float radToDeg(float rad) { return (float)(rad * 180 / Math.PI); }
	protected static float degToRad(float deg) { return (float)(deg * Math.PI / 180); }
	protected static double radToDeg(double rad) { return rad * 180 / Math.PI; }
	protected static double degToRad(double deg) { return deg * Math.PI / 180; }
}
