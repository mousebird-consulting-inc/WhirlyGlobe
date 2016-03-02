/*
 *  MaplyAnnotation.m
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 12/13/13.
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

#import "MaplyAnnotation_private.h"
#import "WhirlyGlobe.h"

@implementation MaplyAnnotation

- (id)init
{
    self = [super init];
    _calloutView = [[SMCalloutView alloc] init];
    _minVis = WhirlyKit::DrawVisibleInvalid;
    _maxVis = WhirlyKit::DrawVisibleInvalid;
    _repositionForVisibility = true;
    
    return self;
}

- (void)setTitle:(NSString *)title
{
    _calloutView.title = title;
}

- (NSString *)title
{
    return _calloutView.title;
}

- (void)setSubTitle:(NSString *)subTitle
{
    _calloutView.subtitle = subTitle;
}

- (NSString *)subTitle
{
    return _calloutView.subtitle;
}

- (void)setLeftAccessoryView:(UIView *)leftAccessoryView
{
    _calloutView.leftAccessoryView = leftAccessoryView;
}

- (UIView *)leftAccessoryView
{
    return _calloutView.leftAccessoryView;
}

- (void)setRightAccessoryView:(UIView *)rightAccessoryView
{
    _calloutView.rightAccessoryView = rightAccessoryView;
}

- (UIView *)rightAccessoryView
{
    return _calloutView.rightAccessoryView;
}

- (void)setTitleView:(UIView *)titleView
{
    _calloutView.titleView = titleView;
}

- (UIView *)titleView
{
    return _calloutView.titleView;
}

- (void)setSubtitleView:(UIView *)subtitleView
{
    _calloutView.subtitleView = subtitleView;
}

- (UIView *)subtitleView
{
    return _calloutView.subtitleView;
}

- (void)setContentView:(UIView *)contentView
{
    _calloutView.contentView = contentView;
}

- (UIView *)contentView
{
    return _calloutView.contentView;
}

- (void)setLoc:(MaplyCoordinate)newLoc
{
    _loc = newLoc;
}

@end
