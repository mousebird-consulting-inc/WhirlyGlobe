/*
 *  QuadTreeNew.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/26/18.
 *  Copyright 2012-2018 Saildrone Inc
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
#import <set>

namespace WhirlyKit
{
    
/// Represents a single quad tree node in simplified form
class QuadTreeIdentifier
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    QuadTreeIdentifier() { }
    /// Construct with the cell coordinates and level.
    QuadTreeIdentifier(int x,int y,int level) : x(x), y(y), level(level) { }

    static int64_t NodeNumber(int x, int y, int level)
    {
        const int64_t twoToZ = 1 << level;
        
        // total count of all levels below
        const int64_t baseCount = (twoToZ * twoToZ - 1) / 3;   // (4 ^ z - 1) / 3
        
        // number within this level
        const int64_t tileNumber = y * twoToZ + x;
        
        return baseCount + tileNumber;
    }
    int64_t NodeNumber() const { return NodeNumber(x,y,level); }

    /// Comparison based on x,y,level.  Used for sorting
    bool operator < (const QuadTreeIdentifier &that) const;
    
    /// Quality operator
    bool operator == (const QuadTreeIdentifier &that) const;
    
    /// Spatial subdivision along the X axis relative to the space
    int x;
    /// Spatial subdivision along tye Y axis relative to the space
    int y;
    /// Level of detail, starting with 0 at the top (low)
    int level;
};

/** New implementation of the spatial quad tree.
    Used to identify tiles to load and unload.
    This version keeps very little state.
  */
class QuadTreeNew
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    QuadTreeNew(const MbrD &mbr,int minLevel,int maxLevel);
    virtual ~QuadTreeNew() = default;
    
    // Single node in the Quad Tree
    class Node
    {
    public:
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

        Node() = default;
        Node(const QuadTreeIdentifier &that) : x(that.x), y(that.y), level(that.level) { }
        Node(const Node &that) : x(that.x), y(that.y), level(that.level) { }
        /// Construct with the cell coordinates and level.
        Node(int x,int y,int level) : x(x), y(y), level(level) { }
        
        /// Comparison based on x,y,level.  Used for sorting
        bool operator < (const Node &that) const;
        
        /// Equality operator
        bool operator == (const Node &that) const;
        
        /// Not equal operator
        bool operator != (const Node &that) const;
        
        int64_t NodeNumber() const { return QuadTreeIdentifier::NodeNumber(x,y,level); }
        
        /// Spatial subdivision along the X axis relative to the space
        int x;
        /// Spatial subdivision along tye Y axis relative to the space
        int y;
        /// Level of detail, starting with 0 at the top (low)
        int level;
    };
    typedef std::set<Node> NodeSet;

    // Node with an importance
    class ImportantNode : public Node
    {
    public:
        ImportantNode() : importance(0.0) { }
        ImportantNode(const ImportantNode &that) : Node((Node)that), importance(that.importance) { }
        ImportantNode(const Node &that,double import) : Node((Node)that), importance(import) { }
        ImportantNode(int x,int y,int level) : Node(x,y,level), importance(0.0) { }

        bool operator < (const ImportantNode &that) const;
        bool operator == (const ImportantNode &that) const;
        
        double importance;
    };
    typedef std::set<ImportantNode> ImportantNodeSet;

    // Calculate a set of nodes to load based on importance, but only up to the maximum
    // siblingNodes forces us to load all four children of a given parent
    ImportantNodeSet calcCoverageImportance(const std::vector<double> &minImportance,int maxNodes,
                                            bool siblingNodes,std::vector<double> &maxRejectedImport);
    
    /** Calculate the set of nodes to load based on importance.
        First figure out the highest level we could load.
        Try to load all visible tiles at that level.
        If it's too many, back off a level. Repeat.
      */
    std::tuple<int,ImportantNodeSet> calcCoverageVisible(const std::vector<double> &minImportance,
                                                         int maxNodes,const std::vector<int> &levelLoads,
                                                         bool keepMinLevel,std::vector<double> &maxRejectedImport);
    
    // Generate a bounding box 
    MbrD generateMbrForNode(const Node &node) const;
    
public:
    // Filled in by the subclass
    virtual double importance(const Node &node) = 0;
    virtual bool visible(const Node &node) = 0;
    
    // Recursively visit the quad tree evaluating as we go
    void evalNodeImportance(ImportantNode node,const std::vector<double> &minImportance,
                            ImportantNodeSet &importSet,std::vector<double> &maxRejectedImport);
    // This version uses pure visibility and goes down to a predefined level
    bool evalNodeVisible(ImportantNode node,const std::vector<double> &minImportance,int maxNodes,
                         const std::set<int> &levelsToLoad,int maxLevel,ImportantNodeSet &visibleSet);
    
    /// Bounding box
    MbrD mbr;
    
    /// Min/max zoom levels
    int minLevel,maxLevel;
};

}
