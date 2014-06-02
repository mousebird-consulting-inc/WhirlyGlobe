/*
 *  PerformanceTimer.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 10/20/12.
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

<<<<<<< HEAD
#import <string>
#import <map>
#import "WhirlyTypes.h"
=======
#import <Foundation/Foundation.h>
#import <string>
#import <map>
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b

namespace WhirlyKit
{
    
/// Simple performance timing class
class PerformanceTimer
{
public:
    /// Used to track a category of timing
    class TimeEntry
    {
    public:
        TimeEntry();
        TimeEntry & operator = (const TimeEntry &that);
        bool operator < (const TimeEntry &that) const;
        
<<<<<<< HEAD
        void addTime(TimeInterval dur);
        
        std::string name;
        TimeInterval minDur,maxDur,avgDur;
=======
        void addTime(NSTimeInterval dur);
        
        std::string name;
        NSTimeInterval minDur,maxDur,avgDur;
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
        int numRuns;
    };
    
    /// Used to track a category of counts
    class CountEntry
    {
    public:
        CountEntry();
        bool operator < (const CountEntry &that) const;
        
        void addCount(int count);
        
        std::string name;
        int minCount,maxCount,avgCount;
        int numRuns;
    };
    
    /// Start timing the given thing
    void startTiming(const std::string &);
    
    /// Stop timing the given thing and add it to the existing timings
    void stopTiming(const std::string &);
    
    /// Add a count for a particular instance
    void addCount(const std::string &what,int count);
    
<<<<<<< HEAD
    /// Print out a string
    void report(const std::string &what);
    
=======
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
    /// Clean out existing timings
    void clear();
    
    /// Write out the timings to NSLog
    void log();
    
protected:
<<<<<<< HEAD
    std::map<std::string,TimeInterval> actives;
=======
    std::map<std::string,NSTimeInterval> actives;
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
    std::map<std::string,TimeEntry> timeEntries;
    std::map<std::string,CountEntry> countEntries;
};
    
}

