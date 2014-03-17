package com.mousebird.maply;

import java.util.ArrayList;

/**
 * The Component Object tracks the various geometry, textures, and outlines
 * associated with a given call to addVectors, addScreenMarkers, etc... in
 * the MaplyController.
 * <p>
 * Users of the toolkit keep these around to ask for modification or deletion
 * of their geometry later.  If they're never going to do that, they can
 * safely ignore these as well.
 * 
 * @author sjg
 *
 */
public class ComponentObject 
{
	// Convert from Long to long array
	long[] convertIDs(ArrayList<Long> inIDs)
	{
		int which = 0;
		long[] ids = new long[inIDs.size()];
		for (Long id : inIDs)
		{
			ids[which] = id;
			which++;
		}
		
		return ids;
	}
	
	// Track the given texture ID as belonging to us
	void addTexID(long id)
	{
		if (texIDs == null)
			texIDs = new ArrayList<Long>();
		texIDs.add(id);
	}

	// Track the given marker ID as associated with us
	void addMarkerID(long id)
	{
		if (markerIDs == null)
			markerIDs = new ArrayList<Long>();
		markerIDs.add(id);
	}

	// Track the given vector ID as associated with us
	void addVectorID(long id)
	{
		if (vectorIDs == null)
			vectorIDs = new ArrayList<Long>();
		vectorIDs.add(id);
	}
	
	// Enable/disable anything the component object is holding
	void enable(MaplyController control,boolean enable,ChangeSet changes)
	{	
		if (vectorIDs != null && vectorIDs.size() > 0)
		{
			control.vecManager.enableVectors(convertIDs(vectorIDs), enable, changes);
		}
		if (markerIDs != null && markerIDs.size() > 0)
			control.markerManager.enableMarkers(convertIDs(markerIDs), enable, changes);
	}
	
	// Clear out anything the component object is holding
	void clear(MaplyController control,ChangeSet changes)
	{
		if (vectorIDs != null && vectorIDs.size() > 0)
		{
			control.vecManager.removeVectors(convertIDs(vectorIDs), changes);
			vectorIDs.clear();
		}
		if (markerIDs != null && markerIDs.size() > 0)
		{
			control.markerManager.removeMarkers(convertIDs(markerIDs), changes);
			markerIDs.clear();
		}
		if (texIDs != null && texIDs.size() > 0)
			for (long texID: texIDs)
				control.texManager.removeTexture(texID, changes);
	}
	
	// Textures in use by this object
	private ArrayList<Long> texIDs;

	// Various render-side object types we're representing
	private ArrayList<Long> markerIDs;
	private ArrayList<Long> vectorIDs;
}
