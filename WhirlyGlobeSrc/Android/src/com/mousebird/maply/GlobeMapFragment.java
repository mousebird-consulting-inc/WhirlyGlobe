package com.mousebird.maply;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.ViewGroup;

/**
 * A fragment that instantiates either a map or a globe.
 * You subclass this and fill in the various stubs to implement your own
 * map or globe.
 */
public class GlobeMapFragment extends android.support.v4.app.Fragment implements MapController.GestureDelegate, GlobeController.GestureDelegate {
    protected MapController mapControl;
    protected GlobeController globeControl;
    protected MaplyBaseController baseControl;

    public enum MapDisplayType {Globe, Map}

    ;
    protected MapDisplayType mapDisplayType = MapDisplayType.Map;

    protected GlobeController.Settings globeSettings = new GlobeController.Settings();
    protected MapController.Settings mapSettings = new MapController.Settings();

    /**
     * Override this to tell the fragment you want either a map or a globe.
     */
    protected MapDisplayType chooseDisplayType() {
        return MapDisplayType.Map;
    }

    /**
     * Override this to be called right before the map or globe is created.
     * This is where you can mess with the globe or map settings.
     */
    protected void preControlCreated() {
    }

    /**
     * Override this to be called when the control is up and running.
     * There are many calls to the map or globe controller you shouldn't make
     * until it has its OpenGL surface set up.  This is where you make those
     * calls.  This will be on the main thread.
     */
    protected void controlHasStarted() {
    }

    /**
     * This is the standard onCreateView() for the fragment.  If you override it,
     * be sure to call this one too.  It does all the setup for the globe or map.
     */
    @Override
    public android.view.View onCreateView(LayoutInflater inflater, ViewGroup container,
                                          Bundle inState) {
        super.onCreateView(inflater, container, inState);

        mapDisplayType = chooseDisplayType();

        preControlCreated();

        if (mapDisplayType == MapDisplayType.Map) {
            mapControl = new MapController(getActivity(), mapSettings);
            baseControl = mapControl;
        } else {
            globeControl = new GlobeController(getActivity(), globeSettings);
            baseControl = globeControl;
        }

        baseControl.addPostSurfaceRunnable(new Runnable() {
            @Override
            public void run() {
                controlHasStarted();
            }
        });

        return baseControl.getContentView();
    }

