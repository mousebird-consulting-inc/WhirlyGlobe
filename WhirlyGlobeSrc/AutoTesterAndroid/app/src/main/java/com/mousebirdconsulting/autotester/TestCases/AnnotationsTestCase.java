package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;

import com.mousebird.maply.Annotation;
import com.mousebird.maply.AttrDictionary;
import com.mousebird.maply.ComponentObject;
import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MapController;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.MarkerInfo;
import com.mousebird.maply.Point2d;
import com.mousebird.maply.ScreenMarker;
import com.mousebird.maply.VectorObject;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;
import com.mousebirdconsulting.autotester.R;

import java.util.ArrayList;


public class AnnotationsTestCase extends MaplyTestCase
{

	public AnnotationsTestCase(Activity activity) {
		super(activity);
		setTestName("Annotations Test");
		setDelay(1000);
		this.implementation = TestExecutionImplementation.Map;
	}

	@Override
	public boolean setUpWithMap(MapController mapVC) throws Exception {
		VectorsTestCase baseView = new VectorsTestCase(getActivity());
		baseView.setUpWithMap(mapVC);

		Point2d loc = Point2d.FromDegrees(-3.6704803, 40.5023056);
		mapVC.setPositionGeo(loc.getX(), loc.getX(), 5);

		Annotation annotation = new Annotation(getActivity(), 50, 50);
		annotation.setTitle("San Francisco");
		annotation.setSubTitle("This is a test");
		annotation.setLoc(Point2d.FromDegrees(-122.4192, 37.7793));
		annotation.generateLayout(Annotation.AnnoType.TitleAndSubTitleText,null);
		annotation.setVisible(true);
		mapVC.addAnnotation(annotation);
		mapVC.setAnnotationVisible(true, annotation);

		Annotation annotation2 = new Annotation(getActivity(), 50, 50);
		annotation2.setTitle("London");
		annotation2.setSubTitle("This is other test");
		annotation2.setLoc(Point2d.FromDegrees(-0.1275, 51.507222));
		annotation2.generateLayout(Annotation.AnnoType.TitleAndSubTitleText,null);
		annotation2.setVisible(true);
		mapVC.addAnnotation(annotation2);
		mapVC.setAnnotationVisible(true, annotation2);

		return true;
	}

}
