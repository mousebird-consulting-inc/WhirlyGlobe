/* LegendEntry.kt
 * AutoTesterAndroid.maply
 *
 * Created by Tim Sylvester on 30/03/2021
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

import android.graphics.Bitmap

class LegendEntry(
    val name: String,
    val ident: String?,
    val image: Bitmap?,
    var entries: Collection<LegendEntry>
    ) {

    override fun toString(): String {
        return printString("")
    }

    fun printString(indent: String): String {
        var retStr =  "$indent name = $name\n"
        entries.forEach {
            retStr += it.printString(indent + " ") + "\n"
        }

        return retStr
    }
}
