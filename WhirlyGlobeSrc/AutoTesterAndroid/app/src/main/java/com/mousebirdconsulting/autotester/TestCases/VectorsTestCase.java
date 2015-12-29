package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;

import com.mousebird.maply.VectorObject;
import com.mousebirdconsulting.autotester.ConfigOptions;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;


public class VectorsTestCase extends MaplyTestCase {
	
	public VectorsTestCase(Activity activity) {
		super(activity);

		this.setTestName("Vectors Test");
		this.setSelected(ConfigOptions.getSelectedTest(activity, getTestName()));
		VectorObject vectorObject = new VectorObject();
	}


}
