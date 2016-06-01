package com.mousebirdconsulting.autotester.Framework;

import android.content.Context;
import android.os.AsyncTask;

import com.mousebirdconsulting.autotester.ConfigOptions;

import java.util.ArrayList;


public class MaplyDownloadManager extends AsyncTask<Void, Void, Void> {

	protected ArrayList<MaplyTestCase> testCases;
	protected MaplyDownloadManagerListener listener;
	protected Context context;

	public interface MaplyDownloadManagerListener {
		void onFinish();
	}

	public MaplyDownloadManager(Context context, ArrayList<MaplyTestCase> testCases, MaplyDownloadManagerListener listener) {
		this.testCases = testCases;
		this.listener = listener;
		this.context = context;
	}

	@Override
	protected void onPreExecute() {
		super.onPreExecute();
	}

	@Override
	protected Void doInBackground(Void... params) {
		for (MaplyTestCase testCase: this.testCases) {
			if (testCase.areResourcesDownloaded()) {
				// already downloaded by previous tests
				ConfigOptions.setTestState(context, testCase.getTestName(),  ConfigOptions.TestState.Ready);
			} else {
				ConfigOptions.setTestState(context, testCase.getTestName(), ConfigOptions.TestState.Downloading);

				testCase.downloadResources();

				if (ConfigOptions.getTestState(this.context, testCase.getTestName()) != ConfigOptions.TestState.Error) {
					ConfigOptions.setTestState(context, testCase.getTestName(),  ConfigOptions.TestState.Ready);
				}
			}
		}
		return null;
	}

	@Override
	protected void onPostExecute(Void aVoid) {
		super.onPostExecute(aVoid);
		if (listener != null){
			listener.onFinish();
		}
	}
}
