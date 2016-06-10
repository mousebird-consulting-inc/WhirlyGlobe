package com.mousebirdconsulting.autotester.VectorStyle;

import android.content.Context;
import android.util.Log;

import com.fasterxml.jackson.core.JsonGenerationException;
import com.fasterxml.jackson.core.type.TypeReference;
import com.fasterxml.jackson.databind.JsonMappingException;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.mousebird.maply.AttrDictionary;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.MaplyTileID;
import com.mousebird.maply.VectorStyle;
import com.mousebird.maply.VectorStyleInterface;

import org.apache.commons.lang3.ArrayUtils;

import java.io.IOException;
import java.io.InputStream;
import java.util.HashMap;
import java.util.Map;

/**
 * Project AutoTesterAndroid
 * Created by jmc on 10/06/16.
 *
 * Reads a set of SimpleJsonStyles from a JSON file in the "vector_styles" assets directory.
 * The JSON file is a map of <"layer_name", SimpleJsonStyle[]>.
 */
public class SimpleJsonStyleProvider implements VectorStyleInterface {

    //***********************************************************************//
    //                           Inner classes                               //
    //***********************************************************************//


    //***********************************************************************//
    //                          Class variables                              //
    //***********************************************************************//

    private static final String TAG = "AutoTester";
    private static final String STYLE_ASSET_DIRECTORY = "vector_styles";

    private static Map<String, SimpleJsonStyle[]> styleMap;

    private static Map<String, SimpleJsonStyle> stylesByUuid = new HashMap<>();

    //***********************************************************************//
    //                         Instance variables                            //
    //***********************************************************************//


    int baseDrawPriority;
    int baseLayoutImportance;


    //***********************************************************************//
    //                            Constructors                               //
    //***********************************************************************//

    public SimpleJsonStyleProvider(Context context, String name, int drawPrioity, int layoutImportance) {

        baseDrawPriority = drawPrioity;
        baseLayoutImportance = layoutImportance;

        ObjectMapper mapper = new ObjectMapper();

        try {

            String assetFileName = STYLE_ASSET_DIRECTORY + "/" + name + ".json";

            InputStream is = context.getAssets().open(assetFileName);
            int size = is.available();
            byte[] json = new byte[size];
            is.read(json);
            is.close();
            styleMap = mapper.readValue(json,
                    new TypeReference<Map<String, SimpleJsonStyle[]>>() {});

        } catch (JsonGenerationException e) {

            String message = String.format("Got %s %s while deserializing style %s", e.getClass().getSimpleName(), e.getMessage(), name);
            Log.e(TAG, message, e);

        } catch (JsonMappingException e) {

            String message = String.format("Got %s %s while deserializing style %s", e.getClass().getSimpleName(), e.getMessage(), name);
            Log.e(TAG, message, e);

        } catch (IOException e) {

            String message = String.format("Got %s %s while deserializing style %s", e.getClass().getSimpleName(), e.getMessage(), name);
            Log.e(TAG, message, e);

        }

    }


    //***********************************************************************//
    //                         Getters and setters                           //
    //***********************************************************************//


    //***********************************************************************//
    //                               Interfaces                              //
    //***********************************************************************//

    /* Implements VectorStyleInterface */

    @Override
    public VectorStyle[] stylesForFeature(AttrDictionary attrDictionary, MaplyTileID maplyTileID, String layer, MaplyBaseController maplyBaseController) {

        SimpleJsonStyle[] styles = styleMap.get(layer);
        SimpleJsonStyle[] resultStyles = new SimpleJsonStyle[0];

        if (styles != null) {

            for (SimpleJsonStyle style : styles) {

                if (!style.isVisibleAtLevel(maplyTileID.level)) {
                    continue;
                }

                String uuid = style.getUuid();
                if (stylesByUuid.get(uuid) == null) {

                    // This is a new style
                    style.setBaseDrawPriority(baseDrawPriority);
                    style.setBaseLayoutImportance(baseLayoutImportance);
                    stylesByUuid.put(uuid, style);
                }

                resultStyles = ArrayUtils.add(resultStyles, style);
            }

            //Log.v(TAG, String.format("Requested style layer %s / level %d ==> [%s])", layer, maplyTileID.level, StringUtils.join(styles, ", ")));
            return resultStyles;
        }

        return new VectorStyle[0];
    }

    @Override
    public boolean layerShouldDisplay(String layer, MaplyTileID maplyTileID) {

        SimpleJsonStyle[] styles = styleMap.get(layer);

        if (styles != null) {
            for (SimpleJsonStyle style : styles) {
                if (style.isVisibleAtLevel(maplyTileID.level)) {
                    return true;
                }
            }
        }

        return false;
    }

    @Override
    public VectorStyle styleForUUID(String uuid, MaplyBaseController maplyBaseController) {
        return stylesByUuid.get(uuid);
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
