package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;

import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MapController;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.MaplyTexture;
import com.mousebird.maply.Point2d;
import com.mousebird.maply.QuadImageTileLayer;
import com.mousebird.maply.Sticker;
import com.mousebird.maply.StickerInfo;
import com.mousebird.maply.VectorObject;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;
import com.mousebirdconsulting.autotester.R;

import java.util.ArrayList;
import java.util.List;


public class StickersTestCase extends MaplyTestCase {

	public StickersTestCase(Activity activity) {
		super(activity);
		setTestName("Stickers Test");
		this.implementation = TestExecutionImplementation.Both;
	}

	@Override
	public boolean setUpWithMap(MapController mapVC) throws Exception {
		VectorsTestCase baseView = new VectorsTestCase(getActivity());
		baseView.setUpWithMap(mapVC);
		addStickers(baseView.getVectors(), mapVC);
		return true;
	}


	@Override
	public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
		VectorsTestCase baseView = new VectorsTestCase(getActivity());
		baseView.setUpWithGlobe(globeVC);
		addStickers(baseView.getVectors(), globeVC);
		return true;
	}

	private void addStickers(ArrayList<VectorObject> vectors, MaplyBaseController baseVC) {
		Bitmap icon = BitmapFactory.decodeResource(getActivity().getResources(), R.drawable.lfbd_pdc);
		MaplyTexture maplyTexture = baseVC.addTexture(icon, new MaplyBaseController.TextureSettings(), MaplyBaseController.ThreadMode.ThreadAny);
		ArrayList<MaplyTexture> textures = new ArrayList<>();
		textures.add(maplyTexture);
		List<Sticker> stickers = new ArrayList<>();
		for (VectorObject vector : vectors) {
			Sticker sticker = new Sticker();
			sticker.setImages(textures);
			Point2d center = Point2d.FromDegrees(-0.381378, 45.089304);
			if (center != null) {
				Point2d centroid = center;
				sticker.setLowerLeft(centroid.getX() - 0.0025f, centroid.getY() - 0.005f);
				sticker.setUpperRight(centroid.getX() + 0.0025f, centroid.getY() + 0.005f);
				sticker.setImageFormat(QuadImageTileLayer.ImageFormat.MaplyImageETC2RGBPA8);
				stickers.add(sticker);

				sticker = new Sticker();
				sticker.setImages(textures);
				sticker.setLowerLeft(centroid);
				sticker.setLowerLeft(centroid.getX() - 0.0025f, centroid.getY() - 0.005f);
				sticker.setUpperRight(centroid.getX() + 0.0025f, centroid.getY() + 0.005f);
				sticker.setImageFormat(QuadImageTileLayer.ImageFormat.MaplyImageETC2RGBPA8);
				sticker.setRotation(Math.PI / -4.0);
				stickers.add(sticker);
			}

			break;
		}
		StickerInfo info = new StickerInfo();
		baseVC.addStickers(stickers, info, MaplyBaseController.ThreadMode.ThreadAny);
	}
}
