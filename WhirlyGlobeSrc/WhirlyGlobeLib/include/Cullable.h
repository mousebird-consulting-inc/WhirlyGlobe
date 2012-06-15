/*
 *  Cullable.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/1/11.
 *  Copyright 2011 mousebird consulting
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

#import "Drawable.h"
#import "CoordSystem.h"

namespace WhirlyKit
{	
    
class Cullable;

/** This is the top level of the culling tree, represented by Cullables.
    In general, you should see this.  It's used by the Scene represented
    for scenegraph culling.
  */
class CullTree
{ 
    friend class Cullable;
public:
    CullTree(WhirlyKit::CoordSystem *coordSystem,Mbr localMbr,int depth,int maxDrawPerNode = 8);
    ~CullTree();
    
    Cullable *getTopCullable() { return topCullable; }
    int getCount() { return numCullables; }
    
protected:
    CoordSystem *coordSystem;
    Cullable *topCullable;
    int depth;
    int maxDrawPerNode;
    int numCullables;
};

/** This is a representation of cullable geometry.  It has
    geometry/direction info and a list of associated
    Drawables.
    Cullables are always rectangles in local space.
    In general, users shouldn't use these.  Your drawables
     will be sorted into them behind the scenes.
 */
class Cullable : public Identifiable
{
public:
    /// Construct recursively down to the given depth
	Cullable(WhirlyKit::CoordSystem *coordSystem,Mbr localMbr,int depth);
    ~Cullable();
	
	/// Add the given drawable to our set or the appropriate children
	void addDrawable(CullTree *cullTree,Mbr localMbr,DrawableRef drawable);
	
	/// Remove a given drawable if it's there
	void remDrawable(CullTree *cullTree,Mbr localMbr,DrawableRef drawable);
	
    /// Get the set of drawables associated with the cullable
	const std::set<DrawableRef,IdentifiableRefSorter> &getDrawables() const { return drawables; }
    
    /// Get the set of drawables for all the children.
    /// We cache them in the parent nodes for speed
	const std::set<DrawableRef,IdentifiableRefSorter> &getChildDrawables() const { return childDrawables; }
    
    /// Return true if there are any children
    bool hasChildren() { return children[0] || children[1] || children[2] || children[3]; }
    
    /// Return true if there are no drawables here (or in children)
    bool isEmpty() { return drawables.empty() && childDrawables.empty(); }
    
    /// Return the Nth child.  Might be null.
    Cullable *getChild(int which) { return children[which]; }
    
    /// Get the bounding box for this cullable
	Mbr getMbr() const { return localMbr; }
        
public:	
    Cullable *getOrAddChild(int which,CullTree *tree);
    void possibleRemoveChild(int which,CullTree *tree);
    void addDrawableToChildren(CullTree *cullTree,Mbr drawLocalMbr,DrawableRef draw);
    void split(CullTree *);
    
    /// 3D locations (in model space) of the corners
	Point3f cornerPoints[4];
	/// Normal vectors (in model space) for the corners
	Vector3f cornerNorms[4];
    /// Opposite of depth.  0 means go no lower
    int height;
    /// Local coordinates for bounding box
    Mbr localMbr;
    /// Bounding boxes for each of the children
    Mbr childMbr[4];
	
    Cullable *children[4];
	std::set<DrawableRef,IdentifiableRefSorter> drawables;
    std::set<DrawableRef,IdentifiableRefSorter> childDrawables;
};

}
