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
    var markerSize: Size? = null
    @ColorInt
    var markerColor: Int = 0
    @ColorInt
    var fillColor: Int = 0
    @ColorInt
    var strokeColor: Int = 0
}