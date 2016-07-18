package io.theoutpost.helloearth;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import com.mousebird.maply.GlobeMapFragment;
import com.mousebird.maply.QuadImageTileLayer;
import com.mousebird.maply.RemoteTileInfo;
import com.mousebird.maply.RemoteTileSource;
import com.mousebird.maply.SphericalMercatorCoordSystem;

import java.io.File;


public class HelloGlobeFragment extends GlobeMapFragment {

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle inState) {
        super.onCreateView(inflater, container, inState);

        // Do app specific setup logic.

        return baseControl.getContentView();
    }

    @Override
    protected MapDisplayType chooseDisplayType() {
        return MapDisplayType.Globe;
    }

    @Override
    protected void controlHasStarted() {
        // setup base layer tiles
        String cacheDirName = "stamen_watercolor";
        File cacheDir = new File(getActivity().getCacheDir(), cacheDirName);
        cacheDir.mkdir();
        RemoteTileSource remoteTileSource = new RemoteTileSource(new RemoteTileInfo("http://tile.stamen.com/watercolor/", "png", 0, 18));
        remoteTileSource.setCacheDir(cacheDir);
        SphericalMercatorCoordSystem coordSystem = new SphericalMercatorCoordSystem();

        // globeControl is the controller when using MapDisplayType.Globe
        // mapControl is the controller when using MapDisplayType.Map
        QuadImageTileLayer baseLayer = new QuadImageTileLayer(globeControl, coordSystem, remoteTileSource);
        baseLayer.setImageDepth(1);
        baseLayer.setSingleLevelLoading(false);
        baseLayer.setUseTargetZoomLevel(false);
        baseLayer.setCoverPoles(true);
        baseLayer.setHandleEdges(true);

        // add layer and position
        globeControl.addLayer(baseLayer);
        globeControl.animatePositionGeo(-3.6704803, 40.5023056, 5, 1.0);
    }

}
