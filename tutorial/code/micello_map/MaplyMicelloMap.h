/*
 *  MaplyMicelloMap.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Ranen Ghosh on 4/12/16.
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

#import "Foundation/Foundation.h"
#import <WhirlyGlobeComponent.h>

/** @brief An entity of interest in a community that corresponds to a geometry in the map.
    @details If an entity is selected by the user, you can use the MaplyMicelloMapEntity object given to you to access information about that entity.
    @see MaplyMicelloMap select:viewC:
 */
@interface  MaplyMicelloMapEntity : NSObject

@property (readonly, nonatomic, assign) int entityID;
@property (readonly, nonatomic, strong) NSDictionary *_Nonnull properties;
@property (readonly, nonatomic, strong) NSString *_Nullable intAddress;
@property (readonly, nonatomic, assign) double lonDeg;
@property (readonly, nonatomic, assign) double latDeg;

@end

/** @brief A level is a subset of a drawing sharing a common vertical alignment.
    @details You probably won't need to use this directly.
 */
@interface  MaplyMicelloMapLevel : NSObject

@property (readonly, nonatomic, assign) int levelID;
@property (readonly, nonatomic, strong) NSString *_Nonnull name;
@property (readonly, nonatomic, assign) int zLevel;
@property (readonly, nonatomic, assign) bool isMain;

/// @brief An array of the MaplyVectorObjects created by parsing the geometries belonging to this level.
@property (readonly, nonatomic, strong) NSArray *_Nonnull features;

@end

/** @brief A subset of the geometries in a Micello map.
    @details You probably won't need to use this directly.
 */
@interface  MaplyMicelloMapDrawing : NSObject

@property (readonly, nonatomic, assign) int drawingID;
@property (readonly, nonatomic, assign) bool isRoot;
@property (readonly, nonatomic, strong) NSString *_Nonnull name;
@property (readonly, nonatomic, strong) NSString *_Nonnull displayName;
@property (readonly, nonatomic, strong) NSString *_Nonnull mapType;

/// @brief A dictionary mapping level ID to level, for levels in this drawing.
@property (readonly, nonatomic, strong) NSDictionary *_Nonnull levels;
@property (readonly, nonatomic, strong) MaplyMicelloMapLevel *_Nullable mainLevel;

@end

/** @brief A simple rule to apply Maply vector attributes to features that match a criterion.
    @details A geometry will receive the vector attributes if, in its properties, the value corresponding to the provided key matches the provided value.
    @details The vector attributes correspond to what MaplyBaseViewController addVectors:desc:mode:
    @details At this point kMaplyColor and kMaplyDrawPriority work.  The draw priority may be important to display geometries in the proper z-order.
    @see MaplyBaseViewController addVectors:desc:mode:
 */
@interface  MaplyMicelloStyleRule : NSObject

/** @brief Initialize a MaplyMicelloStyleRule object.
    @param key The key in the geometry properties to look up.
    @param value The value in the geometry properties to match.
    @param desc The Maply vector attributes to apply if the rule matches.
 */
- (nullable instancetype)initWithKey:(NSString *__nonnull)key value:(NSObject *__nonnull)value desc:(NSDictionary *__nonnull)desc;

/// @brief The key in the geometry properties to look up.
@property (nonatomic, strong) NSString *_Nonnull key;

/// @brief The value in the geometry properties to match.
@property (nonatomic, strong) NSObject *_Nonnull value;

/// @brief The Maply vector attributes to apply if the rule matches.
@property (nonatomic, strong) NSDictionary *_Nonnull desc;

@end

/** @brief Represents a Micello Community Map.
    @details A MaplyMicelloMap object allows retrieval and display of a Micello Community Map.
    @details Instantiate a MaplyMicelloMap and call startFetchMapWithSuccess:failure:.  In the success block, set any desired display properties, add any desired style rules, and set the desired z-level of the map.
 */
@interface  MaplyMicelloMap : NSObject

/// @brief A dictionary mapping to drawings from their drawing IDs.
/// @details Read-only; available after calling startFetchMapWithSuccess:failure:.
@property (readonly, nonatomic, strong) NSDictionary *_Nonnull drawings;

/// @brief The root drawing of the community map
/// @details Read-only; available after calling startFetchMapWithSuccess:failure:.
@property (readonly, nonatomic, strong) MaplyMicelloMapDrawing *_Nullable rootDrawing;

/// @brief A dictionary mapping to entities from their entity IDs.
/// @details Read-only; available after calling startFetchMapWithSuccess:failure:.
@property (readonly, nonatomic, strong) NSDictionary *_Nonnull entities;

/// @brief The longitude coordinate, in degrees, of the center of the community map.
/// @details Read-only; available after calling startFetchMapWithSuccess:failure:.
@property (readonly, nonatomic, assign) double centerLonDeg;

