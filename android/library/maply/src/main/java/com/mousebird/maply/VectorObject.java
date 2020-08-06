/*
 *  VectorObject
 *  com.mousebirdconsulting.maply
 *
 *  Created by Steve Gifford on 12/30/13.
 *  Copyright 2013-2014 mousebird consulting
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

import java.util.Map;
import java.util.Vector;

/**
 * The Maply VectorObject represents a group of vector features.  There can be a single point in here,
 * multiple points, or even a combination of points, areals, and linears.
 * <p>
 * You can create these yourself, but they are typically read from data files, such as GeoJSON or
 * Shapefiles.
 * <p>
 * The VectorObject is meant to be somewhat opaque (but not as opaque as originally planned).  If you
 * need to do a lot of manipulation of your vector data, it's best ot do it elsewhere and then convert
 * to a VectorObject.
 * 
 * @author sjg
 *
 */
public class VectorObject implements Iterable<VectorObject>
{
	/**
	 * Construct empty.
	 */
	public VectorObject()
	{
		initialise(ident);
	}

	/**
	 * Unique ID used by Maply for selection.
	 */
	long ident = Identifiable.genID();

	public enum MaplyVectorObjectType {
		MaplyVectorNoneType(0), MaplyVectorPointType(1), MaplyVectorLinearType(2), MaplyVectorLinear3dType(3), MaplyVectorArealType(4), MaplyVectorMultiType(5);
		private final int objType;
		MaplyVectorObjectType(int objType) {this.objType = objType;}
		public int getValue() { return objType;}
	};

	/**
	 * The vector type is one of point, linear, linear3d, areal, or a combination.
	 */
	public MaplyVectorObjectType getVectorType()
	{
		int vectorType = getVectorTypeNative();
		if (vectorType == MaplyVectorObjectType.MaplyVectorNoneType.getValue())
			return MaplyVectorObjectType.MaplyVectorNoneType;
		else if (vectorType == MaplyVectorObjectType.MaplyVectorPointType.getValue())
			return MaplyVectorObjectType.MaplyVectorPointType;
		else if (vectorType == MaplyVectorObjectType.MaplyVectorLinearType.getValue())
			return MaplyVectorObjectType.MaplyVectorLinearType;
		else if (vectorType == MaplyVectorObjectType.MaplyVectorLinear3dType.getValue())
			return MaplyVectorObjectType.MaplyVectorLinear3dType;
		else if (vectorType == MaplyVectorObjectType.MaplyVectorArealType.getValue())
			return MaplyVectorObjectType.MaplyVectorArealType;
		else if (vectorType == MaplyVectorObjectType.MaplyVectorMultiType.getValue())
			return MaplyVectorObjectType.MaplyVectorMultiType;
		else
			return MaplyVectorObjectType.MaplyVectorNoneType;
	}
	private native int getVectorTypeNative();

	/**
	 * Turn this on if you want the vector object to be selectable.
	 *
	 */
	public native void setSelectable(boolean newSelect);

	/**
	 * On if this features is selectable.
	 */
	public native boolean getSelectable();

	/**
	 * Return attributes for the feature.  If there are multiple features, we get the first one.
	 */
	public native AttrDictionary getAttributes();

	/**
	 * Reset the attributes for the feature.
	 */
	public native void setAttributes(AttrDictionary attrs);
	
	/**
	 * Add a single point
	 */
	public native void addPoint(Point2d pt);

	/**
	 *  Add a linear feature
	 */
	public native void addLinear(Point2d pts[]);
	
	/**
	 *  Add an areal feature with one external loop.
	 */
	public native void addAreal(Point2d pts[]);

	/**
	 * Add an areal feature with a single exterior loop and one or more interior loops.
     */
	public native void addAreal(Point2d ext[],Point2d holes[][]);

	/**
	 * Merge the vectors from the other vector object into this one.
	 */
	public native void mergeVectorsFrom(VectorObject other);

	/**
	 * Vector objects can be made of lots of smaller objects.  If you need to access
	 * each of this individually, this iterator will handle that efficiently.
	 */
	@Override
	public VectorIterator iterator() 
	{
		return new VectorIterator(this);
	}

