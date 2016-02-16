package com.mousebirdconsulting.autotester;

import android.content.Intent;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v4.view.GravityCompat;
import android.support.v4.widget.DrawerLayout;
import android.support.v7.app.AppCompatActivity;
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

	public void prepareTest(){
		getSupportActionBar().setTitle("Running tests...");
		this.testResults.clear();
		if (ConfigOptions.getViewSetting(this) == ConfigOptions.ViewMapOption.ViewMap) {
			selectFragment(this.viewTest);
		}
		this.executing = true;
	}

	public void runTest(MaplyTestCase testCase) {
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
				finalizeTest();
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
		testCase.setListener(listener);
		testCase.execute();
	}

	private void finalizeTest() {
		getSupportActionBar().setTitle(R.string.app_name);
		executing = false;
		Intent intent = new Intent(this, ResultActivity.class);
		Bundle bundle = new Bundle();
		bundle.putSerializable("arraylist", this.testResults);
		intent.putExtras(bundle);
		startActivity(intent);
	}

	public boolean isExecuting() {
		return executing;
	}

	@Override
	public void onItemClick(int itemId) {
		navigationDrawer.setSelectedItemId(itemId);
	}
}
