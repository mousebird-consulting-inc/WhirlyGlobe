/*
 *  PerformanceTimer.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 10/20/12.
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
#import <vector>
#import <algorithm>
#import "PerformanceTimer.h"
#import "Platform.h"
#if defined(__ANDROID__)
#import <android/log.h>
#else
// Note: Porting
//#import <Foundation/Foundation.h>
#endif

namespace WhirlyKit
{
    
PerformanceTimer::TimeEntry::TimeEntry()
{
    name = "";
    minDur = MAXFLOAT;
    maxDur = 0.0;
    avgDur = 0.0;
    numRuns = 0;
}
    
PerformanceTimer::TimeEntry & PerformanceTimer::TimeEntry::operator = (const TimeEntry &that)
{
    name = that.name;
    minDur = that.minDur;
    maxDur = that.maxDur;
    avgDur = that.avgDur;
    numRuns = that.numRuns;
    
    return *this;
}


bool PerformanceTimer::TimeEntry::operator<(const WhirlyKit::PerformanceTimer::TimeEntry &that) const
{
    return name < that.name;
}

void PerformanceTimer::TimeEntry::addTime(TimeInterval dur)
{
    minDur = std::min(minDur,dur);
    maxDur = std::max(maxDur,dur);
    avgDur += dur;
    numRuns++;
}

PerformanceTimer::CountEntry::CountEntry()
{
    name = "";
    minCount = 1<<30;
    maxCount = 0;
    avgCount = 0;
    numRuns = 0;
}

bool PerformanceTimer::CountEntry::operator<(const WhirlyKit::PerformanceTimer::CountEntry &that) const
{
    return name < that.name;
}

void PerformanceTimer::CountEntry::addCount(int count)
{
    minCount = std::min(minCount,count);
    maxCount = std::max(maxCount,count);
    avgCount += count;
    numRuns++;
}

void PerformanceTimer::startTiming(const std::string &what)
{
    actives[what] = TimeGetCurrent();
}

void PerformanceTimer::stopTiming(const std::string &what)
{
    std::map<std::string,TimeInterval>::iterator it = actives.find(what);
    if (it == actives.end())
        return;
    TimeInterval start = it->second;
    actives.erase(it);
    
    std::map<std::string,TimeEntry>::iterator eit = timeEntries.find(what);
    if (eit != timeEntries.end())
        eit->second.addTime(TimeGetCurrent()-start);
    else {
        TimeEntry newEntry;
        newEntry.addTime(TimeGetCurrent()-start);
        newEntry.name = what;
        timeEntries[what] = newEntry;
    }
}

void PerformanceTimer::addCount(const std::string &what,int count)
{
    std::map<std::string,CountEntry>::iterator it = countEntries.find(what);
    if (it != countEntries.end())
        it->second.addCount(count);
    else {
        CountEntry newEntry;
        newEntry.addCount(count);
        newEntry.name = what;
        countEntries[what] = newEntry;
    }
}

void PerformanceTimer::clear()
{
    actives.clear();
    timeEntries.clear();
    countEntries.clear();
}

static bool TimeEntryByMax (const PerformanceTimer::TimeEntry &a,const PerformanceTimer::TimeEntry &b)
{
    return a.avgDur > b.avgDur;
}
    
void PerformanceTimer::report(const std::string &what)
{
#if defined(__ANDROID__)
    __android_log_print(ANDROID_LOG_VERBOSE, "Maply Performance", "%s", what.c_str());
#else
    // Note: Porting
//    NSLog("%s",what.c_str());
#endif
}
    
void PerformanceTimer::log()
{
    std::vector<TimeEntry> sortedEntries;
    sortedEntries.reserve(timeEntries.size());
    
    for (std::map<std::string,TimeEntry>::iterator it = timeEntries.begin();
         it != timeEntries.end(); ++it)
        sortedEntries.push_back(it->second);
    std::sort(sortedEntries.begin(),sortedEntries.end(),TimeEntryByMax);
    for (unsigned int ii=0;ii<sortedEntries.size();ii++)
    {
        TimeEntry &entry = sortedEntries[ii];
        if (entry.numRuns > 0)
        {
            char line[1024];
            sprintf(line,"%s: min, max, avg = (%.2f,%.2f,%.2f) ms",entry.name.c_str(),1000*entry.minDur,1000*entry.maxDur,1000*entry.avgDur / entry.numRuns);
            report(line);
        }
    }
    for (std::map<std::string,CountEntry>::iterator it = countEntries.begin();
         it != countEntries.end(); ++it)
    {
        CountEntry &entry = it->second;
        if (entry.numRuns > 0)
        {
            char line[1024];
            sprintf(line,"%s: min, max, avg = (%d,%d,%2.f,  %d) count",entry.name.c_str(),entry.minCount,entry.maxCount,(float)entry.avgCount / (float)entry.numRuns,entry.avgCount);
        }
    }
}
    
}
