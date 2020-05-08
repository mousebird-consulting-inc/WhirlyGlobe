/*
 *  InternalLabel.java
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

/**
 * An internal representation for the labels.  Toolkit users use ScreenLabel instead of this.
 * 
 */
class InternalLabel 
{
	InternalLabel()
	{
		initialise();
	}

	private void setupScreenLabel(ScreenLabel label,LabelInfo labelInfo)
	{
		if (label.selectable) {
			setSelectID(label.ident);
		}

		setLoc(label.loc);
		setRotation(label.rotation);
		if (label.text != null && !label.text.isEmpty()) {
			// Break up the text by newlines
			String parts[] = label.text.split("\n");

			for (String part : parts) {
				// Convert text over to code points
				int len = part.length();
				int[] codePoints = new int[len];
				int which = 0;
				for (int offset = 0; offset < len; ) {
					int codePoint = part.codePointAt(offset);
					codePoints[which++] = codePoint;
					offset += Character.charCount(codePoint);
				}
				addText(codePoints, which);
			}
		}
		if (label.layoutSize != null)
			setLayoutSize(label.layoutSize.getX(),label.layoutSize.getY());
		if (label.offset != null)
			setOffset(label.offset);

		if (labelInfo.layoutPlacement != -1)
			setLayoutImportance(labelInfo.layoutPlacement);
		if (label.layoutPlacement != -1)
			setLayoutPlacement(label.layoutPlacement);

		if (labelInfo.layoutImportance != Float.MAX_VALUE)
			setLayoutImportance(labelInfo.layoutImportance);
		if (label.layoutImportance != Float.MAX_VALUE)
			setLayoutImportance(label.layoutImportance);
		if (label.uniqueID != null)
			setUniqueID(label.uniqueID);
	}
	
	/**
	 * Construct from a ScreenLabel and LabelInfo.  Translates to internal values suitable
	 * for the rendering engine.
	 * 
	 * @param label Screen label we're going to represent.
	 */
	InternalLabel(ScreenLabel label,LabelInfo labelInfo)
	{
		initialise();

		setupScreenLabel(label,labelInfo);
	}

	/**
	 * Construct with a moving label.
	 */
	InternalLabel(ScreenMovingLabel label,LabelInfo labelInfo,double startTime)
	{
		initialise();

		setupScreenLabel(label,labelInfo);

		setEndLoc(label.endLoc);
		setAnimationRange(startTime,startTime+label.duration);
	}

	public native void setSelectID(long selectID);
	public native void setLoc(Point2d loc);
	public native void setEndLoc(Point2d loc);
	public native void setAnimationRange(double startTime,double endTime);
	public native void setRotation(double rotation);
	public native void setLockRotation(boolean lockRotation);
	public native void addText(int[] codePoints,int len);
	public native void setOffset(Point2d offset);
	public native void setLayoutImportance(float layoutImportance);
	public native void setLayoutPlacement(int layoutPlacement);
	public native void setLayoutSize(double sizeX,double sizeY);
	public native void setUniqueID(String uniqueStr);

	public void finalize()
	{
		dispose();
	}
	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise();
	native void dispose();
	private long nativeHandle;	
}