    /**
     * The standard fragment onSaveInstanceState().  The GlobeMapFragment is doing nothing
     * at this point.
     */
    @Override
    public void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
    }

    /**
     * The standard fragment onRestoreInstanceState().  The GlobeMapFragment is doing nothing
     * at this point.
     *
     * @param inState
     */
    @Override
    public void onViewStateRestored(Bundle inState) {
        super.onViewStateRestored(null);
    }

    @Override
    public void onPause() {
        super.onPause();
    }

    @Override
    public void onResume() {
        super.onResume();
    }

    /**
     * This is the standard fragment onDestroyView().  The GlobeMapFragment calls the controller
     * shutdown here, so be sure to call it if you override this.
     */
    @Override
    public void onDestroyView()
    {
        if (baseControl != null)
        {
            baseControl.shutdown();;
            baseControl = null;
            globeControl = null;
            mapControl = null;
        }

        super.onDestroyView();
    }

    @Override
    public void onDestroy()
    {
        super.onDestroy();
    }

    /**
     * Map delegate callbacks.  Override these to get map feedback.
     */

    /**
     * The user selected the given object.  Up to you to figure out what it is.
     *
     * @param mapControl The maply controller this is associated with.
     * @param selObjs The objects the user selected (e.g. MaplyScreenMarker).
     * @param loc The location they tapped on.  This is in radians.
     * @param screenLoc The location on the OpenGL surface.
     */
    public void userDidSelect(MapController mapControl,SelectedObject[] selObjs,Point2d loc,Point2d screenLoc)
    {
    }

    /**
     * The user tapped somewhere, but not on a selectable object.
     *
     * @param mapControl The maply controller this is associated with.
     * @param loc The location they tapped on.  This is in radians.
     * @param screenLoc The location on the OpenGL surface.
     */
    public void userDidTap(MapController mapControl,Point2d loc,Point2d screenLoc)
    {
    }

    /**
     * The user long pressed somewhere, either on a selectable object or nor
     * @param mapController The maply controller this is associated with.
     * @param selObjs The objects (e.g. MaplyScreenMarker) that the user long pressed or null if there was none
     * @param loc The location they tapped on.  This is in radians.
     * @param screenLoc The location on the OpenGL surface.
     */
    public void userDidLongPress(MapController mapController, SelectedObject[] selObjs, Point2d loc, Point2d screenLoc)
    {
    }

    /**
     * Called when the map first starts moving.
     *
     * @param mapControl The map controller this is associated with.
     * @param userMotion Set if the motion was caused by a gesture.
     */
    public void mapDidStartMoving(MapController mapControl, boolean userMotion)
    {
    }

    /**
     * Called when the map stops moving.
     *
     * @param mapControl The map controller this is associated with.
     * @param corners Corners of the viewport.  If one of them is null, that means it doesn't land anywhere valid.
     * @param userMotion Set if the motion was caused by a gesture.
     */
    public void mapDidStopMoving(MapController mapControl, Point3d corners[], boolean userMotion)
    {
    }

    /**
     * Called for every single visible frame of movement.  Be careful what you do in here.
     *
     * @param mapControl The map controller this is associated with.
     * @param corners Corners of the viewport.  If one of them is null, that means it doesn't land anywhere valid.
     * @param userMotion Set if the motion was caused by a gesture.
     */
    public void mapDidMove(MapController mapControl,Point3d corners[], boolean userMotion)
    {
    }

    /**
     * Globe delegate callbacks.  Override these to get globe feedback.
     */

    /**
     * The user selected the given object.  Up to you to figure out what it is.
     *
     * @param globeControl The maply controller this is associated with.
     * @param selObjs The objects the user selected (e.g. MaplyScreenMarker).
     * @param loc The location they tapped on.  This is in radians.
     * @param screenLoc The location on the OpenGL surface.
     */
    public void userDidSelect(GlobeController globeControl,SelectedObject selObjs[],Point2d loc,Point2d screenLoc)
    {
    }

    /**
     * The user tapped somewhere, but not on a selectable object.
     *
     * @param globeControl The maply controller this is associated with.
     * @param loc The location they tapped on.  This is in radians.  If null, then the user tapped outside the globe.
     * @param screenLoc The location on the OpenGL surface.
     */
    public void userDidTap(GlobeController globeControl,Point2d loc,Point2d screenLoc)
    {
    }

    /**
     * The user tapped outside the globe.
     *
     * @param globeControl The maply controller this is associated with.
     * @param screenLoc The location on the OpenGL surface.
     */
    public void userDidTapOutside(GlobeController globeControl,Point2d screenLoc)
    {
    }

    /**
     * The user did long press somewhere, there might be an object
     * @param globeControl The maply controller this is associated with.
     * @param selObjs The objects the user selected (e.g. MaplyScreenMarker) or null if there was no object.
     * @param loc The location they tapped on.  This is in radians.  If null, then the user tapped outside the globe.
     * @param screenLoc The location on the OpenGL surface.
     */
    public void userDidLongPress(GlobeController globeControl, SelectedObject selObjs[], Point2d loc, Point2d screenLoc)
    {
    }

    /**
     * Called when the globe first starts moving.
     *
     * @param globeControl The globe controller this is associated with.
     * @param userMotion Set if the motion was caused by a gesture.
     */
    public void globeDidStartMoving(GlobeController globeControl, boolean userMotion)
    {
    }

    /**
     * Called when the globe stops moving.
     *
     * @param globeControl The globe controller this is associated with.
     * @param corners Corners of the viewport.  If one of them is null, that means it doesn't land on the globe.
     * @param userMotion Set if the motion was caused by a gesture.
     */
    public void globeDidStopMoving(GlobeController globeControl, Point3d corners[], boolean userMotion)
    {
    }

    /**
     * Called for every single visible frame of movement.  Be careful what you do in here.
     *
     * @param globeControl The globe controller this is associated with.
     * @param corners Corners of the viewport.  If one of them is null, that means it doesn't land on the globe.
     * @param userMotion Set if the motion was caused by a gesture.
     */
    public void globeDidMove(GlobeController globeControl,Point3d corners[], boolean userMotion)
    {
    }

}