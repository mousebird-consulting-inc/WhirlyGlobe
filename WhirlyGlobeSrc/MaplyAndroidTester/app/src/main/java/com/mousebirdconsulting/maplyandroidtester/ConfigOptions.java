package com.mousebirdconsulting.maplyandroidtester;

import android.view.MenuItem;

/**
 * Created by sjg on 8/31/15.
 */
public class ConfigOptions {
    // 2D map or 3D globe
    public enum MapType {
        FlatMap, GlobeMap
    }

    // Identifiers for base layers
    public enum BaseOptions {
        BlankLayer, GeographyClass, MapboxRegular, MapboxSatellite, OSMMapquest, StamenWatercolor, QuadTest, QuadTestAnimate, QuadVectorTest
    }

    // Identifiers for overlay options
    public enum OverlayOptions {
        ForecastIO, MaxOverlayOptions
    }

    // Identifiers for objects to add to the mix
    public enum ObjectOptions
    {
        ScreenMarkers,ScreenLabels, MaxObjectOptions
    }

    // Identifiers for utility options
    public enum UtilOptions
    {
        PerfOutput,MaxUtilOptions
    }

    // Map type
    public MapType mapType = MapType.GlobeMap;
    // Base layer section selection
    public BaseOptions baseSection = BaseOptions.BlankLayer;
    // Which overlays are on
    public boolean overlays[] = new boolean[OverlayOptions.MaxOverlayOptions.ordinal()];
    // Which objects are on
    public boolean objects[] = new boolean[ObjectOptions.MaxObjectOptions.ordinal()];
    // Which utility options are on
    public boolean utils[] = new boolean[UtilOptions.MaxUtilOptions.ordinal()];

    public interface ConfigOptionsListener
    {
        public void userChangedSelections(ConfigOptions config);
    }
    ConfigOptionsListener configListen = null;

    ConfigOptions(ConfigOptionsListener listener)
    {
        configListen = listener;
        overlays[0] = false;
        utils[0] = false;
    }

    // Set the callback for user changes
    public void setConfigListener(ConfigOptionsListener inListener)
    {
        configListen = inListener;
    }

    // Update the options
    public void updateOptions(MenuItem item)
    {
        if (item != null) {
            int id = item.getItemId();

            // Interpret whatever they selected
            switch (id)
            {
                case R.id.map_type:
                case R.id.base_layers:
                case R.id.overlay_layers:
                case R.id.utils:
                    // No changes here
                    return;
                case R.id.flatmap:
                    mapType = MapType.FlatMap;
                    break;
                case R.id.globe:
                    mapType = MapType.GlobeMap;
                    break;
                case R.id.blank_layer:
                    baseSection = BaseOptions.BlankLayer;
                    break;
                case R.id.geography_layer:
                    baseSection = BaseOptions.GeographyClass;
                    break;
                case R.id.mapboxRegular_layer:
                    baseSection = BaseOptions.MapboxRegular;
                    break;
                case R.id.mapboxSatellite_layer:
                    baseSection = BaseOptions.MapboxSatellite;
                    break;
                case R.id.osmmapquest_layer:
                    baseSection = BaseOptions.OSMMapquest;
                    break;
                case R.id.stamen_layer:
                    baseSection = BaseOptions.StamenWatercolor;
                    break;
                case R.id.quadtest_layer:
                    baseSection = BaseOptions.QuadTest;
                    break;
                case R.id.quadtestanimate_layer:
                    baseSection = BaseOptions.QuadTestAnimate;
                    break;
                case R.id.quadvectortest_layer:
                    baseSection = BaseOptions.QuadVectorTest;
                    break;
                case R.id.forecastio_layer:
                    overlays[0] = !overlays[0];
                    break;
                case R.id.screen_markers:
                    objects[0] = !objects[0];
                    break;
                case R.id.perfOutput:
                    utils[0] = !utils[0];
                    break;
                }
            }

        configListen.userChangedSelections(this);
    }
}
