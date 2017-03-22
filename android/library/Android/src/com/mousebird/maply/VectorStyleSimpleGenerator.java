package com.mousebird.maply;

import android.graphics.Color;
import android.graphics.Typeface;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

/**
 * Created by sjg on 6/2/16.
 */
public class VectorStyleSimpleGenerator implements VectorStyleInterface
{
    /**
     * Base class for our vector style implementations.
     */
    public abstract class VectorStyleSimple implements VectorStyle
    {
        String uuid = null;

        /**
         * Draw priority for each feature.
         */
        public int drawPriority = VectorInfo.VectorPriorityDefault;

        @Override
        public String getUuid()
        {
            if (uuid == null)
            {
                uuid = " " + Math.random() * 1000000 + Math.random() * 10000;
            }
            return uuid;
        }

        /**
         * Set if this geometry is additive (e.g. sticks around) rather than being replaced.
         */
        @Override
        public boolean geomIsAdditive()
        {
            return false;
        }
    }

    // Note: This should be ThreadCurrent
    MaplyBaseController.ThreadMode threadMode = MaplyBaseController.ThreadMode.ThreadCurrent;

    /**
     * For points we just turn them into labels for demonstration.
     */
    public class VectorStyleSimplePoint extends VectorStyleSimple
    {
        LabelInfo labelInfo;

        VectorStyleSimplePoint(int inPriority)
        {
            drawPriority = inPriority;
            labelInfo = new LabelInfo();
            labelInfo.setFontSize(32.f);
            labelInfo.setTextColor(Color.WHITE);
            labelInfo.setTypeface(Typeface.DEFAULT);
            labelInfo.setDrawPriority(drawPriority);
            labelInfo.setEnable(false);
        }

        @Override
        public ComponentObject[] buildObjects(List<VectorObject> vecObjs, MaplyTileID tileID, MaplyBaseController controller)
        {
            ArrayList<ScreenLabel> labels = new ArrayList<ScreenLabel>();
            for (VectorObject point : vecObjs)
            {
                // Note: Do we need to split here
                AttrDictionary dict = point.getAttributes();
                String name = dict.getString("name");
                if (name == null)
                    name = dict.getString("value");
                Point2d pt = point.centroid();
                if (pt != null)
                {
                    ScreenLabel label = new ScreenLabel();
                    label.text = name != null ? name : ".";
                    label.loc = pt;

                    labels.add(label);
                }
            }

            ComponentObject compObj = controller.addScreenLabels(labels,labelInfo, threadMode);
            if (compObj != null)
                return new ComponentObject[]{compObj};
            return null;
        }
    }

    /**
     * For linears we turn them into random linear features.
     */
    public class VectorStyleSimpleLinear extends VectorStyleSimple
    {
        double red,green,blue;

        VectorStyleSimpleLinear(int priority)
        {
            drawPriority = priority;
            red = Math.random()/2.0+0.5;
            green = Math.random()/2.0+0.5;
            blue = Math.random()/2.0+0.5;
        }

        @Override
        public ComponentObject[] buildObjects(List<VectorObject> vecObjs,MaplyTileID tileID,MaplyBaseController controller)
        {
            VectorInfo vecInfo = new VectorInfo();
            vecInfo.disposeAfterUse = true;
            vecInfo.setColor((float)red,(float)green,(float)blue,1.f);
            vecInfo.setLineWidth(4.f);
            vecInfo.setFilled(false);
            vecInfo.setDrawPriority(drawPriority);
            vecInfo.setEnable(false);

            ComponentObject compObj = controller.addVectors(vecObjs,vecInfo,threadMode);
            if (compObj != null)
                return new ComponentObject[]{compObj};
            return null;
        }
    }

    /**
     *
     */
    public class VectorStyleSimplePolygon extends VectorStyleSimple
    {
        double red,green,blue;

        VectorStyleSimplePolygon(int priority)
        {
            drawPriority = priority;
            red = Math.random()/2.0+0.5;
            green = Math.random()/2.0+0.5;
            blue = 0.0;
        }

        @Override
        public ComponentObject[] buildObjects(List<VectorObject> vecObjs,MaplyTileID tileID,MaplyBaseController controller)
        {
            VectorInfo vecInfo = new VectorInfo();
            vecInfo.disposeAfterUse = true;
            vecInfo.setColor((float)red,(float)green,(float)blue,1.f);
            vecInfo.setFilled(true);
            vecInfo.setDrawPriority(drawPriority);
            vecInfo.setEnable(false);

            ComponentObject compObj = controller.addVectors(vecObjs,vecInfo,threadMode);
            if (compObj != null)
                return new ComponentObject[]{compObj};
            return null;
        }
    }

    MaplyBaseController controller = null;
    HashMap<String,VectorStyleSimple> stylesByUUID = new HashMap<String,VectorStyleSimple>();
    HashMap<String,VectorStyleSimple> stylesByLayerName = new HashMap<String,VectorStyleSimple>();


    public VectorStyleSimpleGenerator(MaplyBaseController inControl)
    {
        controller = inControl;
    }

    /**
     * We'll return a point, line, or areal vector style
     */
    @Override
    public VectorStyle[] stylesForFeature(AttrDictionary attrs,MaplyTileID tileID,String layerName,MaplyBaseController controller)
    {
        // Look for an existing style if we've already done this lookup
        VectorStyleSimple style = stylesByLayerName.get(layerName);
        if (style == null)
        {
            int layerOrder = attrs.getInt("layer_order");
            int geomType = attrs.getInt("geometry_type");

            // Each layer gets its own style
            switch (geomType)
            {
                case MapboxVectorTileParser.GeomTypePoint:
                    style = new VectorStyleSimplePoint(LabelInfo.LabelPriorityDefault+layerOrder);
                    break;
                case MapboxVectorTileParser.GeomTypeLineString:
                    style = new VectorStyleSimpleLinear(VectorInfo.VectorPriorityDefault+layerOrder);
                    break;
                case MapboxVectorTileParser.GeomTypePolygon:
                    style = new VectorStyleSimplePolygon(VectorInfo.VectorPriorityDefault+layerOrder);
                    break;
                default:
                    break;
            }

            stylesByLayerName.put(layerName,style);
            stylesByUUID.put(style.getUuid(),style);
        }

        return new VectorStyle[]{style};
    }

    /**
     * We'll display all layers
     */
    @Override
    public boolean layerShouldDisplay(String layerName,MaplyTileID tileID)
    {
        return true;
    }

    @Override
    public VectorStyle styleForUUID(String uuid,MaplyBaseController controller)
    {
        VectorStyle style = stylesByUUID.get(uuid);
        return style;
    }
}
