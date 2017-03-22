package com.mousebirdconsulting.autotester.Fragments;

import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import com.mousebirdconsulting.autotester.R;

public class ViewTestFragment extends Fragment {

	private View theLayout;

	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
		if (theLayout == null) {
			theLayout = inflater.inflate(R.layout.viewtest_fragment, container, false);
		}
		return theLayout;
	}

	public void changeViewFragment(View view) {
		theLayout = view;
	}
}
