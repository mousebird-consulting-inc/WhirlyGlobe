/*  VectorObject
 *  com.mousebirdconsulting.maply
 *
 *  Created by Steve Gifford on 12/30/13.
 *  Copyright 2013-2022 mousebird consulting
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
 */

package com.mousebird.maply;

import androidx.annotation.Nullable;

import org.jetbrains.annotations.NotNull;

import java.util.Map;

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
@SuppressWarnings("unused")
public class VectorObject implements Iterable<VectorObject>
{
	/**
	 * Construct empty.
	 */
	public VectorObject() {
		initialise(ident);
	}

	/**
	 * Unique ID used by Maply for selection.
	 */
	long ident = Identifiable.genID();

	public enum MaplyVectorObjectType {
		MaplyVectorNoneType(0),
		MaplyVectorPointType(1),
		MaplyVectorLinearType(2),
		MaplyVectorLinear3dType(3),
		MaplyVectorArealType(4),
		MaplyVectorMultiType(5);
		private final int objType;
		MaplyVectorObjectType(int objType) {
			this.objType = objType;
		}
		public int getValue() {
			return objType;
		}
	}

	/**
	 * The vector type is one of point, linear, linear3d, areal, or a combination.
	 */
	public MaplyVectorObjectType getVectorType()
	{
		final int vectorType = getVectorTypeNative();
		for (MaplyVectorObjectType value : MaplyVectorObjectType.values()) {
			if (value.getValue() == vectorType) {
				return value;
			}
		}
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
	 * Return attributes for the feature.
	 * If there are multiple features, we get the first one.
	 * May return a copy of the internal attribute dictionary.
	 */
	@Nullable
	public native AttrDictionary getAttributes();

	/**
	 * Return attributes for the feature.
	 * If there are multiple features, we get the first one.
	 * Returns null if the internal attribute dictionary cannot be referenced.
	 */
	@Nullable
	public native AttrDictionary getAttributesRef();

	/**
	 * Reset the attributes for the feature.
	 */
	public native void setAttributes(@NotNull AttrDictionary attrs);
	
	/**
	 * Add a single point
	 */
	public native boolean addPoint(Point2d pt);

	/**
	 *  Add a linear feature
	 */
	public native boolean addLinear(Point2d[] pts);
	
	/**
	 *  Add an areal feature with one external loop.
	 */
	public native boolean addAreal(Point2d[] pts);

	/**
	 * Add an areal feature with a single exterior loop and one or more interior loops.
     */
	public native boolean addAreal(Point2d[] ext, Point2d[][] holes);

	/**
	 * Merge the vectors from the other vector object into this one.
	 */
	public native void mergeVectorsFrom(VectorObject other);

	/**
	 * Vector objects can be made of lots of smaller objects.  If you need to access
	 * each of this individually, this iterator will handle that efficiently.
	 */
	@Override
	public VectorIterator iterator() {
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
	 * Reverse the direction of areal loops in this object
	 */
	public native void reverseAreals();

	/**
	 * Create a copy with reversed areal loops
	 */
	public native VectorObject reversedAreals();

	/**
	 * Ensure that areal loops are closed
	 */
	public native void closeLoops();
	/**
	 * Produce a new objet with closed loops
	 */
	public native VectorObject closedLoops();

	/**
	 * Ensure that areal loops are not closed
	 */
	public native void unCloseLoops();
	/**
	 * Produce a new objet with un-closed loops
	 */
	public native VectorObject unClosedLoops();

	/**
	 * Returns the total number of points in a feature.  Used for assessing size.
     */
	public native int countPoints();

	/**
	 * Returns true if any of the segments of lines or areas intersect any others
	 */
	public native boolean anyIntersections();

	/**
	 * Bounding box of all the various features together
	 */
	public native boolean boundingBox(Point2d ll,Point2d ur);

	/**
	 Subdivide the edges in this feature to a given tolerance.

	 This will break up long edges in a vector until they lie flat on a globe to a given epsilon.
	 The epsilon is in display coordinates (radius = 1.0).
	 This routine breaks this up along geographic boundaries.
	 */
	public VectorObject subdivideToGlobe(double epsilon) {
		final VectorObject retVecObj = new VectorObject();
		return subdivideToGlobeNative(retVecObj,epsilon) ? retVecObj : null;
	}

	private native boolean subdivideToGlobeNative(VectorObject retVecObj,double epsilon);

	/**
	 Subdivide the edges in this feature to a given tolerance, using great circle math.

	 This will break up long edges in a vector until they lie flat on a globe to a given epsilon using a great circle route.  The epsilon is in display coordinates (radius = 1.0).
	 */
	public VectorObject subdivideToGlobeGreatCircle(double epsilon) {
		final VectorObject retVecObj = new VectorObject();
		return subdivideToGlobeGreatCircleNative(retVecObj,epsilon) ? retVecObj : null;
	}

	private native boolean subdivideToGlobeGreatCircleNative(VectorObject retVecObj,double epsilon);

	/**
	 Subdivide the edges in this feature to a given tolerance, using great circle math.

	 This version samples a great circle to display on a flat map.
	 */
	public VectorObject subdivideToFlatGreatCircle(double epsilon) {
		final VectorObject retVecObj = new VectorObject();
		return subdivideToFlatGreatCircleNative(retVecObj,epsilon) ? retVecObj : null;
	}

	private native boolean subdivideToFlatGreatCircleNative(VectorObject retVecObj,double epsilon);

	/**
	 Subdivide the edges in this feature to a given tolerance, using ellipsoidal math.

	 This will break up long edges in a vector until they lie flat on a globe to a given epsilon
	 using a great circle route.  The epsilon is in display coordinates (radius = 1.0).
	 */
	public VectorObject subdivideToGlobeGreatCirclePrecise(double epsilon) {
		final VectorObject retVecObj = new VectorObject();
		return subdivideToGlobeGreatCirclePreciseNative(retVecObj,epsilon) ? retVecObj : null;
	}

	private native boolean subdivideToGlobeGreatCirclePreciseNative(VectorObject retVecObj,double epsilon);

	/**
	 Subdivide the edges in this feature to a given tolerance, using ellipsoidal math.

	 This version samples a great circle to display on a flat map.
	 */
	public VectorObject subdivideToFlatGreatCirclePrecise(double epsilon) {
		final VectorObject retVecObj = new VectorObject();
		return subdivideToFlatGreatCirclePreciseNative(retVecObj,epsilon) ? retVecObj : null;
	}

	private native boolean subdivideToFlatGreatCirclePreciseNative(VectorObject retVecObj,double epsilon);

	/**
	 * Tessellate the areal features and return a new vector object.
	 */
	@Deprecated
	public VectorObject tesselate() {
		return tessellate();
	}
	public VectorObject tessellate() {
		final VectorObject retVecObj = new VectorObject();
		return tesselateNative(retVecObj) ? retVecObj : null;
	}

	private native boolean tesselateNative(VectorObject retVecObj);

	/**
	 * Clip the given areal features to a grid of the given size.
	 */
	public VectorObject clipToGrid(Point2d size) {
		final VectorObject retVecObj = new VectorObject();
		return clipToGridNative(retVecObj,size.getX(),size.getY()) ? retVecObj : null;
	}

	private native boolean clipToGridNative(VectorObject retVecObj,double sizeX,double sizeY);

	/**
	 * Clip the given features to an Mbr
	 */
	public VectorObject clipToMbr(Mbr mbr) {
		final VectorObject retVecObj = new VectorObject();
		return clipToMbrNative(retVecObj, mbr.ll.getX(), mbr.ll.getY(), mbr.ur.getX(), mbr.ur.getY()) ? retVecObj : null;
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
	public VectorObject reproject(CoordSystem srcSystem,double scale,CoordSystem destSystem) {
		final VectorObject retVecObj = new VectorObject();
		return reprojectNative(retVecObj,srcSystem,scale,destSystem) ? retVecObj : null;
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
	public VectorObject filterClippedEdges() {
		final VectorObject retVecObj = new VectorObject();
		return filterClippedEdgesNative(retVecObj) ? retVecObj : null;
	}

	private native boolean filterClippedEdgesNative(VectorObject vecObj);

	/**
	 * Convert any linears features into areals, by closing them and return
	 * a new vector object or null.
	 */
	public VectorObject linearsToAreals() {
		final VectorObject retVecObj = new VectorObject();
		return linearsToArealsNative(retVecObj) ? retVecObj : null;
	}

	private native boolean linearsToArealsNative(VectorObject vecObj);

	/**
	 * Convert any areal features to linears (just the outline) and return
	 * a new vector object or null.
	 */
	public VectorObject arealsToLinears() {
		final VectorObject retVecObj = new VectorObject();
		return arealsToLinearsNative(retVecObj) ? retVecObj : null;
	}

	private native boolean arealsToLinearsNative(VectorObject vecObj);

	/**
	 * Load vector objects from a GeoJSON string.
	 * @param json The GeoJSON string, presumably read from a file or over the network
	 * @return false if we were unable to parse the GeoJSON
	 */
	public native boolean fromGeoJSON(String json);

	/**
	 * Load vector objects from a GeoJSON string.
	 * @param json The GeoJSON string, presumably read from a file or over the network
	 * @return null if we were unable to parse the GeoJSON
	 */
	@Nullable
	public static VectorObject createFromGeoJSON(@NotNull String json) {
		VectorObject vo = new VectorObject();
		return vo.fromGeoJSON(json) ? vo : null;
	}

	/**
	 * Create a new vector object as a linear from a collection of points
	 */
	@Nullable
	public static VectorObject createLineString(@NotNull Point2d[] points) {
		return createLineString(points, null);
	}

	/**
	 * Create a new vector object as a linear from a collection of points and attributes
	 */
	@Nullable
	public static native VectorObject createLineString(@NotNull Point2d[] points,
	                                                   @Nullable AttrDictionary attrs);

	/**
	 * Create a new vector object as a polygon from a collection of points
	 */
	@Nullable
	public static VectorObject createAreal(@NotNull Point2d[] points) {
		return createAreal(points, null);
	}

	/**
	 * Create a new vector object as a polygon from a collection of points and attributes
	 */
	@Nullable
	public static native VectorObject createAreal(@NotNull Point2d[] points,
												  @Nullable AttrDictionary attrs);

	/**
	 * Load vector objects from a Shapefile.
	 * @param fileName The filename of the Shapefile.
	 * @return false if we were unable to parse the Shapefile.
	 */
	public native boolean fromShapeFile(String fileName);

	/**
	 * Indicates whether the object contains multiple elements that can be split up
	 */
	public native boolean canSplit();

	/**
	 * Split this feature into individual features.
	 *
	 * A vector object can represent multiple features.
	 * This method will make one vector object per feature, allowing you to operate on those individually.
	 *
	 * If the object does not contain multiple features, null is returned to avoid unnecessarily
	 * copying the shape contents.
	 */
	public VectorIterator splitVectors() {
		return canSplit() ? new VectorIterator(this) : null;
	}

	/**
	 * Make a complete copy of the vector object and return it.
	 */
	public VectorObject deepCopy() {
		final VectorObject vecObj = new VectorObject();
		deepCopyNative(vecObj);
		return vecObj;
	}

	protected native void deepCopyNative(VectorObject vecObj);

	/**
	 * Load vector objects from a GeoJSON assembly, which is just a bunch of GeoJSON stuck together.
	 */
	static public native Map<String,VectorObject> FromGeoJSONAssembly(String json);

	public void finalize() {
		dispose();
	}

	static {
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise(long ident);
	native void dispose();

	private long nativeHandle;
}
