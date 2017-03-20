package com.mousebirdconsulting.autotester;

import android.graphics.Color;

import com.mousebird.maply.AttrDictionary;
import com.mousebird.maply.ComponentObject;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.MaplyTileID;
import com.mousebird.maply.VectorInfo;
import com.mousebird.maply.VectorObject;
import com.mousebird.maply.VectorStyle;
import com.mousebird.maply.VectorStyleInterface;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Project AutoTesterAndroid
 * Created by jmc on 06/07/16.
 */
public class ColorfulStyleGenerator implements VectorStyleInterface {


    //***********************************************************************//
    //                           Inner classes                               //
    //***********************************************************************//

    public abstract class ColorfulSimpleStyle implements VectorStyle {

        protected String uuid;

        @Override
        public String getUuid() {

            if (uuid == null) {
                uuid = "#" + Math.random() * 1000000.0D + Math.random() * 10000.0D;
            }

            return uuid;
        }
    }

    public class ColorfulSimpleVectors extends ColorfulSimpleStyle {

        private int priority;
        private int color;

        public ColorfulSimpleVectors(int priority) {
            this.priority = priority;
            int r = (int) (255 * (Math.random() / 2.0D + 0.5D));
            int g = (int) (255 * (Math.random() / 2.0D + 0.5D));
            int b = (int) (255 * (Math.random() / 2.0D + 0.5D));

            color = Color.rgb(r, g, b);
        }


        @Override
        public boolean geomIsAdditive() {
            return false;
        }


        @Override
        public ComponentObject[] buildObjects(List<VectorObject> vectors, MaplyTileID maplyTileID, MaplyBaseController controller) {

            VectorInfo vecInfo = new VectorInfo();
            vecInfo.setColor(color);
            vecInfo.setLineWidth(4.0F);
            vecInfo.setFilled(false);
            vecInfo.setDrawPriority(priority);
            vecInfo.setEnable(false);
            ComponentObject compObj = controller.addVectors(vectors, vecInfo, MaplyBaseController.ThreadMode.ThreadCurrent);

            return compObj != null ? new ComponentObject[]{compObj} : null;
        }
    }


    public class ColorfulSimpleAreals extends ColorfulSimpleStyle {

        private int priority;

        public ColorfulSimpleAreals(int priority) {
            this.priority = priority;
        }


        @Override
        public boolean geomIsAdditive() {
            return false;
        }

        @Override
        public ComponentObject[] buildObjects(List<VectorObject> vectors, MaplyTileID maplyTileID, MaplyBaseController controller) {
            VectorInfo vecInfo = new VectorInfo();
            vecInfo.setColor(AREAL_COLORS[maplyTileID.level]);
            vecInfo.setFilled(true);
            vecInfo.setDrawPriority(priority);
            vecInfo.setEnable(false);
            ComponentObject compObj = controller.addVectors(vectors, vecInfo, MaplyBaseController.ThreadMode.ThreadCurrent);

            return compObj != null ? new ComponentObject[]{compObj} : null;
        }
    }


    //***********************************************************************//
    //                          Class variables                              //
    //***********************************************************************//

    private static int AREAL_COLORS[] = {Color.WHITE, Color.LTGRAY, Color.GRAY, Color.DKGRAY, Color.GREEN, Color.BLUE, Color.CYAN, Color.MAGENTA, Color.BLACK};


    //***********************************************************************//
    //                         Instance variables                            //
    //***********************************************************************//

    private Map<String, VectorStyle> stylesByUuid = new HashMap<>();
    private Map<String, VectorStyle> stylesByLayerName = new HashMap<>();


    //***********************************************************************//
    //                            Constructors                               //
    //***********************************************************************//


    //***********************************************************************//
    //                         Getters and setters                           //
    //***********************************************************************//


    //***********************************************************************//
    //                               Interfaces                              //
    //***********************************************************************//

    /* Implements VectorStyleInterface */

    @Override
    public VectorStyle[] stylesForFeature(AttrDictionary attrs, MaplyTileID maplyTileID, String layer, MaplyBaseController maplyBaseController) {

        VectorStyle style;
        if ((style = stylesByLayerName.get(layer)) != null) {
            return new VectorStyle[]{style};
        }

        style = new ColorfulSimpleAreals(900);
        int geomType = attrs.getInt("geometry_type").intValue();

        if (geomType == 2) {
            style = new ColorfulSimpleVectors(1000);
        }

        stylesByUuid.put(style.getUuid(), style);
        stylesByLayerName.put(layer, style);

        return new VectorStyle[]{style};
    }

    @Override
    public boolean layerShouldDisplay(String s, MaplyTileID maplyTileID) {
        return false;
    }

    @Override
    public VectorStyle styleForUUID(String s, MaplyBaseController maplyBaseController) {
        return stylesByUuid.get(s);
    }


    //***********************************************************************//
    //                               Overrides                               //
    //***********************************************************************//


    //***********************************************************************//
    //                           Public methods                              //
    //***********************************************************************//


    //***********************************************************************//
    //                           Private methods                             //
    //***********************************************************************//


}
