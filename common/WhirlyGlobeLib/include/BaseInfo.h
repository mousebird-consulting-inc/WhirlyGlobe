/*
 *  BaseInfo.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/6/15.
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

#import <math.h>
#import <set>
#import <map>
#import <string>
#import "Identifiable.h"
#import "Dictionary.h"
#import "WhirlyVector.h"
#import "WhirlyTypes.h"
#import "Drawable.h"
#import "VertexAttribute.h"

namespace WhirlyKit
{
class BasicDrawableBuilder;
typedef std::shared_ptr<BasicDrawableBuilder> BasicDrawableBuilderRef;
class BasicDrawableInstanceBuilder;
typedef std::shared_ptr<BasicDrawableInstanceBuilder> BasicDrawableInstanceBuilderRef;

/// Types of expressions we'll support for certain fields
typedef enum {ExpressionNone,ExpressionLinear,ExpressionExponential} ExpressionInfoType;

/// Base class for expressions
class ExpressionInfo : public Identifiable
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    ExpressionInfo();
    ExpressionInfo(const ExpressionInfo &that);
    
    ExpressionInfoType type;
    
    float base;  // Used for exponential expressions
    std::vector<float> stopInputs;
};

/// Single float expression (e.g. opacity or what have you)
class FloatExpressionInfo: public ExpressionInfo
{
public:
    FloatExpressionInfo() = default;
    FloatExpressionInfo(const FloatExpressionInfo &that) = default;
    
    // Scale the outputs by the given value
    void scaleBy(double scale);

    // Evaluate the expression at the given zoom level
    float evaluate(float zoom, float defaultValue);

    std::vector<float> stopOutputs;
};
typedef std::shared_ptr<FloatExpressionInfo> FloatExpressionInfoRef;

/// Color expression (e.g. for continuous color changes)
class ColorExpressionInfo: public ExpressionInfo
{
public:
    ColorExpressionInfo() = default;
    ColorExpressionInfo(const ColorExpressionInfo &that) = default;

    // Evaluate the expression at the given zoom level
    RGBAColor evaluate(float zoom, RGBAColor defaultValue);

    // Evaluate directly to floats, with no truncation to integer components
    Eigen::Vector4f evaluateF(float zoom, RGBAColor defaultValue);

    std::vector<RGBAColor> stopOutputs;
};
typedef std::shared_ptr<ColorExpressionInfo> ColorExpressionInfoRef;

/** Object use as the base for parsing description dictionaries.
 */
class BaseInfo
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    BaseInfo();
    BaseInfo(const BaseInfo &that);
    BaseInfo(const Dictionary &dict);
    
    // Convert contents to a string for debugging
    virtual std::string toString();
    
    /// Set the various parameters on a basic drawable
    void setupBasicDrawable(BasicDrawableBuilder *drawBuild) const;
    void setupBasicDrawable(const BasicDrawableBuilderRef &drawBuild) const;

    /// Set the various parameters on a basic drawable instance
    void setupBasicDrawableInstance(BasicDrawableInstanceBuilder *drawBuild) const;
    void setupBasicDrawableInstance(const BasicDrawableInstanceBuilderRef &drawBuild) const;

    double minVis,maxVis;
    double minVisBand,maxVisBand;
    double minViewerDist,maxViewerDist;
    int zoomSlot;
    double minZoomVis,maxZoomVis;
    Point3d viewerCenter;
    double drawOffset;
    int64_t drawOrder = DrawOrderTiles;
    int drawPriority;
    bool enable;
    double fade;
    double fadeIn;
    double fadeOut;
    TimeInterval fadeOutTime;
    TimeInterval startEnable,endEnable;
    SimpleIdentity programID;
    int extraFrames;
    bool zBufferRead,zBufferWrite;
    SimpleIdentity renderTargetID;
    bool hasExp;   // Set if we're requiring the expressions to be passed through (problem on Metal)
    
    SingleVertexAttributeSet uniforms;

    // 2^48 = 0x1000000000000 = 281474976710656
    // This is enough to enumerate all the tiles in all levels 0 through 23 (inclusive), which has
    // a resolution of about 2cm/pixel.
    // There's room for 64K of these within drawOrder's 64 bits.
    static const int64_t DrawOrderTileBlock = 1LL << 48;

    static const int64_t DrawOrderTiles = 0;
    static const int64_t DrawOrderBeforeTiles = 1LL << 60;
    static const int64_t DrawOrderAfterTiles = -1;
};
typedef std::shared_ptr<BaseInfo> BaseInfoRef;

}
