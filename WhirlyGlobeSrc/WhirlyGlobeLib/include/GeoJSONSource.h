/*
 *  GeoJSONSource.h
 *  WhirlyGlobeLib
 *
 *  Created by Ranen Ghosh on 4/4/17.
 *  Copyright 2011-2017 mousebird consulting
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

#import <string>
#import <vector>
#import "VectorObject.h"
#import "VectorData.h"


namespace WhirlyKit
{
    
/** TODO: GeoJSONSource description here.
 */
class GeoJSONSource
{
public:
    GeoJSONSource();
    ~GeoJSONSource();
    bool parseData(std::string json, std::vector<VectorObject *> &vecObjs);

private:
    void processPoints(const VectorPointsRef &points, std::vector<VectorObject *> &vecObjs);
    void processLinear(const VectorLinearRef &linear, std::vector<VectorObject *> &vecObjs);
    void processAreal(const VectorArealRef &areal, std::vector<VectorObject *> &vecObjs);
    
};
    
}
