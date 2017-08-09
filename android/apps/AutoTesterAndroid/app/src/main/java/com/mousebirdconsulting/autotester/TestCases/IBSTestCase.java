package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.util.Log;

import com.mousebird.maply.ComponentObject;
import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MapController;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.MaplyTexture;
import com.mousebird.maply.Point2d;
import com.mousebird.maply.QuadImageTileLayer;
import com.mousebird.maply.Sticker;
import com.mousebird.maply.StickerInfo;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;

import java.util.ArrayList;
import java.util.List;
import java.util.Timer;
import java.util.TimerTask;

import static com.mousebirdconsulting.autotester.R.drawable.sticker;

/**
 * Project AutoTesterAndroid
 * Created by jmc on 09/08/2017.
 * (c) Iron Bird. All rights reserved.
 */

public class IBSTestCase extends MaplyTestCase {


    private MaplyBaseController controller;
    private ComponentObject[] components;

    private boolean smallToLarge = true;


    private TimerTask reorderTask = new TimerTask() {
        @Override
        public void run() {

            smallToLarge = !smallToLarge;

            int inc = smallToLarge ? -100 : +100;

            Log.v("maply", String.format("Reordering stikers: %s", smallToLarge ? "SMALL to LARGE" : "LARGE to SMALL"));

            for (int i = 0; i < components.length; i++) {

                ComponentObject component = components[i];

                if (component != null) {
                    StickerInfo info = new StickerInfo();
                    info.setDrawPriority(10000 + i * inc);

                    controller.changeSticker(component, info, MaplyBaseController.ThreadMode.ThreadAny);

                    Log.v("maply", String.format("  > Sticker %d set to priority %d", i, info.getDrawPriority()));
                }
            }

        }
    };


    public IBSTestCase(Activity activity) {
        super(activity);

        setTestName("IBS Test Case - Issue #985");
        setDelay(4);
        this.implementation = TestExecutionImplementation.Both;

    }


    @Override
    public boolean setUpWithMap(MapController mapVC) throws Exception {
        CartoDBMapTestCase mapBoxSatelliteTestCase = new CartoDBMapTestCase(getActivity());
        mapBoxSatelliteTestCase.setUpWithMap(mapVC);
        this.setUpReprocase(mapVC);

        return true;
    }

    @Override
    public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
        CartoDBMapTestCase mapBoxSatelliteTestCase = new CartoDBMapTestCase(getActivity());
        mapBoxSatelliteTestCase.setUpWithGlobe(globeVC);
        this.setUpReprocase(globeVC);
        return true;
    }



    private void setUpReprocase(MaplyBaseController controller) {

        this.controller = controller;

        // Creating the texture
        Bitmap bm = BitmapFactory.decodeResource(getActivity().getResources(), sticker);
        MaplyBaseController.TextureSettings settings = new MaplyBaseController.TextureSettings();
        MaplyTexture texture = controller.addTexture(bm, settings, MaplyBaseController.ThreadMode.ThreadAny);

        // Creating the components
        components = new ComponentObject[3];

        for (int i = 0; i < components.length; i++) {

            Sticker sticker = new Sticker();

            double size = (i + 1) * 1.0;

            double north = 43.60 + size / 2.0;
            double south = 43.60 - size / 2.0;
            double east = 3.85 + size / 2.0;
            double west = 3.85 - size / 2.0;

            Point2d ll = Point2d.FromDegrees(west, south);
            Point2d ur = Point2d.FromDegrees(east, north);

            sticker.setImageFormat(QuadImageTileLayer.ImageFormat.MaplyImageETC2RGBPA8);
            sticker.setLowerLeft(ll);
            sticker.setUpperRight(ur);
            sticker.setRotation((i - 1) * Math.PI / 4);

            ArrayList<MaplyTexture> images = new ArrayList<>();
            images.add(texture);
            sticker.setImages(images);

            StickerInfo info = new StickerInfo();
            info.setDrawPriority(10000 - i * 100);

            List<Sticker> stickers = new ArrayList<>();
            stickers.add(sticker);

            ComponentObject comp = controller.addStickers(stickers, info, MaplyBaseController.ThreadMode.ThreadAny);
            components[i] = comp;
        }

        new Timer().schedule(reorderTask, 1000, 2000);
    }
}
