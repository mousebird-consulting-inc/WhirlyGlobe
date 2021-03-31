/* SimpleStyle.kt
 * WhirlyGlobeLib
 *
 * Created by Tim Sylvester on 11/02/2021
 * Copyright © 2021 mousebird consulting, inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed
 * under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations under the License.
 */

package com.mousebird.maply

import android.util.Size
import androidx.annotation.ColorInt

class SimpleStyle {
    var title: String? = null
    var description: String? = null

    var markerSymbol: String? = null
    var backgroundSymbol: String? = null
    var markerString: String? = null
    var markerSize: Point2d? = null
    var markerCenter: Point2d? = null
    var backgroundCenter: Point2d? = null
    var markerOffset: Point2d? = null
    @ColorInt
    var markerColor: Int? = null
    var markerScale: Double? = null
    var backgroundScale: Double? = null

    var clearBackground: Boolean? = null
    
    var fillOpacity: Float? = null
    @ColorInt
    var fillColor: Int? = null
    
    var strokeWidth: Float? = null
    var strokeOpacity: Float? = null
    @ColorInt
    var strokeColor: Int? = null
    
    @ColorInt
    var labelColor: Int? = null
    var labelSize: Float? = null
    var labelOffset: Point2d? = null
    
    var layoutSize: Size? = null

    var markerTexture: MaplyTexture? = null
}