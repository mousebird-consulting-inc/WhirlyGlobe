package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;

import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;

/**
 * Created by sjg on 8/11/16.
 */
public class StartupShutdownTestCase extends MaplyTestCase
{
    public StartupShutdownTestCase(Activity activity) {
        super(activity);

        setTestName("Startup/Shutdown Test Case");
        setDelay(4);
        this.implementation = TestExecutionImplementation.Both;
    }
}
