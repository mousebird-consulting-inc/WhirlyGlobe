/*  RGBAColor.h
 *  WhirlyGlobeLib
 *
 *  Created by Tim Sylvester on 1/13/23.
 *  Copyright 2023-2023 mousebird consulting
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

#import <cmath>
#import <cstdint>
#import <memory>

namespace WhirlyKit
{

/// Color. RGBA, 8 bits per channel.
class RGBAColor
{
public:
    constexpr RGBAColor() : RGBAColor(0,0,0,0) { }

    constexpr RGBAColor(int r,int g,int b,int a) :
            r((uint8_t)r), g((uint8_t)g), b((uint8_t)b), a((uint8_t)a) { }
    constexpr RGBAColor(int r,int g,int b) :
            r((uint8_t)r), g((uint8_t)g), b((uint8_t)b), a(255) { }

#if defined(EIGEN_CORE_H)
    template <typename T>
    RGBAColor(const Eigen::Matrix<T,4,1> &v) :
        RGBAColor((uint8_t)(v.x()*255),(uint8_t)(v.y()*255),
                  (uint8_t)(v.z()*255),(uint8_t)(v.w()*255)) { }

    template <typename T>
    static RGBAColor FromVec(const Eigen::Matrix<T,4,1> &v) { return RGBAColor(v); }

    Eigen::Vector4f asRGBAVecF() const { return {r/255.f,g/255.f,b/255.f,a/255.f}; }
    Eigen::Vector4d asRGBAVecD() const { return {r/255.0,g/255.0,b/255.0,a/255.0}; }
#endif

    /**
    * Replace the alpha channel with a new value, leaving others unaffected
    * i.e., apply an absolute alpha value to a non-pre-multiplied color
    */
    constexpr RGBAColor withAlpha(int newA) const { return RGBAColor(r,g,b,(uint8_t)newA); }
    constexpr RGBAColor withAlpha(float newA) const { return RGBAColor(r,g,b,(uint8_t)(newA * 255)); }
    constexpr RGBAColor withAlpha(double newA) const { return RGBAColor(r,g,b,(uint8_t)(newA * 255)); }

    /**
    * Assign a new alpha channel, multiplying that value through the other channels
    * i.e., Change a color to pre-multiplied alpha
    */
    constexpr RGBAColor withAlphaMultiply(float newA) const {
        return RGBAColor((uint8_t)(r*newA),(uint8_t)(g*newA),
                         (uint8_t)(b*newA),(uint8_t)(newA * 255));
    }
    constexpr RGBAColor withAlphaMultiply(double newA) const {
        return RGBAColor((uint8_t)(r*newA),(uint8_t)(g*newA),
                         (uint8_t)(b*newA),(uint8_t)(newA * 255));
    }

    /**
    * Multiply the alpha channel by the given value, leaving others unaffected
    * i.e., apply an additional alpha value to a non-pre-multiplied color
    */
    constexpr RGBAColor withAlphaFactor(float newA) const { return RGBAColor(r,g,b,(uint8_t)(a * newA)); }

    /**
    * Multiply the alpha channel by the specified value, then multiply the result through the other channels.
    * i.e., apply an additional alpha to an already pre-multiplied color
    */
    constexpr RGBAColor applyMultipliedAlpha(float newA) const {
        return RGBAColor((uint8_t)(r*newA),(uint8_t)(g*newA),
                         (uint8_t)(b*newA),(uint8_t)(a * newA));
    }


    // Create an RGBColor from unit floats
    static constexpr RGBAColor FromUnitFloats(const float *f) {
        return FromUnitFloats4(f);
    }
    static constexpr RGBAColor FromUnitFloats3(const float *f) {
        return FromUnitFloats(f[0],f[1],f[2]);
    }
    static constexpr RGBAColor FromUnitFloats4(const float *f) {
        return FromUnitFloats(f[0],f[1],f[2],f[3]);
    }
    static constexpr RGBAColor FromUnitFloats(float r, float g, float b) {
        return FromUnitFloats(r,g,b,1.0f);
    }
    static constexpr RGBAColor FromUnitFloats(float r, float g, float b, float a) {
        return RGBAColor((uint8_t)(r * 255.0f),(uint8_t)(g * 255.0f),
                         (uint8_t)(b * 255.0f),(uint8_t)(a * 255.0f));
    }
    static constexpr RGBAColor FromUnitFloats(double r, double g, double b) {
        return FromUnitFloats(r,g,b,1.0);
    }
    static constexpr RGBAColor FromUnitFloats(double r, double g, double b, double a) {
        return RGBAColor((uint8_t)(r * 255.0),(uint8_t)(g * 255.0),
                         (uint8_t)(b * 255.0),(uint8_t)(a * 255.0));
    }

    // Create an RGBAColor from an int
    static constexpr RGBAColor FromInt(uint32_t color) { return FromARGBInt(color); }
    static constexpr RGBAColor FromARGBInt(uint32_t color) {
        return RGBAColor((uint8_t)((color >> 16) & 0xff),
                         (uint8_t)((color >> 8) & 0xff),
                         (uint8_t)(color & 0xff),
                         (uint8_t)(color >> 24));
    }
    
    // Create an RGBAColor from HSV
    static /*constexpr*/ RGBAColor FromHSV(int hue,double s,double v) {
        const double c = s * v;
        const double x = c * (1 - std::abs(std::fmod(hue / 60.0, 2.0) - 1));
        const double m = v - c;
        double rs=c,gs=0,bs=x;
        if (hue >= 0 && hue < 60) {
            rs= c;  gs = x;  bs = 0;
        } else if(hue >= 60 && hue < 120) {
            rs = x;  gs = c;  bs = 0;
        } else if(hue >= 120 && hue < 180) {
            rs = 0;  gs = c;  bs = x;
        } else if(hue >= 180 && hue < 240) {
            rs = 0;  gs = x;  bs = c;
        } else if(hue >= 240 && hue < 300) {
            rs = x;  gs = 0;  bs = c;
        }
        return RGBAColor((rs + m) * 255.0, (gs + m) * 255.0, (bs + m) * 255.0);
    }

    // Create an RGBAColor from HSL
    static /*constexpr*/ RGBAColor FromHSL(int hue,double s,double l) {
        const double c = (1 - std::abs(2*l - 1)) * s;
        const double x = c * (1 - std::abs(fmod(hue / 60.0, 2.0) - 1));
        double m = l - c/2.0;
        double rs=c,gs=0,bs=x;
        if (hue >= 0 && hue < 60) {
            rs= c;  gs = x;  bs = 0;
        } else if(hue >= 60 && hue < 120) {
            rs = x;  gs = c;  bs = 0;
        } else if(hue >= 120 && hue < 180) {
            rs = 0;  gs = c;  bs = x;
        } else if(hue >= 180 && hue < 240) {
            rs = 0;  gs = x;  bs = c;
        } else if(hue >= 240 && hue < 300) {
            rs = x;  gs = 0;  bs = c;
        }
        return RGBAColor((rs + m) * 255.0, (gs + m) * 255.0, (bs + m) * 255.0);
    }

    // Standard colors to create & return

    static constexpr RGBAColor black() { return RGBAColor(0,0,0,255); }
    static constexpr RGBAColor white() { return RGBAColor(255,255,255,255); }
    static constexpr RGBAColor red() { return RGBAColor(255,0,0,255); }
    static constexpr RGBAColor green() { return RGBAColor(0,255,0,255); }
    static constexpr RGBAColor blue() { return RGBAColor(0,0,255,255); }
    static constexpr RGBAColor clear() { return RGBAColor(0,0,0,0); }

    /// Returns an an array of 4 floats
    void asUnitFloats(float *ret) const { ret[0] = (float)r / 255.0;  ret[1] = (float)g / 255.0; ret[2] = (float)b / 255.0; ret[3] = (float)a / 255.0; }
    
    /// Convert to a 32 bit integer (ala Android)
    constexpr int asInt() const { return asARGBInt(); }
    constexpr int asARGBInt() const { return a << 24 | r << 16 | g << 8 | b; }
    constexpr int asRGBAInt() const { return r << 24 | g << 16 | b << 8 | a; }

    /// Returns as a 4 component array of unsigned chars
    void asUChar4(unsigned char *ret) const { ret[0] = r; ret[1] = g; ret[2] = b; ret[3] = a; }

    constexpr bool operator == (const RGBAColor &that) const { return (r == that.r && g == that.g && b == that.b && a == that.a); }
    constexpr bool operator != (const RGBAColor &that) const { return (r != that.r || g != that.g || b != that.b || a != that.a); }

    constexpr RGBAColor operator * (float alpha) const { return RGBAColor(r*alpha,g*alpha,b*alpha,a*alpha); }

    unsigned char r,g,b,a;
};
typedef std::shared_ptr<RGBAColor> RGBAColorRef;

}   // namespace WhirlyKit
