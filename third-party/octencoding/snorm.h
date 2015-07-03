#ifndef snorm_h
#define snorm_h
#include <cmath>
#include <stdint.h>

inline float clamp(float val, float low, float hi) {
    if (val <= low) {
        return low;
    } else if (val >= hi) {
        return hi;
    } else {
        return val;
    }
}   

inline float snorm_round(float f) {
    return floor(f + 0.5f);
}

/** The underlying representation is always int in this implementation.
    With template tricks or a type argument one can implement 
    a variant that chooses between int8, int16, int32, and int64 for
    the internal representation...or use the somewhat more obscure C
    "int m_bits:bitcount;" syntax, which does not actually save
    space below 8 bits because of alignment requirements.
*/
template<int bitcount>
class Snorm {
private:
    typedef int T;
    T   m_bits;

    /** Private to prevent illogical conversions without explicitly
     stating that the input should be treated as bits; see fromBits. */
    explicit Snorm(T b) : m_bits(b) {}

public:

    explicit Snorm() : m_bits(0) {}

    /** Equivalent to: \code snorm8 u = reinterpret_cast<const snorm8&>(255);\endcode */
    static Snorm fromBits(T b) {
        return Snorm(b);
    }

    /** Maps f to the underlying bits of round(f * ((2^(bitcount - 1) - 1)).*/
    explicit Snorm(float f) {
        m_bits = (T)snorm_round(clamp(f, -1.0f, 1.0f) * ((uint64_t(1) << (bitcount - 1)) - 1));
    }

    /** this function will be used to find the top left corner of the corresponding square to allow for discrete searching*/
    static Snorm flooredSnorms(float f) {
        return fromBits((T)(floor(clamp(f, -1.0f, 1.0f) * ((uint64_t(1) << (bitcount - 1)) - 1))));
    }

    /** Returns a number on [0.0f, 1.0f] */
    operator float() const {
        return float(clamp(int(m_bits) * (1.0f / float((uint64_t(1) << (bitcount - 1)) - 1)), -1.0f, 1.0f));
    }

    T bits() const {
        return m_bits;
    }
};
#endif
