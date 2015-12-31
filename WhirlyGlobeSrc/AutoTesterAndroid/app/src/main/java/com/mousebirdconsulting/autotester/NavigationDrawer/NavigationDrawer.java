package com.mousebirdconsulting.autotester.NavigationDrawer;

import android.content.Context;
import android.graphics.Color;
import android.graphics.Typeface;
import android.os.Build;
import android.util.AttributeSet;
import android.util.TypedValue;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.mousebirdconsulting.autotester.ConfigOptions;
import com.mousebirdconsulting.autotester.R;

import butterknife.ButterKnife;
import butterknife.OnClick;


public class NavigationDrawer extends LinearLayout {

	public boolean seeView = false;

	public NavigationDrawer(Context context) {
		this(context, null);
	}

	public NavigationDrawer(Context context, AttributeSet attrs) {
		super(context, attrs);

		setOrientation(VERTICAL);


		TypedValue typedValue = new TypedValue();
		context.getTheme().resolveAttribute(android.R.attr.windowBackground, typedValue, true);
		setBackgroundColor(typedValue.data);

		LayoutInflater.from(context).inflate(R.layout.view_navigation_drawer, this, true);
		ButterKnife.inject(this);
	}

	@OnClick({R.id.deselectAll, R.id.selectAll, R.id.runMap, R.id.runGlobe, R.id.runBoth, R.id.seeView})
	void onItemClick(View view) {
		if (getContext() instanceof Listener) {
			((Listener) getContext()).onItemClick(view.getId());
		}
	}

	public void setSelectedItemId(int itemId) {

		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
			TextView runMapView, runGlobeView, runBothView, seeViewView;
			runGlobeView = (TextView) findViewById(R.id.runGlobe);
			runBothView = (TextView) findViewById(R.id.runBoth);
			runMapView = (TextView) findViewById(R.id.runMap);
			seeViewView = (TextView) findViewById(R.id.seeView);
			switch (itemId) {
				case R.id.runMap:
					deselectView(runBothView);
					deselectView(runGlobeView);
					selectView(runMapView);
					ConfigOptions.setTestType(getContext(), ConfigOptions.TestType.MapTest);
					break;
				case R.id.runGlobe:
					deselectView(runBothView);
					deselectView(runMapView);
					selectView(runGlobeView);
					ConfigOptions.setTestType(getContext(), ConfigOptions.TestType.GlobeTest);
					break;
				case R.id.runBoth:
					deselectView(runMapView);
					deselectView(runGlobeView);
					selectView(runBothView);
					ConfigOptions.setTestType(getContext(), ConfigOptions.TestType.BothTest);
					break;
				case R.id.seeView:
					if (!seeView) {
						selectView(seeViewView);
						ConfigOptions.setViewSetting(getContext(), ConfigOptions.ViewMapOption.ViewMap);
						seeView = true;
					} else {
						seeView = false;
						deselectView(seeViewView);
						ConfigOptions.setViewSetting(getContext(), ConfigOptions.ViewMapOption.None);
					}
					break;
			}
		}
	}

	private void deselectView(TextView view) {
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
			view.getCompoundDrawables()[0].setTintList(null);
			view.setTypeface(null, Typeface.NORMAL);
		}
	}

	private void selectView(TextView view) {
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP)
			view.getCompoundDrawables()[0].setTint(Color.BLACK);
		view.setTypeface(null, Typeface.BOLD);
	}

	public void getUserPreferences() {

		switch (ConfigOptions.getTestType(getContext())) {
			case MapTest:
				setSelectedItemId(R.id.runMap);
				break;
			case BothTest:
				setSelectedItemId(R.id.runBoth);
				break;
			case GlobeTest:
				setSelectedItemId(R.id.runGlobe);
				break;
		}

		switch (ConfigOptions.getViewSetting(getContext())) {
			case None:
				seeView = true;
				break;
			case ViewMap:
				seeView = false;
				break;
		}
		setSelectedItemId(R.id.seeView);
	}

	public interface Listener {
		void onItemClick(int itemId);
	}
}
