package com.mousebirdconsulting.autotester.Framework;

import java.io.Serializable;


public class MaplyTestResult implements Serializable {

	private boolean passed;
	private String testName;

	MaplyTestResult(boolean passed, String testName) {
		this.passed = passed;
		this.testName = testName;
	}

	public boolean isPassed() {
		return passed;
	}

	public String getTestName() {
		return testName;
	}
}
