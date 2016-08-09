/*
 *  QuadTree.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/28/11.
 *  Copyright 2012-2015 mousebird consulting
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
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    
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
        
        /// Quality operator
        bool operator == (const Identifier &that) const;
        
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
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
        
        NodeInfo() { phantom = false;  importance = 0; frameLoadingFlags = 0; childrenLoading = 0; childrenEval = 0; eval = false; failed = false; childCoverage = false; frameFlags = 0;}
        NodeInfo(const NodeInfo &that) : ident(that.ident), mbr(that.mbr), importance(that.importance),phantom(that.phantom),frameLoadingFlags(that.frameLoadingFlags),childrenLoading(that.childrenLoading),eval(that.eval), failed(that.failed), childrenEval(that.childrenEval), childCoverage(that.childCoverage), frameFlags(that.frameFlags), attrs(that.attrs) { }
        NodeInfo(const Identifier &ident) : ident(ident), importance(0.0), phantom(false), frameLoadingFlags(0), eval(false), failed(false), childrenLoading(0), childrenEval(0), childCoverage(false), frameFlags(0) { }
        NodeInfo & operator = (const NodeInfo &that) { ident = that.ident;  mbr = that.mbr;  importance = that.importance;  phantom = that.phantom; frameLoadingFlags = that.frameLoadingFlags; eval = that.eval;  failed = that.failed; childrenLoading = that.childrenLoading; childrenEval = that.childrenEval;  childCoverage = that.childCoverage; frameFlags = that.frameFlags; attrs = that.attrs; return *this; }
        ~NodeInfo() { }
        
        /// Compare based on importance.  Used for sorting
        bool operator < (const NodeInfo &that) const;
        
        /// Check if the given frame is loaded
        bool isFrameLoaded(int theFrame) const;
        
        /// Check if the given frame is loading
        bool isFrameLoading(int theFrame) const;
        
        /// Set the frame loading bit
        void setFrameLoading(int theFrame,bool val);

        /// Unique identifier for the particular node
        Identifier ident;
        /// Bounding box of just this quad node
        Mbr mbr;
        /// Importance as calculated by the callback.  More is better.
        float importance;
        /// Set if this is a phantom tile.  We pretended to load it, but it's not really here.
        bool phantom;
        /// Tile is in the process of evaluation
        bool eval;
        /// This node failed to load
        bool failed;
        /// Number of children in the process of loading
        int childrenLoading;
        /// Number of children being evalulated
        int childrenEval;
        /// This tile is covered by loaded children.
        bool childCoverage;
        /// 64 bits of frame flags
        long long frameFlags;
        /// 64 bits of frame loading flags
        long long frameLoadingFlags;

        /// Put any attributes you'd like to keep track of here.
        /// There are things you might calculate for a given tile over and over.
        Dictionary attrs;
    };

    /// Check if the given tile is already present
    bool isTilePresent(const Identifier &ident);

    /** Check if the quad tree will accept the given tile.
        This means either there's room or less important nodes loaded
        It could already be loaded.  Check that separately.
     */
    bool shouldLoadTile(const Identifier &ident,int frame);
    
    /// Return true if we've go the maximum nodes loaded in
    bool isFull();
    
    /// Return true if this is a phantom tile
    bool isPhantom(const Identifier &ident);
    
    /// Set the phantom flag on the given node
    void setPhantom(const Identifier &nodeInfo,bool newPhantom);
    
    /// Return true if this tile is loading
    bool isLoading(const Identifier &ident,int frame);
    
    /// Set the loading flag on the given node
    void setLoading(const Identifier &nodeInfo,int frame,bool newLoading);
    
    /// Mark a tile (with a frame) as loaded
    void didLoad(const Identifier &nodeInfo,int frame);
    
    /// Set if something is evaluating this node
    bool isEvaluating(const Identifier &ident);

    /// Set the evaluating flag
    void setEvaluating(const Identifier &nodeInfo,bool newEval);
    
    /// Check if a given tile failed to load at one point
    bool didFail(const Identifier &ident);
    
    /// Set the failed flag
    void setFailed(const Identifier &ident,bool newFail);
    
    /// Check if any of the children of the given node failed to load
    bool childFailed(const Identifier &ident);
    
    /// Number of nodes in the eval queue
    int numEvals();
    
    /// Clear everything that's evaluating
    void clearEvals();
    
    /// Clear all the failure flags
    void clearFails();
    
    /// Return the next nodes we're evaluating
    bool popLastEval(NodeInfo &);
    
    /// Look for children of this tile being loaded
    bool childrenLoading(const Identifier &ident);
    
    /// Look for children of this tile being evaluated
    bool childrenEvaluating(const Identifier &ident);
    
    /// Recalculate the importance of everything.  This calls the callback.
    void reevaluateNodes();
    
    /// Given an identifier, fill out the node info such as
    /// MBR and importance.
    NodeInfo generateNode(const Identifier &ident);
    
    /// Return the node info for a given node
    const NodeInfo *getNodeInfo(const Identifier &ident);
    
    /// Add the given tile, without looking for any to remove.  This is probably a phantom.
    const Quadtree::NodeInfo *addTile(const Identifier &ident,bool newEval,bool checkImportance,std::vector<Identifier> &newlyCoveredTiles);
    
    /// Explicitly remove a given tile
    void removeTile(const Identifier &which);
    
    /// Return the IDs for this node's children.  Doesn't check if they're there
    void childrenForNode(const Identifier &ident,std::vector<Identifier> &childIdents);
    
    /// Check if a parent is in the process of loading
    bool parentIsLoading(const Identifier &ident);
    
    /// Check if the given node has a parent loaded.
    /// Return true if so, false if not.
    bool hasParent(const Identifier &dent,Identifier &parentIdent);
    
    /// Check if the given node has children loaded
    bool hasChildren(const Identifier &ident);
    
    /// Generate an MBR for the given node identifier
    Mbr generateMbrForNode(const Identifier &ident);
    
    /// Generate a bounding box for a given node ID in 64 bit precision
    void generateMbrForNode(const Identifier &ident,Point2d &ll,Point2d &ur);
    
    /// Fetch the least important (smallest) node currently loaded.
    /// Returns false if there wasn't one
    bool leastImportantNode(NodeInfo &nodeInfo,bool force=false);
    
    /// Update the maximum number of nodes.
    /// This won't check to see if we already have more than that
    void setMaxNodes(int newMaxNodes);
    
    /// Change the minimum importance value
    void setMinImportance(float newMinImportance);
    
    /// Recalculate the child coverage for a given node
    void updateParentCoverage(const Identifier &ident,std::vector<Identifier> &coveredTiles,std::vector<Identifier> &unCoveredTiles);
    
    /// Return the loaded count for a given frame (if we're loading frames)
    int getFrameCount(int frame);
    
    /// Check if a given frame is completely loaded (if we're in frame mode)
    bool frameIsLoaded(int frame,int *tilesLoaded);
    
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
                return a->nodeInfo.ident < b->nodeInfo.ident;
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
        bool parentLoading();
        bool hasNonPhantomParent();
        void Print();
        /// Recalculate child coverage
        bool recalcCoverage();
 
    protected:
        NodesByIdentType::iterator identPos;
        NodesBySizeType::iterator sizePos;
        NodesBySizeType::iterator evalPos;
        Node *parent;
        Node *children[4];
        bool childOffscreen[4];
    };
        
    Node *getNode(const Identifier &ident);
    void removeNode(Node *);
    /// Recalculate child coverage for a node and its parents
    void recalcCoverage(Node *node);
    /// Add an entry for the given flag index
    void addFrameLoaded(int frame);
    /// Clear the flag counts for the given flag entries
    void clearFlagCounts(int frameFlags);

    Mbr mbr;
    int minLevel,maxLevel;
    int maxNodes;
    float minImportance;
    int numPhantomNodes;
    /// Used to calculate importance for a particular 
    QuadTreeImportanceCalculator *importDelegate;
    
    // All nodes, sorted by ID
    NodesByIdentType nodesByIdent;
    // Child nodes, sorted by importance
    NodesBySizeType nodesBySize;
    // Nodes we're evaluating
    NodesBySizeType evalNodes;
    std::vector<int> frameLoadCounts;
};

/// Fill in this protocol to return the importance value for a given tile.
class QuadTreeImportanceCalculator
{
public:
    virtual ~QuadTreeImportanceCalculator() { }
    /// Return a number signifying importance.  MAXFLOAT is very important, 0 is not at all
    /// 0 also means the tile is off screen
    virtual double importanceForTile(const Quadtree::Identifier &ident,const Mbr &mbr,Quadtree *tree,Dictionary *attrs) = 0;
};
    
}

