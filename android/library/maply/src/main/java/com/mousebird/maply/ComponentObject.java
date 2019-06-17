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
	public ComponentObject()
	{
		initialise();
	}

	public void finalize()
	{
		dispose();
	}

	// Return the unique ID of this component object
	public native long getID();

	// Track selection IDs associated with this object
	public native void addSelectID(long id);

	public native long[] getSelectIDs();

	// Track the given marker ID as associated with us
	public native void addMarkerID(long id);

	// Track the given sticker ID as associated with us
	public native void addStickerID(long id);

	public native long[] getStickerIDs();

	// Track the given vector ID as associated with us
	public native void addVectorID(long id);

	public native void addLoftID(long id);

	public native long[] getVectorIDs();

	public native void addWideVectorID(long id);

	public native long[] getWideVectorIDs();

	// Track the given label ID as associated with us
	public native void addLabelID(long id);

	public native void addShapeID(long shapeId);

	public native void addBillboardID(long billId);

	public native void addParticleSystemID(long id);

	public native void addGeometryID(long id);

	public native void addVector(VectorObject vecObj);

	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise();
	native void dispose();
	private long nativeHandle;
}