	/**
	 * Calculate the center (not the centroid).
	 */
	public native Point2d center();

	/**
	 * Calculate the centroid of the object.  This assumes we've got at least one areal loop.
	 * 
	 * @return Centroid of the object.
	 */
	public native Point2d centroid();

	/**
	 * Find the largest loop and return its center.  Also returns the bounding box the loop.
	 * 
	 * @param ll Lower left corner of the largest loop.
	 * @param ur Upper right corner of the largest loop.
	 * @return Center of the largest loop.
	 */
	public native Point2d largestLoopCenter(Point2d ll,Point2d ur);
	
	/**
	 * Calculate the midpoint of a multi-point line.  Also return the rotation.
	 * 
	 * @param middle Midpoint of the line
	 * @return Orientation along the long at that point
	 */
	public native double linearMiddle(Point2d middle);

	/**
	 * Calculate the midpoint of a multi-point line.  Also return the rotation.
	 * Does its work in the coordinate system passed in
	 *
	 * @param middle Midpoint of the line
	 * @param coordSys Coordinate system to project into first.
	 * @return Orientation along the long at that point
	 */
	public native double linearMiddle(Point2d middle,CoordSystem coordSys);

	/**
	 * Return the point right in the middle (index-wise) of a linear features.
	 * @param middle Return point
	 * @return True if this was a linear
	 */
	public native boolean middleCoordinate(Point2d middle);

	/**
	 * Return true if the given point (in geo radians) is inside the vector feature.
	 * Only makes sense with areals.
     */
	public native boolean pointInside(Point2d pt);

	/**
	 * Area of all the out loops together.
	 */
	public native double areaOfOuterLoops();

	/**
	 * Returns the total number of points in a feature.  Used for assessing size.
     */
	public native int countPoints();

	/**
	 * Bounding box of all the various features together
	 */
	public native boolean boundingBox(Point2d ll,Point2d ur);

	/**
	 Subdivide the edges in this feature to a given tolerance.

	 This will break up long edges in a vector until they lie flat on a globe to a given epsilon.  The epislon is in display coordinates (radius = 1.0). This routine breaks this up along geographic boundaries.
	 */
	public VectorObject subdivideToGlobe(double epsilon)
	{
		VectorObject retVecObj = new VectorObject();
		if (!subdivideToGlobeNative(retVecObj,epsilon))
			return null;

		return retVecObj;
	}

	private native boolean subdivideToGlobeNative(VectorObject retVecObj,double epsilon);

	/**
	 Subdivide the edges in this feature to a given tolerance, using great circle math.

	 This will break up long edges in a vector until they lie flat on a globe to a given epsilon using a great circle route.  The epsilon is in display coordinates (radius = 1.0).
	 */
	public VectorObject subdivideToGlobeGreatCircle(double epsilon)
	{
		VectorObject retVecObj = new VectorObject();
		if (!subdivideToGlobeGreatCircleNative(retVecObj,epsilon))
			return null;

		return retVecObj;
	}

	private native boolean subdivideToGlobeGreatCircleNative(VectorObject retVecObj,double epsilon);

	/**
	 Subdivide the edges in this feature to a given tolerance, using great circle math.

	 This version samples a great circle to display on a flat map.
	 */
	public VectorObject subdivideToFlatGreatCircle(double epsilon)
	{
		VectorObject retVecObj = new VectorObject();
		if (!subdivideToFlatGreatCircleNative(retVecObj,epsilon))
			return null;

		return retVecObj;
	}

	private native boolean subdivideToFlatGreatCircleNative(VectorObject retVecObj,double epsilon);

	/**
	 * Tesselate the areal features and return a new vector object.
	 */
	public VectorObject tesselate()
	{
		VectorObject retVecObj = new VectorObject();
		if (!tesselateNative(retVecObj))
			return null;

		return retVecObj;
	}

	private native boolean tesselateNative(VectorObject retVecObj);

