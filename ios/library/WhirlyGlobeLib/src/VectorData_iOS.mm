/*
 *  VectorData_iOS.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/6/19.
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

#import "VectorData_iOS.h"
#import "Dictionary_NSDictionary.h"

namespace WhirlyKit
{

// Parse a single coordinate out of an array
bool VectorParseCoord(Point2f &coord,NSArray *coords)
{
    if (![coords isKindOfClass:[NSArray class]] || ([coords count] != 2 && [coords count] != 3))
        return false;
    coord.x() = DegToRad([[coords objectAtIndex:0] floatValue]);
    coord.y() = DegToRad([[coords objectAtIndex:1] floatValue]);
    
    return true;
}

// Parse coordinates out of a coordinate string
bool VectorParseCoords(VectorRing &coords,NSArray *coordArray)
{
    if (![coordArray isKindOfClass:[NSArray class]])
        return false;
    
    // Look at the type of the first object.  If it's not an array, we've got a coord.
    NSObject *firstObj = [coordArray objectAtIndex:0];
    if (![firstObj isKindOfClass:[NSArray class]])
    {
        coords.resize(1);
        if (!VectorParseCoord(coords[0], coordArray))
            return false;
    } else {
        coords.resize([coordArray count]);
        int ci = 0;
        for (NSArray *coord in coordArray)
        {
            if (!VectorParseCoord(coords[ci], coord))
                return false;
            ci++;
        }
    }
    
    return true;
}

// Parse geometry objects out of the JSON
bool VectorParseGeometry(ShapeSet &shapes,NSDictionary *jsonDict)
{
    NSString *type = [jsonDict objectForKey:@"type"];
    if (![type isKindOfClass:[NSString class]])
        return false;
    
    if (![type compare:@"Point"])
    {
        VectorRing coords;
        if (!VectorParseCoords(coords,[jsonDict objectForKey:@"coordinates"]))
            return false;
        if (coords.size() != 1)
            return false;
        VectorPointsRef pts = VectorPoints::createPoints();
        pts->pts.push_back(coords[0]);
        pts->initGeoMbr();
        shapes.insert(pts);
    } else if (![type compare:@"LineString"])
    {
        VectorRing coords;
        if (!VectorParseCoords(coords,[jsonDict objectForKey:@"coordinates"]))
            return false;
        if (coords.empty())
            return false;
        VectorLinearRef lin = VectorLinear::createLinear();
        lin->pts = coords;
        lin->initGeoMbr();
        shapes.insert(lin);
    } else if (![type compare:@"Polygon"])
    {
        NSArray *coordsArray = [jsonDict objectForKey:@"coordinates"];
        if (![coordsArray isKindOfClass:[NSArray class]])
            return false;
        VectorArealRef ar = VectorAreal::createAreal();
        for (NSArray *coordsEntry in coordsArray)
        {
            VectorRing coords;
            if (!VectorParseCoords(coords, coordsEntry))
                return false;
            if (coords.empty())
                return false;
            ar->loops.push_back(coords);
        }
        ar->initGeoMbr();
        shapes.insert(ar);
    } else if (![type compare:@"MultiPoint"])
    {
        VectorRing coords;
        if (!VectorParseCoords(coords,[jsonDict objectForKey:@"coordinates"]))
            return false;
        if (coords.empty())
            return false;
        VectorPointsRef pts = VectorPoints::createPoints();
        pts->pts = coords;
        pts->initGeoMbr();
        shapes.insert(pts);
    } else if (![type compare:@"MultiLineString"])
    {
        NSArray *coordsArray = [jsonDict objectForKey:@"coordinates"];
        if (![coordsArray isKindOfClass:[NSArray class]])
            return false;
        for (NSArray *coordsEntry in coordsArray)
        {
            VectorRing coords;
            if (!VectorParseCoords(coords, coordsEntry) || coords.empty())
                return false;
            VectorLinearRef lin = VectorLinear::createLinear();
            lin->pts = coords;
            lin->initGeoMbr();
            shapes.insert(lin);
        }
    } else if (![type compare:@"MultiPolygon"])
    {
        NSArray *polyArray = [jsonDict objectForKey:@"coordinates"];
        if (![polyArray isKindOfClass:[NSArray class]])
            return false;
        for (NSArray *polyEntry in polyArray)
        {
            if ([polyEntry isKindOfClass:[NSArray class]])
            {
                VectorArealRef ar = VectorAreal::createAreal();
                for (NSArray *coordsEntry in polyEntry)
                {
                    VectorRing coords;
                    if (!VectorParseCoords(coords, coordsEntry) || coords.empty())
                        return false;
                    ar->loops.push_back(coords);
                }
                ar->initGeoMbr();
                shapes.insert(ar);
            } else
                return false;
        }
    } else if (![type compare:@"GeometryCollection"])
    {
        // Recurse down for the other geometry
        NSArray *geom = [jsonDict objectForKey:@"geometries"];
        if (![geom isKindOfClass:[NSArray class]])
            return false;
        for (NSDictionary *geomDict in geom)
        {
            if (![geomDict isKindOfClass:[NSDictionary class]])
                return false;
            if (!VectorParseGeometry(shapes, geomDict))
                return false;
        }
    } else
        return false;
    
    return true;
}

// Parse a single feature out of geoJSON
bool VectorParseFeature(ShapeSet &shapes,NSDictionary *jsonDict)
{
    NSString *idStr = [jsonDict objectForKey:@"id"];
    NSDictionary *geom = [jsonDict objectForKey:@"geometry"];
    NSDictionary *prop = [jsonDict objectForKey:@"properties"];
    
    if (![geom isKindOfClass:[NSDictionary class]])
        return false;
    
    // Parse out the geometry.  May result in multiple shapes
    if (!VectorParseGeometry(shapes, geom))
        return false;
    
    // Apply the attributes if there are any
    if ([prop isKindOfClass:[NSDictionary class]])
        for (ShapeSet::iterator it = shapes.begin(); it != shapes.end(); ++it) {
            iosMutableDictionary *dict = new iosMutableDictionary([NSMutableDictionary dictionaryWithDictionary:prop]);
            (*it)->setAttrDict(MutableDictionaryRef(dict));
        }
    
    // Apply the identity if there is one
    if ([idStr isKindOfClass:[NSString class]])
        for (ShapeSet::iterator it = shapes.begin(); it != shapes.end(); ++it)
        {
            iosMutableDictionary *dict = (iosMutableDictionary *)(*it)->getAttrDict().get();
            [dict->dict setObject:idStr forKey:@"id"];
        }
    
    return true;
}

// Parse a set of features out of GeoJSON using an NSDictionary
bool VectorParseGeoJSON(ShapeSet &shapes,NSDictionary *jsonDict)
{
    NSString *type = [jsonDict objectForKey:@"type"];
    if (![type isKindOfClass:[NSString class]])
        return false;
    
    if (![type compare:@"FeatureCollection"])
    {
        NSArray *features = [jsonDict objectForKey:@"features"];
        if (![features isKindOfClass:[NSArray class]])
            return false;
        
        for (NSDictionary *featDict in features)
        {
            if (![featDict isKindOfClass:[NSDictionary class]])
                return false;
            
            ShapeSet featShapes;
            if (VectorParseFeature(featShapes,featDict))
                shapes.insert(featShapes.begin(),featShapes.end());
            else
                return false;
        }
    }
    
    return true;
}

}
