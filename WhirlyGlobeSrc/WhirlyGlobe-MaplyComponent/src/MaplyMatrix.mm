/*
 *  MaplyMatrix.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 10/16/14.
 *  Copyright 2011-2014 mousebird consulting
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

#import "MaplyMatrix_private.h"

using namespace Eigen;

@implementation MaplyMatrix

- (id)init
{
    self = [super init];
    _mat = _mat.Identity();
    
    return self;
}

- (id)initWithYaw:(double)alpha pitch:(double)gamma roll:(double)beta
{
    self = [super init];
    _mat = _mat.Identity();
    
    double sinAlpha = sin(-alpha);
    double cosAlpha = cos(-alpha);
    double sinBeta = sin(beta);
    double cosBeta = cos(beta);
    double sinGamma = sin(gamma);
    double cosGamma = cos(gamma);
    
    _mat(0,0) = cosAlpha * cosBeta;
    _mat(0,1) = cosAlpha * sinBeta * sinGamma - sinAlpha * cosGamma;
    _mat(0,2) = cosAlpha * sinBeta * cosGamma + sinAlpha * sinGamma;
    _mat(0,3) = 0.0;
    
    _mat(1,0) = sinAlpha * cosBeta;
    _mat(1,1) = sinAlpha * sinBeta * sinGamma + cosAlpha * cosGamma;
    _mat(1,2) = sinAlpha * sinBeta * cosGamma - cosAlpha * sinGamma;
    _mat(1,3) = 0.0;
    
    _mat(2,0) = -sinBeta;
    _mat(2,1) = cosBeta * sinGamma;
    _mat(2,2) = cosBeta * cosGamma;
    _mat(2,3) = 0.0;
    
    _mat(3,0) = 0.0;
    _mat(3,1) = 0.0;
    _mat(3,2) = 0.0;
    _mat(3,3) = 1.0;
    
    return self;
}

- (id)initWithScale:(double)scale
{
    self = [super init];
    _mat = _mat.Identity();
    
    _mat(0,0) = scale;
    _mat(1,1) = scale;
    _mat(2,2) = scale;
    
    return self;
}

- (id)initWithAngle:(double)ang axisX:(double)x axisY:(double)y axisZ:(double)z
{
    self = [super init];
    Affine3d rotMat(AngleAxisd(ang,Vector3d(x,y,z)));
    _mat = rotMat.matrix();
    
    return self;
}

- (id)multiplyWith:(MaplyMatrix *)other
{
    MaplyMatrix *newMat = [[MaplyMatrix alloc] init];
    Matrix4d mat1 = _mat;
    Matrix4d mat2 = other.mat;
    Matrix4d resMat = mat1 * mat2;
    newMat.mat = resMat;
    
    return newMat;
}

@end
