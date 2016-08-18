package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.graphics.Typeface;

import com.mousebird.maply.ComponentObject;
import com.mousebird.maply.GlobeController;
import com.mousebird.maply.LabelInfo;
import com.mousebird.maply.MapController;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.MarkerInfo;
import com.mousebird.maply.Point2d;
import com.mousebird.maply.ScreenLabel;
import com.mousebird.maply.ScreenMarker;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;
import com.mousebirdconsulting.autotester.R;

import java.util.ArrayList;
import java.util.List;

/**
 * Project AutoTesterAndroid
 * Created by jmc on 17/08/16.
 */
public class MarkersAndLinesTestCase extends MaplyTestCase {

    //***********************************************************************//
    //                           Inner classes                               //
    //***********************************************************************//


    //***********************************************************************//
    //                          Class variables                              //
    //***********************************************************************//

    private double[][] COORDS = {
            {2.35149916815661, 48.8566101095316},
            {5.36995251900775, 43.2961742945861},
            {4.83201141297802, 45.7578136790559},
            {1.4442469046015, 43.6044621670154},
            {7.26839122452122, 43.7009357928554},
            {-1.5541361065088, 47.2186370612099},
            {7.75071269057704, 48.5846140177533},
            {3.8767336871979, 43.6112421949733},
            {-0.580036339906628, 44.8412250008144},
            {3.07064142643971, 50.6305088997931},
            {-1.68001972221832, 48.1113386665044},
            {4.44362435073131, 50.4120332112303},
            {6.14660141835058, 46.2017558838992},
            {4.03192600686169, 49.2577885717312},
            {0.107973185237803, 49.4938975049351},
            {7.58782612084171, 47.5581076788605},
            {4.38730582146636, 45.4401466607567},
            {5.93049191381938, 43.1257310642547},
            {5.72107728360614, 45.1824779689245},
            {5.04147006064019, 47.3215805996622},
            {-0.551558757253227, 47.4739883798232},
            {4.88693387045992, 45.7733105051169},
            {0.199533880740156, 48.0077780931396},
            {4.3600687223888, 43.8374248690446},
            {5.44747380426072, 43.5298423638856},
            {-4.48600875229816, 48.3905282739893},
            {3.08194268187158, 45.7774550754068},
            {1.26448467377022, 45.8354243106347},
            {0.688926794881045, 47.3900474153847},
            {6.63270245613282, 46.5218268592873},
            {2.29569510405488, 49.8941708007113},
            {6.1763551476966, 49.1196964147251},
            {2.88447133302376, 42.6953868051022},
            {6.02432462833833, 47.2379530147024},
            {2.24020598929176, 48.83566490905},
            {1.90860655655135, 47.9027336048355},
            {1.09396574788643, 49.4404591168357},
            {7.33993547919976, 47.7494187604284},
            {-0.369081458307233, 49.1828008109054},
            {2.35802318290753, 48.9357729750477},
            {6.18340970745432, 48.6937222616616},
            {2.24817970524819, 48.9479068830172},
            {6.1297989699503, 49.6112768154598},
            {2.44121840715805, 48.8623356855645},
            {3.17417343074436, 50.6915892653387},
            {3.15930819925428, 50.7217303973427},
            {2.37725251987939, 51.0347707950966},
            {2.45183712263704, 48.783072715727},
            {4.80603285361352, 43.9493142873587},
            {2.20712669676392, 48.8924273008969},
    };

    private String[] NAMES = {
            "Paris",
            "Marseille",
            "Lyon",
            "Toulouse",
            "Nice",
            "Nantes",
            "Strasbourg",
            "Montpellier",
            "Bordeaux",
            "Lille",
            "Rennes",
            "Charleroi",
            "Genève",
            "Reims",
            "Le Havre",
            "Basel",
            "Saint-Etienne",
            "Toulon",
            "Grenoble",
            "Dijon",
            "Angers",
            "Villeurbanne",
            "Le Mans",
            "Nîmes",
            "Aix-en-Provence",
            "Brest",
            "Clermont-Ferrand",
            "Limoges",
            "Tours",
            "Lausanne",
            "Amiens",
            "Metz",
            "Perpignan",
            "Besançon",
            "Boulogne-Billancourt",
            "Orléans",
            "Rouen",
            "Mulhouse",
            "Caen",
            "Saint-Denis",
            "Nancy",
            "Argenteuil",
            "Luxembourg",
            "Montreuil",
            "Roubaix",
            "Tourcoing",
            "Dunkerque",
            "Créteil",
            "Avignon",
            "Nanterre"
    };




    //***********************************************************************//
    //                         Instance variables                            //
    //***********************************************************************//

