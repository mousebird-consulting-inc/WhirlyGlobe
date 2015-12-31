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
	private TestListFragment testList;
	private ViewTestFragment viewTest;

	private boolean executing = false;
	private boolean cancelled;

	private ArrayList<MaplyTestResult> testResults;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		setContentView(R.layout.activity_main);
		ButterKnife.inject(this);

		//Create toolbar
		configureToolbar();
		configureNavigationDrawer();

		this.navigationDrawer.getUserPreferences();
		this.testList = new TestListFragment();
		this.testResults = new ArrayList<>();
		this.viewTest = new ViewTestFragment();
	}

	@Override
	protected void onResume() {
		super.onResume();

		this.navigationDrawer.getUserPreferences();
		this.testList = new TestListFragment();
		selectFragment(this.testList);
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
		selectFragment(this.testList);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// Handle action bar item clicks here. The action bar will
		// automatically handle clicks on the Home/Up button, so long
		// as you specify a parent activity in AndroidManifest.xml.
		int id = item.getItemId();

		switch (id) {
			case R.id.playTests:
				if (!executing) {
					this.runTests();
				} else {
					this.stopTests();
				}
				break;
			case android.R.id.home:
				if (!executing) {
					drawerLayout.openDrawer(GravityCompat.START);
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

	@Override
	public void onItemClick(int itemId) {
		switch (itemId) {
			case R.id.selectAll:
				testList.changeItemsState(true);
				drawerLayout.closeDrawer(GravityCompat.START);
				break;
			case R.id.deselectAll:
				testList.changeItemsState(false);
				drawerLayout.closeDrawer(GravityCompat.START);
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
			if (head.isSelected()) {
				head.setOptions(ConfigOptions.getTestType(this));
				MaplyTestCase.MaplyTestCaseListener listener = new MaplyTestCase.MaplyTestCaseListener() {
					@Override
					public void onFinish(MaplyTestResult resultMap, MaplyTestResult resultGlobe) {
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
					public void onExecute(View view) {
						if (ConfigOptions.getViewSetting(MainActivity.this) == ConfigOptions.ViewMapOption.ViewMap) {
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
				head.execute();
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

	public boolean isExecuting() {
		return executing;
	}

}
