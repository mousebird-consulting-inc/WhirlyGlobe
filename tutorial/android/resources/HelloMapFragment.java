package com.mousebirdconsulting.helloearth;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import com.mousebird.maply.GlobeMapFragment;
import com.mousebird.maply.QuadImageLoader;
import com.mousebird.maply.RemoteTileInfoNew;
import com.mousebird.maply.RenderController;
import com.mousebird.maply.SamplingParams;
import com.mousebird.maply.SphericalMercatorCoordSystem;

import java.io.File;

public class HelloMapFragment extends GlobeMapFragment {

    protected MapDisplayType chooseDisplayType() {
        return MapDisplayType.Map;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle inState) {
        super.onCreateView(inflater, container, inState);

        // Do app specific setup logic.

        return baseControl.getContentView();
    }

    @Override
    protected void controlHasStarted() {
        // Set up the local cache directory
        String cacheDirName = "stamen_watercolor6";
        File cacheDir = new File(getActivity().getCacheDir(), cacheDirName);
        cacheDir.mkdir();

        // Set up access to the tile images
        RemoteTileInfoNew tileInfo =
                new RemoteTileInfoNew("http://tile.stamen.com/watercolor/{z}/{x}/{y}.png",0, 18);
        tileInfo.cacheDir = cacheDir;

        // Set up the map parameters
        SamplingParams params = new SamplingParams();
        params.setCoordSystem(new SphericalMercatorCoordSystem());
        params.setCoverPoles(true);
        params.setEdgeMatching(true);
        params.setMinZoom(tileInfo.minZoom);
        params.setMaxZoom(tileInfo.maxZoom);
        params.setSingleLevel(true);

        // Set up an image loader, tying all the previous together.
        QuadImageLoader loader = new QuadImageLoader(params, tileInfo, baseControl);
        loader.setImageFormat(RenderController.ImageFormat.MaplyImageUShort565);

        // Go to a specific location with animation
        //
        // `globeControl` is the controller when using MapDisplayType.Globe
        // `mapControl` is the controller when using MapDisplayType.Map
        // `baseControl` refers to whichever of them is used.
        double latitude = 40.5023056 * Math.PI / 180;
        double longitude = -3.6704803 * Math.PI / 180;
        double zoom_earth_radius = 2.0;
        mapControl.animatePositionGeo(longitude, latitude, zoom_earth_radius, 1.0);
    }
}
