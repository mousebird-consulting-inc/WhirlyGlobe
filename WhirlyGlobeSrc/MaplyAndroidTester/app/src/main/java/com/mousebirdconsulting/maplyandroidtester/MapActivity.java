package com.mousebirdconsulting.maplyandroidtester;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.Menu;
import android.view.MenuItem;

public class MapActivity extends AppCompatActivity
{
    ConfigOptions config = null;
    MapGlobeTestFragment mapfrag = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_map);

        mapfrag = (MapGlobeTestFragment) getFragmentManager().findFragmentById(R.id.topfragment);
        if (mapfrag != null) {
            config = new ConfigOptions(mapfrag);
            config.updateOptions(null);
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_map, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (config != null) {
            if (config.mapType != mapfrag.mode)
            {
                // Note: Rebuild the fragment
            }
            config.updateOptions(item);
        }

        return super.onOptionsItemSelected(item);
    }
}
