/*
 *  StickersTestCase.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford.
 *  Copyright 2011-2019 mousebird consulting
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;

import com.mousebird.maply.ComponentObject;
import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MapController;
import com.mousebird.maply.BaseController;
import com.mousebird.maply.MaplyTexture;
import com.mousebird.maply.Point2d;
import com.mousebird.maply.RenderController;
import com.mousebird.maply.Sticker;
import com.mousebird.maply.StickerInfo;
import com.mousebird.maply.VectorInfo;
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

	private void addStickers(ArrayList<VectorObject> vectors, BaseController baseVC) {
		Bitmap icon = BitmapFactory.decodeResource(getActivity().getResources(), R.drawable.sticker);
		MaplyTexture maplyTexture = baseVC.addTexture(icon, null, RenderController.ThreadMode.ThreadAny);
		ArrayList<MaplyTexture> textures = new ArrayList<>();
		textures.add(maplyTexture);
		List<Sticker> stickers = new ArrayList<>();
		for (VectorObject vector : vectors) {
			Sticker sticker = new Sticker();
			sticker.setTextures(textures);
			Point2d center = vector.centroid();
			if (center != null) {
				Point2d centroid = vector.centroid();
				sticker.setLowerLeft(centroid);
				sticker.setUpperRight(new Point2d(centroid.getX() + 0.1, centroid.getY() + 0.1));
				sticker.setRotation(45.0/180.0 * Math.PI);
				stickers.add(sticker);
			}
		}
		StickerInfo info = new StickerInfo();
		info.setDrawPriority(VectorInfo.VectorPriorityDefault+1000);
		ComponentObject compObj = baseVC.addStickers(stickers, info, RenderController.ThreadMode.ThreadCurrent);
		StickerInfo info2 = new StickerInfo();
		info2.setDrawPriority(VectorInfo.VectorPriorityDefault-1);
		baseVC.changeSticker(compObj, info2, RenderController.ThreadMode.ThreadCurrent);
	}
}
