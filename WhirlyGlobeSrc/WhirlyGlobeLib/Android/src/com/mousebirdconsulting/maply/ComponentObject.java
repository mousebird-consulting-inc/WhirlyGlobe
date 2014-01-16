package com.mousebirdconsulting.maply;

import java.util.ArrayList;

public class ComponentObject 
{
	// Convert from Long to long array
	long[] convertIDs(ArrayList<Long> inIDs)
	{
		int which = 0;
		long[] ids = new long[vectorIDs.size()];
		for (Long id : ids)
		{
			ids[which] = id;
			which++;
		}
		
		return ids;
	}
	
	public void addTexID(long id)
	{
		if (texIDs == null)
			texIDs = new ArrayList<Long>();
		texIDs.add(id);
	}

	public void addMarkerID(long id)
	{
		if (markerIDs == null)
			markerIDs = new ArrayList<Long>();
		markerIDs.add(id);
	}

	public void addVectorID(long id)
	{
		if (vectorIDs == null)
			vectorIDs = new ArrayList<Long>();
		vectorIDs.add(id);
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