    private ComponentObject markers1;
    private ComponentObject markers2;
    private ComponentObject labels1;
    private ComponentObject labels2;
    private ComponentObject lines;


    //***********************************************************************//
    //                            Constructors                               //
    //***********************************************************************//

    public MarkersAndLinesTestCase(Activity activity) {
        super(activity);

        setTestName("Markers And Lines Test");
        setDelay(4);
        this.implementation = TestExecutionImplementation.Both;
    }



    //***********************************************************************//
    //                         Getters and setters                           //
    //***********************************************************************//


    //***********************************************************************//
    //                               Interfaces                              //
    //***********************************************************************//

    /* Implements TheInterface */


    //***********************************************************************//
    //                               Overrides                               //
    //***********************************************************************//

    @Override
    public boolean setUpWithMap(MapController mapVC) throws Exception {
        CartoDBMapTestCase mapBoxSatelliteTestCase = new CartoDBMapTestCase(getActivity());
        mapBoxSatelliteTestCase.setUpWithMap(mapVC);
        this.addMarkersAndLines(mapVC);
        return true;
    }


    @Override
    public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
        CartoDBMapTestCase mapBoxSatelliteTestCase = new CartoDBMapTestCase(getActivity());
        mapBoxSatelliteTestCase.setUpWithGlobe(globeVC);
        this.addMarkersAndLines(globeVC);
        return true;
    }


    //***********************************************************************//
    //                           Public methods                              //
    //***********************************************************************//


    //***********************************************************************//
    //                           Private methods                             //
    //***********************************************************************//


    private void addMarkersAndLines(MaplyBaseController controller) {


        // First layer
        List<ScreenMarker> markers = this.getMarkers();

        int drawPriority = 10000000;

        MarkerInfo mInfo = new MarkerInfo();
        mInfo.setDrawPriority(drawPriority);
        mInfo.setColor(Color.DKGRAY);

        markers1 = controller.addScreenMarkers(markers, mInfo, MaplyBaseController.ThreadMode.ThreadCurrent);

        drawPriority += 1;

        List<ScreenLabel> labels = this.getLabels();

        LabelInfo lInfo = new LabelInfo();
        lInfo.setTextColor(Color.DKGRAY);

        lInfo.setFontSize(15);
        lInfo.setTypeface(Typeface.DEFAULT_BOLD);
        lInfo.setDrawPriority(drawPriority);

        lInfo.setLayoutPlacement(LabelInfo.LayoutRight);

        labels1 = controller.addScreenLabels(labels, lInfo, MaplyBaseController.ThreadMode.ThreadCurrent);

        drawPriority += 100;

        // Second Layer

        markers = this.getMarkers();

        mInfo = new MarkerInfo();
        mInfo.setDrawPriority(drawPriority);
        mInfo.setColor(Color.DKGRAY);

        markers2 = controller.addScreenMarkers(markers, mInfo, MaplyBaseController.ThreadMode.ThreadCurrent);

        drawPriority += 1;

        labels = this.getLabels();

        lInfo = new LabelInfo();
        lInfo.setTextColor(Color.DKGRAY);

        lInfo.setFontSize(15);
        lInfo.setTypeface(Typeface.DEFAULT_BOLD);
        lInfo.setDrawPriority(drawPriority);

        lInfo.setLayoutPlacement(LabelInfo.LayoutRight);

        labels2 = controller.addScreenLabels(labels, lInfo, MaplyBaseController.ThreadMode.ThreadCurrent);

    }

    private List<ScreenLabel> getLabels() {

        List<ScreenLabel> labels = new ArrayList<>();

        for (int i = 0;  i < COORDS.length; i++) {

            double[] coords = COORDS[i];
            String name = NAMES[i];

            ScreenLabel label = new ScreenLabel();

            label.loc = new Point2d(Math.toRadians(coords[0]), Math.toRadians(coords[1]));
            label.selectable = true;
//            label.text = name;
            label.offset = new Point2d(10, 0);

            labels.add(label);
        }

        return labels;

    }

    private List<ScreenMarker> getMarkers() {

        List<ScreenMarker> markers = new ArrayList<>();

        Bitmap icon = BitmapFactory.decodeResource(getActivity().getResources(), R.drawable.testtarget);

        for (int i = 0;  i < COORDS.length; i++) {

            double[] coords = COORDS[i];
            String object = NAMES[i];

            ScreenMarker marker = new ScreenMarker();

            marker.image = icon;
            marker.loc = new Point2d(Math.toRadians(coords[0]), Math.toRadians(coords[1]));
            marker.size = new Point2d(10, 10);
            marker.selectable = true;
            marker.userObject = object;

            markers.add(marker);
        }

        return markers;
    }


}
