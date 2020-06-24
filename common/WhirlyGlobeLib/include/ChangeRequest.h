/*
 *  ChangeRequest.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/8/19.
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

#import <vector>
#import <set>
#import <map>
#import "Identifiable.h"
#import "StringIndexer.h"
#import "WhirlyKitView.h"

namespace WhirlyKit
{

/** Base class for render related setup information.
    This might include version for OpenGL ES.
  */
class RenderSetupInfo
{
public:
};
    
class Scene;
class SceneRenderer;
    
/** This is the base clase for a change request.  Change requests
 are how we modify things in the scene.  The renderer is running
 on the main thread and we want to keep our interaction with it
 very simple.  So instead of deleting things or modifying them
 directly, we ask the renderer to do so through a change request.
 */
class ChangeRequest
{
public:
    ChangeRequest();
    virtual ~ChangeRequest();
    
    /// Return true if this change requires a GL Flush in the thread it was executed in
    virtual bool needsFlush();
    
    /// Fill this in to set up whatever resources we need on the GL side
    virtual void setupForRenderer(const RenderSetupInfo *);
    
    /// Make a change to the scene.  For the renderer.  Never call this.
    virtual void execute(Scene *scene,SceneRenderer *renderer,View *view) = 0;
    
    /// Set this if you need to be run before the active models are run
    virtual bool needPreExecute();
    
    /// If non-zero we'll execute this request after the given absolute time
    TimeInterval when;
};

/// Representation of a list of changes.  Might get more complex in the future.
typedef std::vector<ChangeRequest *> ChangeSet;
typedef std::shared_ptr<ChangeSet> ChangeSetRef;

typedef struct
{
    bool operator () (const ChangeRequest *a,const ChangeRequest *b) const
    {
        if (a->when == b->when)
            return a < b;
        return a->when < b->when;
    }
} ChangeSorter;
/// This version is sorted by when to run it
typedef std::set<ChangeRequest *,ChangeSorter> SortedChangeSet;
    
}
