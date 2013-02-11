/*
 *  MaplyShader.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 2/7/13.
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

#import <UIKit/UIKit.h>
#import <string>
#import <WhirlyGlobe.h>
#import "MaplyShader_private.h"
#import "MaplyBaseViewController_private.h"

using namespace WhirlyKit;

@implementation MaplyShader
{
    NSString *buildError;
    EAGLContext *context;
}

- (id)initWithName:(NSString *)name vertexFile:(NSString *)vertexFileName fragmentFile:(NSString *)fragFileName viewC:(MaplyBaseViewController *)baseViewC
{
    NSError *error = nil;
    NSString *vertexShader = [NSString stringWithContentsOfFile:vertexFileName encoding:NSASCIIStringEncoding error:&error];
    if (!vertexShader)
    {
        vertexFileName = [[NSBundle mainBundle] pathForResource:vertexFileName ofType:@""];
        vertexShader = [NSString stringWithContentsOfFile:vertexFileName encoding:NSASCIIStringEncoding error:&error];
        if (!vertexShader)
        {
            buildError = @"Could not find vertex shader file.";
            return nil;
        }
    }

    NSString *fragShader = [NSString stringWithContentsOfFile:fragFileName encoding:NSASCIIStringEncoding error:&error];
    if (!fragShader)
    {
        fragFileName = [[NSBundle mainBundle] pathForResource:fragFileName ofType:@""];
        fragShader = [NSString stringWithContentsOfFile:fragFileName encoding:NSASCIIStringEncoding error:&error];
        if (!fragShader)
        {
            buildError = @"Could not find fragment shader file.";
            return nil;
        }
    }
    
    return [self initWithName:name vertex:vertexShader fragment:fragShader viewC:baseViewC];
}

- (id)initWithName:(NSString *)name vertex:(NSString *)vertexProg fragment:(NSString *)fragProg viewC:(MaplyBaseViewController *)baseViewC
{
    if (!vertexProg || !fragProg)
    {
        buildError = @"Empty vertex or fragment shader program.";
        return nil;
    }
    
    std::string vertexStr = [vertexProg cStringUsingEncoding:NSASCIIStringEncoding];
    std::string fragStr = [fragProg cStringUsingEncoding:NSASCIIStringEncoding];
    std::string nameStr = [name cStringUsingEncoding:NSASCIIStringEncoding];

    EAGLContext *oldContext = [EAGLContext currentContext];
    [baseViewC useGLContext];
    _program = new OpenGLES2Program(nameStr,vertexStr,fragStr);
    if (oldContext)
        [EAGLContext setCurrentContext:oldContext];
    
    if (!_program->isValid())
    {
        buildError = @"Could not compile program.";
        delete _program;
        _program = NULL;
        return nil;
    }
    
    [baseViewC addShader:self];
    
    return self;
}

// We're assuming the view controller has set the proper context
- (void)shutdown
{
    if (_program)
    {
//        _program->cleanUp();
//        delete _program;
        _program = NULL;
    }
}

- (void)dealloc
{
}

- (bool)valid
{
    return _program != NULL;
}

- (NSString *)getError
{
    return buildError;
}


@end