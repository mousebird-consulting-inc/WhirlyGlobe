package com.mousebirdconsulting.autotester;

import android.content.Intent;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v4.view.GravityCompat;
import android.support.v4.widget.DrawerLayout;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.view.menu.ActionMenuItemView;
import android.support.v7.widget.Toolbar;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;

import com.mousebirdconsulting.autotester.Fragments.TestListFragment;
import com.mousebirdconsulting.autotester.Fragments.ViewTestFragment;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;
import com.mousebirdconsulting.autotester.Framework.MaplyTestResult;
import com.mousebirdconsulting.autotester.NavigationDrawer.NavigationDrawer;

import java.io.File;
import java.util.ArrayList;

import butterknife.ButterKnife;
import butterknife.InjectView;

public class MainActivity extends AppCompatActivity implements NavigationDrawer.Listener {

	@InjectView(R.id.toolbar)
	Toolbar toolbar;
	@InjectView(R.id.drawer_layout)
	DrawerLayout drawerLayout; //Drawerlayout
	@InjectView(R.id.navigation_drawer)
	NavigationDrawer navigationDrawer;
	private Menu menu;
	private TestListFragment testList;
	private ViewTestFragment viewTest;
	private MaplyTestCase interactiveTest = null;

	private boolean cancelled = false;
	private boolean executing = false;

	private ArrayList<MaplyTestResult> testResults;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		setContentView(R.layout.activity_main);
		ButterKnife.inject(this);

		// Force a load of the library
		System.loadLibrary("Maply");

