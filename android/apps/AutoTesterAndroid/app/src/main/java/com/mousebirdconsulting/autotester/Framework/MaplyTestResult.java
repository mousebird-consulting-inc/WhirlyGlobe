package com.mousebirdconsulting.autotester.Framework;

import java.io.Serializable;


public class MaplyTestResult implements Serializable {

	private String testName;
	private Throwable exception;

	MaplyTestResult(String testName, Throwable exception) {
		super();

		this.testName = testName;
		this.exception = exception;
	}

	MaplyTestResult(String testName) {
		super();

		this.testName = testName;
	}

	public boolean isPassed() {
		return (exception == null);
	}

	public String getTestName() {
		return testName;
	}

	public Throwable getException() {
		return exception;
	}
}
