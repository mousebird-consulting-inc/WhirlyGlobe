/*
 *  Identifiable.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/7/11.
 *  Copyright 2011-2016 mousebird consulting
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

#import <set>
#import <memory>

namespace WhirlyKit
{

/** Simple Identities are just numbers we use to refer to objects within the
    rendering system.  The idea is that some operations are dangerous enough
    with multiple threads (and prone to error) that it's just safer to request
    an operation on a given ID rather than letting the developer muck around
    in the internals.
  */
typedef unsigned long long SimpleIdentity;
    
/// This is the standard empty identity.  It means there were none of something
///  or it's just ignored.
static const SimpleIdentity EmptyIdentity = 0;
    
/// A set of identities.  Often passed back as query result.
typedef std::set<SimpleIdentity> SimpleIDSet;

/** Simple unique ID base class.
    If you subclass this you'll get your own unique ID
    for the given object instance.  See the SimpleIdentity
    for an explanation of why we use these.
 */
class Identifiable
{
public:
	/// Construct with a new ID
	Identifiable();
    /// Construct with an existing ID.  Used for searching mostly.
    Identifiable(SimpleIdentity oldId) : myId(oldId) { }
	virtual ~Identifiable() { }
	
	/// Return the identity
	SimpleIdentity getId() const { return myId; }
	
	/// Think carefully before setting this
    /// In most cases you should be using the one you inherit
	void setId(SimpleIdentity inId) { myId = inId; }

	/// Generate a new ID without an object.
    /// We use this in cases where we're going to be creating an
    ///  Identifiable subclass, but haven't yet.
	static SimpleIdentity genId();
    
    /// Used for sorting
    bool operator < (const Identifiable &that) const { return myId < that.myId; }
		
protected:
	SimpleIdentity myId;
};

/// Reference counted version of Identifiable
typedef std::shared_ptr<Identifiable> IdentifiableRef;
	
/// Used to sort identifiables in a set or similar STL container
typedef struct
{
	bool operator () (const Identifiable *a,const Identifiable *b) const { return a->getId() < b->getId(); }
} IdentifiableSorter;
    
/// Used to sort identifiable Refs in a container
typedef struct
{
	bool operator () (const IdentifiableRef a,const IdentifiableRef b) const { return a->getId() < b->getId(); }
} IdentifiableRefSorter;

}

