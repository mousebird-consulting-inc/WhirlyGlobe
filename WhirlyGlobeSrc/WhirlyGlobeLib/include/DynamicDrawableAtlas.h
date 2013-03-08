/*
 *  DynamicDrawableAtlas.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/6/13.
 *  Copyright 2011-2013 mousebird consulting
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

#import "BigDrawable.h"

namespace WhirlyKit
{

/** The dynamic drawable atlas is used to consolidate drawables into a small
    set of big drawables that can be rendered quickly.
  */
class DynamicDrawableAtlas
{
public:
    /// Construct with a name (for debugging), individual vertex size, and
    ///  number of bytes for each big drawable
    DynamicDrawableAtlas(const std::string &name,int vertexSize,int numBytes);
    ~DynamicDrawableAtlas();
    
    /// Set the draw priority for any drawables we create
    void setDrawPriority(int newPriority) { drawPriority = newPriority; }
    
    /// Add the given drawable to the drawable atlas.
    /// Returns true on success.  Reference the drawable by its ID.
    bool addDrawable(BasicDrawable *draw,std::vector<ChangeRequest *> &changes);
    
    /// Remove the data for a drawable by ID
    bool removeDrawable(SimpleIdentity drawId,std::vector<ChangeRequest *> &changes);
    
    /// Flush out any outstanding changes.
    /// This may block waiting for the renderer
    void flush(std::vector<ChangeRequest *> &changes);
    
    /// Remove anything associated with the drawable atlas
    void shutdown(std::vector<ChangeRequest *> &changes);
    
protected:
    /// Used to track where a drawable wound up in a big drawable
    class DrawRepresent : public Identifiable
    {
    public:
        // Constructor for sorting
        DrawRepresent(SimpleIdentity theId) : Identifiable(theId), pos(0), size(0) { }
        
        /// Which big drawable this is in
        SimpleIdentity bigDrawId;
        
        /// Position and size within the big drawable (in bytes)
        int pos,size;
    };
    
    std::string name;
    int vertexSize;
    int numBytes;
    int drawPriority;
    
    typedef std::set<BigDrawable *,IdentifiableSorter> BigDrawableSet;
    BigDrawableSet bigDrawables;
    
    // Used to track where the individual drawables wound up
    typedef std::set<DrawRepresent> DrawRepresentSet;
    DrawRepresentSet drawables;
};
    
}

