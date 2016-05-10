/*
 *  ComponentObject.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2014 mousebird consulting
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
	
	// Track selection IDs associated with this object
	void addSelectID(long id)
	{
		if (selectIDs == null)
			selectIDs = new ArrayList<Long>();
		selectIDs.add(id);
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

	// Track the given sticker ID as associated with us
	void addStickerID(long id)
	{
		if (stickerIDs == null)
			stickerIDs = new ArrayList<Long>();
		stickerIDs.add(id);
	}

	long[] getStickerIDs()
	{
		if (stickerIDs == null)
			return null;
		long[] retIDs = new long[stickerIDs.size()];
		int which = 0;
		for (Long id : stickerIDs) {
			retIDs[which++] = id;
		}

		return retIDs;
	}

	// Track the given vector ID as associated with us
	void addVectorID(long id)
	{
		if (vectorIDs == null)
			vectorIDs = new ArrayList<Long>();
		vectorIDs.add(id);
	}

	long[] getVectorIDs()
	{
		if (vectorIDs == null)
			return null;
		long[] retIDs = new long[vectorIDs.size()];
		int which = 0;
		for (Long id : vectorIDs) {
			retIDs[which++] = id;
		}

		return retIDs;
	}

	// Track the given label ID as associated with us
	void addLabelID(long id)
	{
		if (labelIDs == null)
			labelIDs = new ArrayList<Long>();
		labelIDs.add(id);
	}

	public void addShapeID(long shapeId) {
		if (shapeID == null)
			shapeID = new ArrayList<>();
		shapeID.add(shapeId);
	}

	public void addBillboardID(long billId) {
		if (this.billIDs == null)
			this.billIDs = new ArrayList<>();
		billIDs.add(billId);
	}

	void addParticleSystemID(long id) {
		if (particleSystemIDs == null)
			particleSystemIDs = new ArrayList<Long>();
		particleSystemIDs.add(id);
	}

	long[] getParticleSystemIDs()
	{
		if (particleSystemIDs == null)
			return null;
		long[] retIDs = new long[particleSystemIDs.size()];
		int which = 0;
		for (Long id : particleSystemIDs) {
			retIDs[which++] = id;
		}

		return retIDs;
	}
	
	// Enable/disable anything the component object is holding
	void enable(MaplyBaseController control,boolean enable,ChangeSet changes)
	{	
		if (vectorIDs != null && vectorIDs.size() > 0)
			control.vecManager.enableVectors(convertIDs(vectorIDs), enable, changes);
		if (markerIDs != null && markerIDs.size() > 0)
			control.markerManager.enableMarkers(convertIDs(markerIDs), enable, changes);
		if (stickerIDs != null && stickerIDs.size() > 0)
			control.stickerManager.enableStickers(convertIDs(stickerIDs), enable, changes);
		if (labelIDs != null && labelIDs.size() > 0)
			control.labelManager.enableLabels(convertIDs(labelIDs), enable, changes);
		if (particleSystemIDs != null && particleSystemIDs.size() >0) {
			for (Long id : particleSystemIDs) {
				control.particleSystemManager.enableParticleSystem(id, enable, changes);
			}
		}
		if (shapeID != null && shapeID.size() > 0) {
			control.shapeManager.enableShapes(convertIDs(shapeID), enable, changes);
		}
		if (billIDs != null && billIDs.size() > 0) {
			control.billboardManager.enableBillboards(convertIDs(billIDs), enable, changes);
		}

	}
	
	// Clear out anything the component object is holding
	void clear(MaplyBaseController control,ChangeSet changes)
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
		if (stickerIDs != null && stickerIDs.size() > 0)
		{
			control.stickerManager.removeStickers(convertIDs(stickerIDs), changes);
			stickerIDs.clear();
		}
		if (labelIDs != null && labelIDs.size() > 0)
		{
			control.labelManager.removeLabels(convertIDs(labelIDs), changes);
			labelIDs.clear();
		}
		if (particleSystemIDs != null && particleSystemIDs.size() >0){
			for (Long id : particleSystemIDs){
				control.particleSystemManager.removeParticleSystem(id, changes);
			}
			particleSystemIDs.clear();
		}
		if (texIDs != null && texIDs.size() > 0) {
			for (long texID : texIDs)
				control.texManager.removeTexture(texID, changes);
			texIDs.clear();
		}

		if (shapeID != null && shapeID.size()>0) {
			control.shapeManager.removeShapes(convertIDs(shapeID), changes);
			shapeID.clear();
		}

		if (billIDs != null && billIDs.size() >0){
			control.billboardManager.removeBillboards(convertIDs(billIDs), changes);
			billIDs.clear();
		}
	}
	
	// Selection IDs associated with this object
	protected ArrayList<Long> selectIDs = null;
	
	// Textures in use by this object
	private ArrayList<Long> texIDs = null;

	// Various render-side object types we're representing
	private ArrayList<Long> markerIDs = null;
	private ArrayList<Long> stickerIDs = null;
	private ArrayList<Long> vectorIDs = null;
	private ArrayList<Long> labelIDs = null;
	private ArrayList<Long> particleSystemIDs = null;
	private ArrayList<Long> shapeID = null;
	private ArrayList<Long> billIDs = null;

}