/// @brief The latitude coordinate, in degrees, of the center of the community map.
/// @details Read-only; available after calling startFetchMapWithSuccess:failure:.
@property (readonly, nonatomic, assign) double centerLatDeg;

/// @brief A sorted array of the z-levels available in the community map.
/// @details (An array of NSNumbers)
/// @details Read-only; available after calling startFetchMapWithSuccess:failure:.
@property (readonly, nonatomic, strong) NSArray *_Nonnull zLevels;

/// @brief A dictionary mapping z-levels to arrays of MaplyMicelloMapLevel objects.
/// @details A dictionary mapping each available z-level to an unsorted array of the corresponding MaplyMicelloMapLevel objects
/// @details Read-only; available after calling startFetchMapWithSuccess:failure:.
@property (readonly, nonatomic, strong) NSDictionary *_Nonnull zLevelsToLevels;

/// @brief The current z-level of the community map.  If not yet set, this equals -1.
/// @details Read-only; available after calling startFetchMapWithSuccess:failure:.
@property (readonly, nonatomic, assign) int zLevel;

/// @brief Set to change the default fill color.
/// @details Set to change the default fill color (when no style rules match).  Defaults to white.
/// @details Display property; set before calling setZLevel:viewC:.
@property (nonatomic, strong) UIColor *_Nonnull fillColor;

/// @brief Set to change the outline color; defaults to black.
/// @details Display property; set before calling setZLevel:viewC:.
@property (nonatomic, strong) UIColor *_Nonnull outlineColor;

/// @brief Set to change the selected outline color; defaults to blue.
/// @details Display property; set before calling setZLevel:viewC:.
@property (nonatomic, strong) UIColor *_Nonnull selectedOutlineColor;

/// @brief Set to change the outline width; defaults to 3.0.
/// @details Display property; set before calling setZLevel:viewC:.
@property (nonatomic, assign) float lineWidth;

/// @brief Set to change the selected outline width; defaults to 10.0.
/// @details Display property; set before calling setZLevel:viewC:.
@property (nonatomic, assign) float selectedLineWidth;


/** @brief Initialize a MaplyMicelloMap object.
    @details Initialize a MaplyMicelloMap object with the necessary parameters.
    @param baseURL The base URL; the common prefix shared by the URLS for Community Map file, the Community Entity file, and the Level Geometry files.
    @param projectKey The secret key associated with your Micello account and project.
    @param baseDrawPriority The minimum WhirlyGlobe-Maply draw priority value to reserve for this community map. A range of values should be reserved; the highest is this value plus 50. Do not use values in that range for other elements of the WhirlyGlobe-Maply map.
 */
- (nullable instancetype)initWithBaseURL:(NSString *__nonnull)baseURL projectKey:(NSString *__nonnull)projectKey baseDrawPriority:(int)baseDrawPriority;

/** @brief Start the fetching of the Community Map.
    @details This method starts fetching the data for the community map including drawings, levels, and entities. When it's time to set the initial z-level, or if you want to inspect that data, do so in the success block, as this operation is asynchronous.
    @param successBlock The code block to be executed upon successful loading of the community map.
    @param failureBlock The code block to be executed in the event of failure.
 */
- (void)startFetchMapWithSuccess:(nonnull void (^)()) successBlock failure:(nullable void(^)(NSError *__nonnull error)) failureBlock;

/** @brief Add a style rule to the Community Map.
    @details Add one or more style rules before setting the z-level of the map. Rules are evaluated in the order in which they are added, so in the case of ambiguity add more important rules first.
    @details The vector attributes correspond to what MaplyBaseViewController addVectors:desc:mode:
    @details At this point kMaplyColor and kMaplyDrawPriority work.  The draw priority may be important to display geometries in the proper z-order.
    @param styleRule The style rule to add.
    @see MaplyMicelloStyleRule
 */
- (void)addStyleRule:(MaplyMicelloStyleRule *__nonnull)styleRule;

/** @brief Add default style rules to the Community Map.
 */
- (void)addDefaultStyleRules;

/** @brief Set the Z-level of the Community Map.
    @details Calling this method will fetch all geometries for levels of all drawings matching the provided z-level, and present them on the globe/map.
    @param The z-level to be set.
    @param viewC the Maply view controller (globe or map)
 */
- (void)setZLevel:(int)zLevel viewC:(MaplyBaseViewController *__nonnull)viewC;

/** @brief Select an entity of the Community Map.
    @details Call this from the relevant didSelect method of the globe or map view controller.  If an entity of the Micello Community Map was selected, the entity will be returned, otherwise this returns nil.
 */
- (MaplyMicelloMapEntity *__nullable)select:(NSObject *__nonnull) selectedObj viewC:(MaplyBaseViewController *__nonnull)viewC;

/** @brief Clear any selection in the Community Map.
 */
- (void)clearSelectionViewC:(MaplyBaseViewController *__nonnull)viewC;

@end