	/**
	 * Clip the given areal features to a grid of the given size.
	 */
	public VectorObject clipToGrid(Point2d size)
	{
		VectorObject retVecObj = new VectorObject();
		if (!clipToGridNative(retVecObj,size.getX(),size.getY()))
			return null;

		return retVecObj;
	}

	private native boolean clipToGridNative(VectorObject retVecObj,double sizeX,double sizeY);

	/**
	 * Clip the given features to an Mbr
	 */
	public VectorObject clipToMbr(Mbr mbr)
	{
		VectorObject retVecObj = new VectorObject();
		if (!clipToMbrNative(retVecObj, mbr.ll.getX(), mbr.ll.getY(), mbr.ur.getX(), mbr.ur.getY()))
			return null;

		return retVecObj;
    }

	private native boolean clipToMbrNative(VectorObject retVecObj,double llX,double llY, double urX, double urY);

	/**
	 * Reproject the vectors from the given system (geographic, by default) into
	 * the destination coordinate system.  We don't explicitly track units, so if the coordinates
	 * need to be scaled, pass that in as well.
	 *
	 * @param srcSystem Source coordinate system (that the data is already in)
	 * @param scale Scale factor to apply to the coordinates. 1.0 is a good default.
	 * @param destSystem Destination coordinate system that we'll project data into.
	 * @return The new vector object or null.
	 */
	public VectorObject reproject(CoordSystem srcSystem,double scale,CoordSystem destSystem)
	{
		VectorObject retVecObj = new VectorObject();
		if (!reprojectNative(retVecObj,srcSystem,scale,destSystem))
			return null;

		return retVecObj;
	}

	private native boolean reprojectNative(VectorObject vecObj,CoordSystem srcSystem,double scale,CoordSystem destSystem);

	/**
	 * Filter out edges created from clipping areal features on the server.
	 *
	 * In some very specific cases (OSM water) we get polygons that are obviously clipped
	 * along internal boundaries.  We can clear this up with some very, very specific logic.
	 *
	 * Input must be closed areals and output is linears.
	 *
	 * @return The filtered vector object or null.
	 */
	public VectorObject filterClippedEdges()
	{
		VectorObject retVecObj = new VectorObject();
		if (!filterClippedEdgesNative(retVecObj))
			return null;
		return retVecObj;
	}

	private native boolean filterClippedEdgesNative(VectorObject vecObj);

	/**
	 * Convert any linears features into areals, by closing them and return
	 * a new vector object or null.
	 */
	public VectorObject linearsToAreals()
	{
		VectorObject retVecObj = new VectorObject();
		if (!linearsToArealsNative(retVecObj))
			return null;
		return retVecObj;
	}

	private native boolean linearsToArealsNative(VectorObject vecObj);

	/**
	 * Convert any areal features to linears (just the outline) and return
	 * a new vector object or null.
	 */
	public VectorObject arealsToLinears()
	{
		VectorObject retVecObj = new VectorObject();
		if (!arealsToLinearsNative(retVecObj))
			return null;
		return retVecObj;
	}

	private native boolean arealsToLinearsNative(VectorObject vecObj);

	/**
	 * Load vector objects from a GeoJSON string.
	 * @param json The GeoSJON string, presumably read from a file or over the network
	 * @return false if we were unable to parse the GeoJSON
	 */
	public native boolean fromGeoJSON(String json);

	/**
	 * Load vector objects from a Shapefile.
	 * @param fileName The filename of the Shapefile.
	 * @return false if we were unable to parse the Shapefile.
	 */
	public native boolean fromShapeFile(String fileName);

	/**
	 * Make a complete copy of the vector object and return it.
	 */
	public VectorObject deepCopy()
	{
		VectorObject vecObj = new VectorObject();
		deepCopyNative(vecObj);

		return vecObj;
	}

	protected native void deepCopyNative(VectorObject vecObj);

	/**
	 * Load vector objects from a GeoJSON assembly, which is just a bunch of GeoJSON stuck together.
	 * @param json
	 * @return
	 */
	static public native Map<String,VectorObject> FromGeoJSONAssembly(String json);

	public void finalize()
	{
		dispose();
	}

	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise(long ident);
	native void dispose();

	private long nativeHandle;
}
