/*
 *  ShapeReader.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/1/11.
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

#import <math.h>
#import "VectorData.h"
#import "GlobeMath.h"

namespace WhirlyKit
{

/** Shape File Reader.
	Open a shapefile and return the features as requested.
 */
class ShapeReader : public VectorReader
{
public:
    /// Construct with a file name
	ShapeReader(const std::string &fileName);
	virtual ~ShapeReader();
	
	/// Return true if we managed to load the file
	virtual bool isValid();
	
	/// Return the next feature
	virtual VectorShapeRef getNextObject(const StringSet *filterAttrs);
    
    /// We can do random seeking
    virtual bool canReadByIndex() { return true; }
    
    /// The total number of shapes in the file
    virtual unsigned int getNumObjects();

    /// Fetch an object by the index
    virtual VectorShapeRef getObjectByIndex(unsigned int vecIndex,const StringSet *filter);
    
protected:
	void *shp;
	void *dbf;
	int where,numEntity,shapeType;
	double minBound[4], maxBound[4];
};

}