		//Create toolbar
		configureToolbar();
		configureNavigationDrawer();
		deleteRecursive(ConfigOptions.getCacheDir(this));
		this.navigationDrawer.getUserPreferences();
		this.testList = new TestListFragment();
		this.testResults = new ArrayList<>();
		this.viewTest = new ViewTestFragment();
	}

	void deleteRecursive(File fileOrDirectory) {
		if (fileOrDirectory.isDirectory())
			for (File child : fileOrDirectory.listFiles())
				deleteRecursive(child);

		fileOrDirectory.delete();
	}

	@Override
	protected void onResume() {
		super.onResume();

		this.navigationDrawer.getUserPreferences();
		this.testList = new TestListFragment();
		this.testList.downloadResources();
		selectFragment(this.testList);
		this.viewTest = new ViewTestFragment();
	}

	private void configureToolbar() {
		setSupportActionBar(toolbar);

		//Add home button
		android.support.v7.app.ActionBar actionBar = getSupportActionBar();
		actionBar.setDisplayHomeAsUpEnabled(true);
		actionBar.setDisplayShowTitleEnabled(true);
		actionBar.setHomeAsUpIndicator(R.drawable.ic_options_action);
	}

	private void configureNavigationDrawer() {
		this.drawerLayout.setDrawerShadow(R.drawable.drawer_shadow, GravityCompat.START);
		drawerLayout.setDrawerLockMode(DrawerLayout.LOCK_MODE_LOCKED_CLOSED);
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.menu_main, menu);
		this.menu = menu;
		selectFragment(this.testList);
		this.testList.downloadResources();
		showOverflowMenu(ConfigOptions.getExecutionMode(getApplicationContext())== ConfigOptions.ExecutionMode.Multiple);
		return true;
	}

	public void showOverflowMenu(boolean showMenu){
		if(menu == null)
			return;
		menu.setGroupVisible(R.id.main_menu_group, showMenu);
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// Handle action bar item clicks here. The action bar will
		// automatically handle clicks on the Home/Up button, so long
		// as you specify a parent activity in AndroidManifest.xml.
		int id = item.getItemId();

		switch (id) {
			case android.R.id.home:
				if (!executing) {
					drawerLayout.openDrawer(GravityCompat.START);
				}
				break;
			case R.id.playTests:
				if (ConfigOptions.getExecutionMode(getApplicationContext()) == ConfigOptions.ExecutionMode.Interactive) {
					finalizeInteractiveTest();
				}
				if (ConfigOptions.getExecutionMode(getApplicationContext()) == ConfigOptions.ExecutionMode.Multiple) {
					if (!executing) {
						this.runTests();
					} else {
						this.stopTests();
					}
				}
				break;
		}
		return super.onOptionsItemSelected(item);
	}

	private void selectFragment(Fragment fragment) {
		this.drawerLayout.closeDrawer(GravityCompat.START);

		getSupportFragmentManager()
			.beginTransaction()
			.replace(R.id.content_frame, fragment)
			.commit();
	}

	public void prepareTest(MaplyTestCase testCase) {
		ConfigOptions.TestState testState = ConfigOptions.getTestState(getApplicationContext(), testCase.getTestName());

		if (testState.canRun()) {
			String titleText = "Running tests...";
			if (ConfigOptions.getExecutionMode(getApplicationContext()) == ConfigOptions.ExecutionMode.Interactive) {
				titleText = "Interactive test";
				this.interactiveTest = testCase;
				MenuItem item = menu.findItem(R.id.playTests);
				if (item != null) {
					item.setIcon(getResources().getDrawable(R.drawable.ic_done_action));
				}
				showOverflowMenu(true);
			}

			getSupportActionBar().setTitle(titleText);
			this.testResults.clear();
			if (ConfigOptions.getViewSetting(this) == ConfigOptions.ViewMapOption.ViewMap || ConfigOptions.getExecutionMode(getApplicationContext()) == ConfigOptions.ExecutionMode.Interactive) {
				selectFragment(this.viewTest);
			}
			this.executing = true;
		}
	}

	public void runTest(final MaplyTestCase testCase) {
		ConfigOptions.TestState testState = ConfigOptions.getTestState(getApplicationContext(), testCase.getTestName());

		if (testState.canRun()) {
			ConfigOptions.setTestState(getApplicationContext(), testCase.getTestName(), ConfigOptions.TestState.Executing);
			testCase.setOptions(ConfigOptions.getTestType(this));
			MaplyTestCase.MaplyTestCaseListener listener = new MaplyTestCase.MaplyTestCaseListener() {
				@Override
				public void onFinish(MaplyTestResult resultMap, MaplyTestResult resultGlobe) {
					if (resultMap != null) {
						MainActivity.this.testResults.add(resultMap);
					}
					if (resultGlobe != null) {
						MainActivity.this.testResults.add(resultGlobe);
					}
					finalizeTest(testCase);
				}

				@Override
				public void onStart(View view) {
					if (ConfigOptions.getViewSetting(MainActivity.this) == ConfigOptions.ViewMapOption.ViewMap || ConfigOptions.getExecutionMode(getApplicationContext()) == ConfigOptions.ExecutionMode.Interactive) {
						viewTest = new ViewTestFragment();
						viewTest.changeViewFragment(view);
						selectFragment(viewTest);
					}
				}

				@Override
				public void onExecute(View view) {
					if (ConfigOptions.getViewSetting(MainActivity.this) == ConfigOptions.ViewMapOption.ViewMap || ConfigOptions.getExecutionMode(getApplicationContext()) == ConfigOptions.ExecutionMode.Interactive) {
						viewTest = new ViewTestFragment();
						viewTest.changeViewFragment(view);
						selectFragment(viewTest);
					}
				}
			};
			testCase.setListener(listener);
			testCase.start();
		}
	}

	private void finalizeTest(MaplyTestCase testCase) {
		getSupportActionBar().setTitle(R.string.app_name);
		ConfigOptions.setTestState(getApplicationContext(), testCase.getTestName(), ConfigOptions.TestState.Ready);
		executing = false;
		Intent intent = new Intent(this, ResultActivity.class);
		Bundle bundle = new Bundle();
		bundle.putSerializable("arraylist", this.testResults);
		intent.putExtras(bundle);
		startActivity(intent);
	}

	private void finalizeInteractiveTest() {
		getSupportActionBar().setTitle(R.string.app_name);
		interactiveTest.cancel(true);
		interactiveTest.shutdown();
		ConfigOptions.setTestState(getApplicationContext(), interactiveTest.getTestName(), ConfigOptions.TestState.Ready);
		interactiveTest = null;
		MenuItem item = menu.findItem(R.id.playTests);
		if (item != null){
			item.setIcon(getResources().getDrawable(R.drawable.ic_play_action));
		}
		showOverflowMenu(false);
		executing = false;
		onResume();
	}

	public boolean isExecuting() {
		return executing;
	}

	@Override
	public void onItemClick(int itemId) {
		switch (itemId){
			case R.id.selectAll:
				testList.changeItemsState(true);
				drawerLayout.closeDrawer(GravityCompat.START);
				break;
			case R.id.deselectAll:
				testList.changeItemsState(false);
				drawerLayout.closeDrawer(GravityCompat.START);
				break;
			case R.id.runInteractive:
				showOverflowMenu(false);
				onResume();
				break;

			case R.id.runMultiple:
				showOverflowMenu(true);
				onResume();
				break;

			case R.id.runSingle:
				showOverflowMenu(false);
				onResume();
				break;
		}

		navigationDrawer.setSelectedItemId(itemId);
	}

	private void runTests() {
		ActionMenuItemView playButton = (ActionMenuItemView) findViewById(R.id.playTests);
		playButton.setIcon(getResources().getDrawable(R.drawable.ic_stop_action));
		getSupportActionBar().setTitle("Running tests...");
		this.testResults.clear();
		ArrayList<MaplyTestCase> tests = this.testList.getTests();
		if (ConfigOptions.getViewSetting(this) == ConfigOptions.ViewMapOption.ViewMap) {
			selectFragment(this.viewTest);
		}
		this.executing = true;
		startTests(tests, 0);
	}


	private void startTests(final ArrayList<MaplyTestCase> tests, final int index) {
		if (tests.size() != index) {
			final MaplyTestCase head = tests.get(index);
			if (ConfigOptions.getTestState(getApplicationContext(), head.getTestName()) == ConfigOptions.TestState.Selected) {
				head.setOptions(ConfigOptions.getTestType(this));
				ConfigOptions.setTestState(getApplicationContext(), head.getTestName(), ConfigOptions.TestState.Executing);
				MaplyTestCase.MaplyTestCaseListener listener = new MaplyTestCase.MaplyTestCaseListener() {
					@Override
					public void onFinish(MaplyTestResult resultMap, MaplyTestResult resultGlobe) {
						ConfigOptions.setTestState(getApplicationContext(), head.getTestName(), ConfigOptions.TestState.Selected);
						if (MainActivity.this.cancelled) {
							MainActivity.this.finishTests();
						} else {
							if (resultMap != null) {
								MainActivity.this.testResults.add(resultMap);
							}
							if (resultGlobe != null) {
								MainActivity.this.testResults.add(resultGlobe);
							}
							if (ConfigOptions.getViewSetting(MainActivity.this) == ConfigOptions.ViewMapOption.None) {
								head.setIcon(R.drawable.ic_action_selectall);
								MainActivity.this.testList.notifyIconChanged(index);
							}
							MainActivity.this.startTests(tests, index + 1);
						}
					}

					@Override
					public void onStart(View view) {

						if (ConfigOptions.getViewSetting(MainActivity.this) == ConfigOptions.ViewMapOption.ViewMap) {
							viewTest = new ViewTestFragment();
							viewTest.changeViewFragment(view);
							selectFragment(viewTest);
						}
					}

					@Override
					public void onExecute(View view) {
						if (ConfigOptions.getViewSetting(MainActivity.this) == ConfigOptions.ViewMapOption.ViewMap || ConfigOptions.getExecutionMode(getApplicationContext()) == ConfigOptions.ExecutionMode.Interactive) {
							viewTest = new ViewTestFragment();
							viewTest.changeViewFragment(view);
							selectFragment(viewTest);
						}
					}
				};
				head.setListener(listener);
				head.setActivity(this);
				if (ConfigOptions.getViewSetting(this) == ConfigOptions.ViewMapOption.None) {
					head.setIcon(R.drawable.ic_options_action);
					this.testList.notifyIconChanged(index);
				}
				head.start();
			} else {
				startTests(tests, index + 1);
			}
		} else {
			finishTests();
		}
	}

	private void finishTests() {
		ActionMenuItemView playButton = (ActionMenuItemView) findViewById(R.id.playTests);
		playButton.setIcon(getResources().getDrawable(R.drawable.ic_play_action));
		getSupportActionBar().setTitle(R.string.app_name);
		executing = false;
		this.viewTest = new ViewTestFragment();
		if (!cancelled) {
			Intent intent = new Intent(this, ResultActivity.class);
			Bundle bundle = new Bundle();
			bundle.putSerializable("arraylist", this.testResults);
			intent.putExtras(bundle);
			startActivity(intent);
		} else {
			cancelled = false;
			this.testList = new TestListFragment();
			selectFragment(this.testList);
		}
	}

	private void stopTests() {
		getSupportActionBar().setTitle("Cancelling...");
		cancelled = true;
	}
}
