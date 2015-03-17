/*
 *  ViewState.java
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
 * The view state encapsulates what's in a view at a certain point in time.
 * It's here so we can pass that around without fear of making a mess.
 * <p>
 * In general, toolkit users shouldn't need to interact with these.
 * 
 */
class ViewState 
{
	protected ViewState()
	{
	}
	
	/**
	 * Compare two view states and return true if they're equal.
	 * 
	 * @param viewState View state to compare.
	 * @return Return true if the view states are the same.
	 */
	public native boolean isEqual(ViewState viewState);

	private long nativeHandle;
}
