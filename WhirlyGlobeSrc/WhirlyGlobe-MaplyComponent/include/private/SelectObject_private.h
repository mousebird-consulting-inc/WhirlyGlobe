/*
 *  SelectObject_private.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 9/19/12.
 *  Copyright 2011-2015 mousebird consulting
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

#import <WhirlyGlobe.h>

// Used to map IDs to individual user objects (e.g. markers)
class SelectObject
{
public:
    SelectObject(WhirlyKit::SimpleIdentity selID) : selID(selID) { }
    SelectObject(WhirlyKit::SimpleIdentity selID,NSObject *obj) : selID(selID), obj(obj) { }
    
    // Comparison operator sorts on select ID
    bool operator < (const SelectObject &that) const
    {
        return selID < that.selID;
    }
        
    WhirlyKit::SimpleIdentity selID;
    NSObject * __strong obj;
};

typedef std::set<SelectObject> SelectObjectSet;
