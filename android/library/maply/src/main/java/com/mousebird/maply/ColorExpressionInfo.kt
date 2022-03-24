/* ColorExpressionInfo.kt
 * AutoTesterAndroid.maply
 *
 * Created by Tim Sylvester on 22/03/2022
 * Copyright © 2022 mousebird consulting, inc.
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

import android.graphics.Color
import android.os.Build
import androidx.annotation.*

class ColorExpressionInfo {

    fun finalize() {
        dispose()
    }

    external fun dispose()
    
    companion object {
        @RequiresApi(Build.VERSION_CODES.O)
        fun createLinear(minZoom: Float, minColor: Color,
                         maxZoom: Float, maxColor: Color): ColorExpressionInfo? =
            createLinear(minZoom, minColor.toArgb(), maxZoom, maxColor.toArgb())
    
        @JvmStatic
        external fun createLinear(minZoom: Float, @ColorInt minColor: Int,
                                  maxZoom: Float, @ColorInt maxColor: Int): ColorExpressionInfo?
        @JvmStatic
        private external fun nativeInit()
        
        init {
            nativeInit()
        }
    }
    
    @Keep
    @Suppress("unused")     // Used by JNI
    private var nativeHandle: Long = 0
}