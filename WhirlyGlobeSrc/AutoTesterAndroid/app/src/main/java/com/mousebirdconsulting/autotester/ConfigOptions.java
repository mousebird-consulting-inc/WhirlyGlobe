package com.mousebirdconsulting.autotester;

import android.content.Context;
import android.content.SharedPreferences;

import com.mousebird.maply.StringWrapper;


public class ConfigOptions {

	public static final String seeView = "seeViewSetting";
	public static final String testType = "testTypeSetting";
	public static final String preferences = "AutoTesterAndroid";
	public static final String executionMode = "executionMode";

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

	public static final void setSelectedTest(Context context, String testName, boolean selected) {
		SharedPreferences preferences = context.getSharedPreferences(ConfigOptions.preferences, Context.MODE_PRIVATE);
		SharedPreferences.Editor editor = preferences.edit();
		editor.putBoolean(testName, selected);
		editor.commit();
	}

	public static final boolean getSelectedTest(Context context, String testName) {
		SharedPreferences preferences = context.getSharedPreferences(ConfigOptions.preferences, Context.MODE_PRIVATE);
		boolean defaultValue = false;
		return preferences.getBoolean(testName, defaultValue);
	}

	public enum TestType {
		MapTest, GlobeTest, BothTest
	}

	public enum ViewMapOption {
		ViewMap, None
	}
}
