/*  WhirlyEigen.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/18/11.
 *  Copyright 2011-2022 mousebird consulting
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
 */

// Avoid alignment errors on Eigen stuff passed by value.
// Though we shouldn't do that, at least it won't crash.
// See:
//  http://eigen.tuxfamily.org/dox-devel/group__TopicUnalignedArrayAssert.html
//  http://eigen.tuxfamily.org/dox-devel/group__TopicPassingByValue.html
//  http://eigen.tuxfamily.org/dox-devel/TopicPreprocessorDirectives.html#TopicPreprocessorDirectivesPerformance
#define EIGEN_UNALIGNED_VECTORIZE 0

#define EIGEN_NO_IO         // Exclude I/O stuff for Eigen types, we don't need it
#define EIGEN_MPL2_ONLY     // Exclude LGPL features

#import <Eigen/Eigen>

namespace WhirlyKit
{

using Point4f = Eigen::Vector4f;
using Point4d = Eigen::Vector4d;
using Point3f = Eigen::Vector3f;
using Point3d = Eigen::Vector3d;
using Point2d = Eigen::Vector2d;
using Point2f = Eigen::Vector2f;

using Point2fVector = std::vector<Point2f,Eigen::aligned_allocator<Point2f>>;
using Point2dVector = std::vector<Point2d,Eigen::aligned_allocator<Point2d>>;
using Point3fVector = std::vector<Point3f,Eigen::aligned_allocator<Point3f>>;
using Point3dVector = std::vector<Point3d,Eigen::aligned_allocator<Point3d>>;
using Vector4fVector = std::vector<Eigen::Vector4f,Eigen::aligned_allocator<Eigen::Vector4f>>;
using Vector4dVector = std::vector<Eigen::Vector4d,Eigen::aligned_allocator<Eigen::Vector4d>>;
using Matrix4fVector = std::vector<Eigen::Matrix4f,Eigen::aligned_allocator<Eigen::Matrix4f>>;
using Matrix4dVector = std::vector<Eigen::Matrix4d,Eigen::aligned_allocator<Eigen::Matrix4d>>;

}
