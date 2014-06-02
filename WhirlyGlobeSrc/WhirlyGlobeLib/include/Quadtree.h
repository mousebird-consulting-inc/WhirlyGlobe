/*
 *  QuadTree.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/28/11.
 *  Copyright 2012 mousebird consulting
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

#import "WhirlyVector.h"
#import "Dictionary.h"
#import <set>

<<<<<<< HEAD
=======
/// @cond
@class WhirlyKitViewState;
@protocol WhirlyKitQuadTreeImportanceDelegate;
/// @endcond

>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
namespace WhirlyKit
{

class QuadTreeImportanceCalculator;
class ViewState;
    
/** The Quadtree is used to represented the quad tree spatial subdivision
    algorithm.  This version tracks abstract representations of quad tree
    nodes.  It's up to the caller to track their own specific data along with
    it.
 */
class Quadtree
{
public:   
    /// Construct with the spatial information, number of nodes, min importance to consider
    ///  and a delegate to calculate importance.
    Quadtree(Mbr mbr,int minLevel,int maxLevel,int maxNodes,float minImportance,QuadTreeImportanceCalculator *importDelegate);
    ~Quadtree();

    /// Represents a single quad tree node
    class Identifier
    {
    public:
        Identifier() { }
        /// Construct with the cell coordinates and level.
        Identifier(int x,int y,int level) : x(x), y(y), level(level) { }
        
        /// Comparison based on x,y,level.  Used for sorting
        bool operator < (const Identifier &that) const;
        
        /// Spatial subdivision along the X axis relative to the space
        int x;
        /// Spatial subdivision along tye Y axis relative to the space
        int y;
        /// Level of detail, starting with 0 at the top (low)
        int level;
    };

    /// Quad tree node with bounding box and importance, which is possibly screen size
    class NodeInfo
    {
    public:
<<<<<<< HEAD
        NodeInfo() { }
        NodeInfo(const NodeInfo &that) : ident(that.ident), mbr(that.mbr), importance(that.importance), phantom(that.phantom), attrs(that.attrs) { }
=======
        NodeInfo() { attrs = [NSMutableDictionary dictionary]; }
        NodeInfo(const NodeInfo &that) : ident(that.ident), mbr(that.mbr), importance(that.importance) { attrs = [NSMutableDictionary dictionaryWithDictionary:that.attrs]; }
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
        ~NodeInfo() { }
        
        /// Compare based on importance.  Used for sorting
        bool operator < (const NodeInfo &that) const;
        
        /// Unique identifier for the particular node
        Identifier ident;
        /// Bounding box of just this quad node
        Mbr mbr;
        /// Importance as calculated by the callback.  More is better.
        float importance;
<<<<<<< HEAD
        /// Set if this is a phantom tile.  We pretended to load it, but it's not really here.
        bool phantom;

        /// Put any attributes you'd like to keep track of here.
        /// There are things you might calculate for a given tile over and over.
        Dictionary attrs;
=======

        /// Put any attributes you'd like to keep track of here.
        /// There are things you might calculate for a given tile over and over.
        NSMutableDictionary *attrs;
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
    };

    /// Check if the given tile is already present
    bool isTileLoaded(Identifier ident);

    /** Check if the quad tree will accept the given tile.
        This means either there's room or less important nodes loaded
        It could already be loaded.  Check that separately.
     */
    bool willAcceptTile(const NodeInfo &nodeInfo);
    
    /// Return true if this is a phantom tile
    bool isPhantom(const Identifier &ident);
    
    /// Set the phantom flag on the given node
    void setPhantom(const Identifier &nodeInfo,bool newPhantom);
    
    /// Recalculate the importance of everything.  This calls the callback.
    void reevaluateNodes();
    
    /// Add the given tile, keeping track of what needed to be removed
    void addTile(NodeInfo nodeInfo,std::vector<Identifier> &tilesRemoved);
    
    /// Explicitly remove a given tile
    void removeTile(Identifier which);
    
    // Remove the given tile
//    void removeTile(Identifier ident);

    /// Given an identifier, fill out the node info such as
    /// MBR and importance.
    NodeInfo generateNode(Identifier ident);

