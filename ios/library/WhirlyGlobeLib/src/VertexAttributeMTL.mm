/*
 *  VertexAttributeMTL.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/16/19.
 *  Copyright 2011-2019 mousebird consulting
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

#import "VertexAttributeMTL.h"
#import "DrawableMTL.h"

namespace WhirlyKit
{
VertexAttributeMTL::VertexAttributeMTL(BDAttributeDataType dataType,StringIdentity nameID)
    : VertexAttribute(dataType,-1,nameID)
{
}

VertexAttributeMTL::VertexAttributeMTL(const VertexAttributeMTL &that)
    : VertexAttribute(that), buffer(that.buffer)
{
}

int VertexAttributeMTL::sizeMTL() const
{
    // Note: Should reference Metal info for this
    switch (dataType)
    {
        case BDFloat4Type:
            return 4*4;
            break;
        case BDFloat3Type:
            return 3*4;
            break;
        case BDChar4Type:
            return 4;
            break;
        case BDFloat2Type:
            return 2*4;
            break;
        case BDFloatType:
            return 4;
            break;
        case BDIntType:
            return 4;
            break;
        default:
            return 0;
    }
}

MTLVertexFormat VertexAttributeMTL::formatMTL() const
{
    switch (dataType)
    {
            
        case BDFloat4Type:
            return MTLVertexFormatFloat4;
            break;
        case BDFloat3Type:
            return MTLVertexFormatFloat3;
            break;
        case BDChar4Type:
            return MTLVertexFormatUChar4Normalized;
            break;
        case BDFloat2Type:
            return MTLVertexFormatFloat2;
            break;
        case BDFloatType:
            return MTLVertexFormatFloat;
            break;
        case BDIntType:
            return MTLVertexFormatInt;
            break;
        default:
            return MTLVertexFormatFloat;
    }
}

}
