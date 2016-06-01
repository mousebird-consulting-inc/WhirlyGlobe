package com.mousebirdconsulting.autotester;

import android.content.Context;
import android.content.SharedPreferences;

import com.mousebird.maply.StringWrapper;


public class ConfigOptions {

	public static final String seeView = "seeViewSetting";
	public static final String testType = "testTypeSetting";
	public static final String preferences = "AutoTesterAndroid";
	public static final String executionMode = "executionMode";
	public static final String testState = "testState";

	public enum ExecutionMode {
		Interactive, Multiple, Single;
	}

	public static final void setExecutionMode(Context context, ExecutionMode mode) {
		SharedPreferences preferences = context.getSharedPreferences(ConfigOptions.preferences, Context.MODE_PRIVATE);
		SharedPreferences.Editor editor = preferences.edit();
		editor.putString(ConfigOptions.executionMode, mode.name());
		editor.commit();
	}

	public static final ExecutionMode getExecutionMode(Context context) {
		SharedPreferences preferences = context.getSharedPreferences(ConfigOptions.preferences, Context.MODE_PRIVATE);
		String defaultValue = ExecutionMode.Interactive.name();
		String executionMode = preferences.getString(ConfigOptions.executionMode, defaultValue);
		return ExecutionMode.valueOf(executionMode);
	}

	public static final TestState getTestState(Context context, String testName) {
		SharedPreferences preferences = context.getSharedPreferences(ConfigOptions.preferences, Context.MODE_PRIVATE);
		String defaultValue = TestState.Downloading.name();
		String testState = preferences.getString(testName, defaultValue);
		return TestState.valueOf(testState);
	}

	public static final void setTestState(Context context, String testName, TestState state) {
		SharedPreferences preferences = context.getSharedPreferences(ConfigOptions.preferences, Context.MODE_PRIVATE);
		SharedPreferences.Editor editor = preferences.edit();
		editor.putString(testName, state.name());
		editor.commit();
	}

	public static final void setTestType(Context context, TestType testType) {
		SharedPreferences preferences = context.getSharedPreferences(ConfigOptions.preferences, Context.MODE_PRIVATE);
		SharedPreferences.Editor editor = preferences.edit();
		editor.putString(ConfigOptions.testType, testType.name());
		editor.commit();
	}

	public static final TestType getTestType(Context context) {
		SharedPreferences preferences = context.getSharedPreferences(ConfigOptions.preferences, Context.MODE_PRIVATE);
		String defaultValue = TestType.BothTest.name();
		String testType = preferences.getString(ConfigOptions.testType, defaultValue);
		return TestType.valueOf(testType);
	}

	public static final void setViewSetting(Context context, ViewMapOption viewMapOption) {
		SharedPreferences preferences = context.getSharedPreferences(ConfigOptions.preferences, Context.MODE_PRIVATE);
		SharedPreferences.Editor editor = preferences.edit();
		editor.putString(ConfigOptions.seeView, viewMapOption.name());
		editor.commit();
	}

	public static final ViewMapOption getViewSetting(Context context) {
		SharedPreferences preferences = context.getSharedPreferences(ConfigOptions.preferences, Context.MODE_PRIVATE);
		String defaultValue = ViewMapOption.None.name();
		String viewSetting = preferences.getString(ConfigOptions.seeView, defaultValue);
		return ViewMapOption.valueOf(viewSetting);
	}

	public enum TestType {
		MapTest, GlobeTest, BothTest;

		public boolean isMap() {
			return (this == MapTest || this == BothTest);
		}

		public boolean isGlobe() {
			return (this == GlobeTest || this == BothTest);
		}

	}

	public enum ViewMapOption {
		ViewMap, None
	}

	public enum TestState {
		Downloading, Ready, Executing, Error, Selected;

		public boolean canRun() {
			return (this != Executing && this != Error && this != Downloading);
		}
	}
}