    /// Given the identifier of a parent, fill out the children IDs.
    /// This does not load them.
    void generateChildren(Identifier ident,std::vector<NodeInfo> &nodes);
    
    /// Return the loaded children of the given node.
    /// If the node isn't in the tree, return false
    bool childrenForNode(Identifier ident,std::vector<Identifier> &childIdents);
    
    /// Check if the given node has a parent loaded.
    /// Return true if so, false if not.
    bool hasParent(Identifier ident,Identifier &parentIdent);
    
    /// Check if the given node has children loaded
    bool hasChildren(Identifier ident);
    
    /// Generate an MBR for the given node identifier
    Mbr generateMbrForNode(Identifier ident);
    
    /// Fetch the least important (smallest) node currently loaded.
    /// Returns false if there wasn't one
<<<<<<< HEAD
    bool leastImportantNode(NodeInfo &nodeInfo,bool ignoreImportance=false,int targetLevel=-1);
=======
    bool leastImportantNode(NodeInfo &nodeInfo,bool ignoreImportance=false);
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b

    /// Return a vector of all nodes less than the given importance without children
    void unimportantNodes(std::vector<NodeInfo> &nodes,float importance);
    
    /// Update the maximum number of nodes.
    /// This won't check to see if we already have more than that
    void setMaxNodes(int newMaxNodes);
    
    /// Change the minimum importance value
    void setMinImportance(float newMinImportance);
    
    /// Dump out to the log for debugging
    void Print();
    
protected:
    class Node;

    // Sorter based on id
    typedef struct
    {
        bool operator() (const Node *a,const Node *b)
        {
            return a->nodeInfo.ident < b->nodeInfo.ident;
        }
    } NodeIdentSorter;

    // Sorter based on node importance
    typedef struct
    {
        bool operator() (const Node *a,const Node *b)
        {
            if (a->nodeInfo.importance == b->nodeInfo.importance)
                return a < b;
            return a->nodeInfo.importance < b->nodeInfo.importance;
        }
    } NodeSizeSorter;

    typedef std::set<Node *,NodeIdentSorter> NodesByIdentType;
    typedef std::set<Node *,NodeSizeSorter> NodesBySizeType;

    /// Single quad tree node with pointer to parent and children
    class Node
    {
        friend class Quadtree;
    public:
        Node(Quadtree *tree);
        
        NodeInfo nodeInfo;
        
        void addChild(Quadtree *tree,Node *child);
        void removeChild(Quadtree *tree,Node *child);
        bool hasChildren();
        void Print();
        
    protected:
        NodesByIdentType::iterator identPos;
        NodesBySizeType::iterator sizePos;
        Node *parent;
        Node *children[4];
    };
        
    Node *getNode(Identifier ident);
    void removeNode(Node *);

    Mbr mbr;
    int minLevel,maxLevel;
    int maxNodes;
    float minImportance;
    int numPhantomNodes;
    /// Used to calculate importance for a particular 
<<<<<<< HEAD
    QuadTreeImportanceCalculator *importDelegate;
=======
    NSObject<WhirlyKitQuadTreeImportanceDelegate> * __weak importDelegate;
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
    
    // All nodes, sorted by ID
    NodesByIdentType nodesByIdent;
    // Child nodes, sorted by importance
    NodesBySizeType nodesBySize;
};

/// Fill in this protocol to return the importance value for a given tile.
<<<<<<< HEAD
class QuadTreeImportanceCalculator
{
public:
    virtual ~QuadTreeImportanceCalculator() { }
    /// Return a number signifying importance.  MAXFLOAT is very important, 0 is not at all
    virtual double importanceForTile(const Quadtree::Identifier &ident,const Mbr &mbr,Quadtree *tree,Dictionary *attrs) = 0;
};
    
}
=======
@protocol WhirlyKitQuadTreeImportanceDelegate
/// Return a number signifying importance.  MAXFLOAT is very important, 0 is not at all
- (double)importanceForTile:(WhirlyKit::Quadtree::Identifier)ident mbr:(WhirlyKit::Mbr)mbr tree:(WhirlyKit::Quadtree *)tree attrs:(NSMutableDictionary *)attrs;
@end
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b

