#ifndef __VEC_H
#define __VEC_H

#include <math.h>
#include "fxmath.h"

// well known inverse sqrt from quake 3 source :)
inline float Q_rsqrt( float number ){
	long i;
	float x2, y;
	const float threehalfs = 1.5F;

	x2 = number * 0.5F;
	y  = number;
	i  = * ( long * ) &y;                       // evil floating point bit level hacking
	i  = 0x5f375a86 - ( i >> 1 );               // what the fuck? (more accurate constant also)
	y  = * ( float * ) &i;
	y  = y * ( threehalfs - ( x2 * y * y ) );   // 1st iteration
//	y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed

	return y;
}

// ----------------------------------

struct vec2i;
struct vec2f;
struct vec2x;
struct vec3f;
struct vec3x;
struct vec4f;
struct vec4x;

// ----------------------------------
// integer
struct vec2i {
    signed long x;
    signed long y;
    
    // add/subtract another vec2i
    inline vec2i& operator+=(const vec2i & rhs) {x += rhs.x; y += rhs.y; return *this;}
    inline vec2i& operator-=(const vec2i & rhs) {x -= rhs.x; y -= rhs.y; return *this;}
    
    // multiply/divide by scalar
    inline vec2i& operator*=(const signed rhs) {x *= rhs;   y *= rhs;   return *this;}
    inline vec2i& operator/=(const signed rhs) {x /= rhs;   y /= rhs;   return *this;}
    inline vec2i& operator*=(const float  rhs) {x *= rhs;   y *= rhs;   return *this;}
    inline vec2i& operator/=(const float  rhs) {x /= rhs;   y /= rhs;   return *this;}
    
    
    // signs and other stuff
    inline vec2i& operator-() {x = -x; y = -y; return *this;}
    
    // add/subtract two vec2f
    inline const vec2i operator+(const vec2i& rhs) {return vec2i(*this) += rhs;}
    inline const vec2i operator-(const vec2i& rhs) {return vec2i(*this) -= rhs;}
    
    // multiply/divide by scalar and throw to another vec2i
    inline const vec2i operator*(const signed rhs) {return vec2i(*this) *= rhs;}
    inline const vec2i operator/(const signed rhs) {return vec2i(*this) /= rhs;}
    
    inline operator vec2f();
    inline operator vec2x();
};

// dot product
inline signed long dot (const vec2i& lhs, const vec2i& rhs) {return (lhs.x*rhs.x + lhs.y*rhs.y);}
// length
inline signed long abs (const vec2i& lhs) {return sqrt(lhs.x*lhs.x + lhs.y*lhs.y);}
// norm
inline vec2i norm(const vec2i& lhs) {vec2i r = lhs; return (r / abs(r));}

// ----------------------------------

struct vec2f {
    float x;
    float y;
    
    // add/subtract another vec2f
    inline vec2f& operator+=(const vec2f & rhs) {x += rhs.x; y += rhs.y; return *this;}
    inline vec2f& operator-=(const vec2f & rhs) {x -= rhs.x; y -= rhs.y; return *this;}
    
    // multiply/divide by scalar
    inline vec2f& operator*=(const signed rhs) {x *= rhs;   y *= rhs;   return *this;}
    inline vec2f& operator/=(const signed rhs) {x /= rhs;   y /= rhs;   return *this;}
    inline vec2f& operator*=(const float  rhs) {x *= rhs;   y *= rhs;   return *this;}
    inline vec2f& operator/=(const float  rhs) {x /= rhs;   y /= rhs;   return *this;}
    
    // signs and other stuff
    inline vec2f& operator-() {x = -x; y = -y; return *this;}
    
    // add/subtract two vec2f
    inline const vec2f operator+(const vec2f& rhs) {return vec2f(*this) += rhs;}
    inline const vec2f operator-(const vec2f& rhs) {return vec2f(*this) -= rhs;}
    
    // multiply/divide by scalar and throw to another vec2f
    inline const vec2f operator*(const signed rhs) {return vec2f(*this) *= rhs;}
    inline const vec2f operator/(const signed rhs) {return vec2f(*this) /= rhs;}
    inline const vec2f operator*(const float  rhs) {return vec2f(*this) *= rhs;}
    inline const vec2f operator/(const float  rhs) {return vec2f(*this) /= rhs;}
    
    inline operator vec2i();
    inline operator vec2x();
};

// dot product
inline float dot (const vec2f& lhs, const vec2f& rhs) {return (lhs.x*rhs.x + lhs.y*rhs.y);}
// length
inline float abs (const vec2f& lhs) {return sqrt(lhs.x*lhs.x + lhs.y*lhs.y);}
inline float abs2(const vec2f& lhs) {return lhs.x*lhs.x + lhs.y*lhs.y;}
// norm
inline vec2f norm(const vec2f& lhs) {vec2f r = lhs; return r*Q_rsqrt(r.x*r.x + r.y*r.y);}

// ----------------------------------
// fixed point 16:16
struct vec2x {
    signed long x;
    signed long y;
    
    // add/subtract another vec2f
    inline vec2x& operator+=(const vec2x & rhs) {x += rhs.x; y += rhs.y; return *this;}
    inline vec2x& operator-=(const vec2x & rhs) {x -= rhs.x; y -= rhs.y; return *this;}
    
    // multiply/divide by scalar
    inline vec2x& operator*=(const signed rhs) {x *= rhs; y *= rhs;   return *this;}
    inline vec2x& operator/=(const signed rhs) {x /= rhs; y /= rhs;   return *this;}
    
    inline vec2x& operator*=(const float  rhs) {
        x = imul16(x, (signed long)(rhs*65536.0f));
        y = imul16(y, (signed long)(rhs*65536.0f));
        return *this;
    }
    inline vec2x& operator/=(const float  rhs) {
        x = idiv16(x, (signed long)(rhs*65536.0f));
        y = idiv16(y, (signed long)(rhs*65536.0f));
        return *this;
    }
    
    // signs and other stuff
    inline vec2x& operator-() {x = -x; y = -y; return *this;}
    
    // add/subtract two vec2f
    inline const vec2x operator+(const vec2x& rhs) {return vec2x(*this) += rhs;}
    inline const vec2x operator-(const vec2x& rhs) {return vec2x(*this) -= rhs;}
    
    // multiply/divide by scalar and throw to another vec2f
    inline const vec2x operator*(const signed rhs) {return vec2x(*this) *= rhs;}
    inline const vec2x operator/(const signed rhs) {return vec2x(*this) /= rhs;}
    inline const vec2x operator*(const float  rhs) {return vec2x(*this) *= rhs;}
    inline const vec2x operator/(const float  rhs) {return vec2x(*this) /= rhs;}
    
    inline operator vec2f();
    inline operator vec2i();
};

// dot product
inline float dot (const vec2x& lhs, const vec2x& rhs) {return (imul16(lhs.x, rhs.x) + imul16(lhs.y, rhs.y));}
// length
inline float abs (const vec2x& lhs) {return sqrt(imul16(lhs.x, lhs.x) + imul16(lhs.y, lhs.y));}
// norm
inline vec2x norm(const vec2x& lhs) {vec2x r = lhs; return (r / abs(r));}

// type conversions

inline vec2f::operator vec2x() {vec2x r; fist(&r.x, x*65536.0f); fist(&r.y, y*65536.0f); return r;}
inline vec2f::operator vec2i() {vec2i r = {x, y}; return r;}
inline vec2x::operator vec2f() {vec2f r = {((float)x / 65536.0f), ((float)y / 65536.0f)}; return r;}
inline vec2x::operator vec2i() {vec2i r = {(x >> 16), (x >> 16)}; return r;}

// ----------------------------------

struct vec3f {
    float x;
    float y;
    float z;
    
    // add/subtract another vec2f
    inline vec3f& operator+=(const vec3f & rhs) {x += rhs.x; y += rhs.y; z += rhs.z; return *this;}
    inline vec3f& operator-=(const vec3f & rhs) {x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this;}
    
    // multiply/divide by scalar
    inline vec3f& operator*=(const signed rhs) {x *= rhs; y *= rhs; z *= rhs; return *this;}
    inline vec3f& operator/=(const signed rhs) {x /= rhs; y /= rhs; z /= rhs; return *this;}
    inline vec3f& operator*=(const float  rhs) {x *= rhs; y *= rhs; z *= rhs; return *this;}
    inline vec3f& operator/=(const float  rhs) {x /= rhs; y /= rhs; z /= rhs; return *this;}
    
    inline vec3f& operator*=(signed &rhs) {x *= rhs; y *= rhs; z *= rhs; return *this;}
    inline vec3f& operator/=(signed &rhs) {x /= rhs; y /= rhs; z /= rhs; return *this;}
    inline vec3f& operator*=(float  &rhs) {x *= rhs; y *= rhs; z *= rhs; return *this;}
    inline vec3f& operator/=(float  &rhs) {x /= rhs; y /= rhs; z /= rhs; return *this;}
    
    // signs and other stuff
    inline vec3f& operator-() {x = -x; y = -y; z = -z; return *this;}
    
    // add/subtract two vec2f
    inline const vec3f operator+(const vec3f& rhs) {return vec3f(*this) += rhs;}
    inline const vec3f operator-(const vec3f& rhs) {return vec3f(*this) -= rhs;}
    
    // multiply/divide by scalar and throw to another vec2f
    inline const vec3f operator*(const signed rhs) {return vec3f(*this) *= rhs;}
    inline const vec3f operator/(const signed rhs) {return vec3f(*this) /= rhs;}
    inline const vec3f operator*(const float  rhs) {return vec3f(*this) *= rhs;}
    inline const vec3f operator/(const float  rhs) {return vec3f(*this) /= rhs;}
    
    inline operator vec3x();
};

// dot product
inline float dot  (const vec3f& lhs, const vec3f& rhs) {return (lhs.x*rhs.x + lhs.y*rhs.y + lhs.z*rhs.z);}
// cross product
inline vec3f cross(const vec3f& lhs, const vec3f& rhs) {
    vec3f r = {lhs.y * rhs.z - lhs.z * rhs.y,
               lhs.z * rhs.x - lhs.x * rhs.z,
               lhs.x * rhs.y - lhs.y * rhs.x};
    return r;
}
// length
inline float abs  (const vec3f& lhs) {return sqrt(lhs.x*lhs.x + lhs.y*lhs.y + lhs.z*lhs.z);}
// norm
inline vec3f norm (const vec3f& lhs) {vec3f r = lhs; return r*Q_rsqrt(r.x*r.x + r.y*r.y + r.z*r.z);}

// ----------------------------------
// fixed point 16:16
struct vec3x {
    signed long x;
    signed long y;
    signed long z;
    
    // add/subtract another vec2f
    inline vec3x& operator+=(const vec3x & rhs) {x += rhs.x; y += rhs.y; z += rhs.z; return *this;}
    inline vec3x& operator-=(const vec3x & rhs) {x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this;}
    
    // multiply/divide by scalar
    inline vec3x& operator*=(const float  rhs) {
        x = imul16(x, (signed long)(rhs*65536.0f));
        y = imul16(y, (signed long)(rhs*65536.0f));
        z = imul16(z, (signed long)(rhs*65536.0f));
        return *this;
    }
    inline vec3x& operator/=(const float  rhs) {
        x = idiv16(x, (signed long)(rhs*65536.0f));
        y = idiv16(y, (signed long)(rhs*65536.0f));
        z = idiv16(z, (signed long)(rhs*65536.0f));
        return *this;
    }
    
    // signs and other stuff
    inline vec3x& operator-() {x = -x; y = -y; z = -z; return *this;}
    
    // add/subtract two vec2f
    inline const vec3x operator+(const vec3x& rhs) {return vec3x(*this) += rhs;}
    inline const vec3x operator-(const vec3x& rhs) {return vec3x(*this) -= rhs;}
    
    // multiply/divide by scalar and throw to another vec2f
    inline const vec3x operator*(const signed rhs) {return vec3x(*this) *= rhs;}
    inline const vec3x operator/(const signed rhs) {return vec3x(*this) /= rhs;}
    inline const vec3x operator*(const float  rhs) {return vec3x(*this) *= rhs;}
    inline const vec3x operator/(const float  rhs) {return vec3x(*this) /= rhs;}
    
    inline operator vec3f();
};

// dot product
inline float dot  (const vec3x& lhs, const vec3x& rhs) {return (imul16(lhs.x, rhs.x) + imul16(lhs.y, rhs.y) + imul16(lhs.z, rhs.z));}
// cross product
inline vec3x cross(const vec3x& lhs, const vec3x& rhs) {
    vec3x r = {imul16(lhs.y, rhs.z) - imul16(lhs.z, rhs.y),
               imul16(lhs.z, rhs.x) - imul16(lhs.x, rhs.z),
               imul16(lhs.x, rhs.y) - imul16(lhs.y, rhs.x)};
    return r;
}
// length
inline float abs  (const vec3x& lhs) {return sqrt(imul16(lhs.x, lhs.x) + imul16(lhs.y, lhs.y) + imul16(lhs.z, lhs.z));}
// norm
inline vec3x norm(const vec3x& lhs) {vec3x r = lhs; return (r / abs(r));}

// type conversions
inline vec3f::operator vec3x() {vec3x r; fist(&r.x, x*65536.0f); fist(&r.y, y*65536.0f); fist(&r.z, z*65536.0f); return r;}
inline vec3x::operator vec3f() {vec3f r = {((float)x / 65536.0f), ((float)y / 65536.0f), ((float)z / 65536.0f)}; return r;}

// ----------------------------------

// in fact these are homogenous vectors, i.e. most R4 vector rules are heavily simplified
struct vec4f {
    union {float x; float b;};
    union {float y; float g;};
    union {float z; float r;};
    union {float w; float a;};
    
    // add/subtract another vec2f
    inline vec4f& operator+=(const vec4f & rhs) {x += rhs.x; y += rhs.y; z += rhs.z; return *this;}
    inline vec4f& operator-=(const vec4f & rhs) {x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this;}
    
    // multiply/divide by scalar
    inline vec4f& operator*=(const signed rhs) {x *= rhs; y *= rhs; z *= rhs; return *this;}
    inline vec4f& operator/=(const signed rhs) {x /= rhs; y /= rhs; z /= rhs; return *this;}
    inline vec4f& operator*=(const float  rhs) {x *= rhs; y *= rhs; z *= rhs; return *this;}
    inline vec4f& operator/=(const float  rhs) {x /= rhs; y /= rhs; z /= rhs; return *this;}
    
    // signs and other stuff
    inline vec4f& operator-() {x = -x; y = -y; z = -z; return *this;}
    
    // add/subtract two vec2f
    inline const vec4f operator+(const vec4f& rhs) {return vec4f(*this) += rhs;}
    inline const vec4f operator-(const vec4f& rhs) {return vec4f(*this) -= rhs;}
    
    // multiply/divide by scalar and throw to another vec2f
    inline const vec4f operator*(const signed rhs) {return vec4f(*this) *= rhs;}
    inline const vec4f operator/(const signed rhs) {return vec4f(*this) /= rhs;}
    inline const vec4f operator*(const float  rhs) {return vec4f(*this) *= rhs;}
    inline const vec4f operator/(const float  rhs) {return vec4f(*this) /= rhs;}
  
    inline operator vec4x();  
};

// dot product
inline float dot  (const vec4f& lhs, const vec4f& rhs) {return (lhs.x*rhs.x + lhs.y*rhs.y + lhs.z*rhs.z);}
// cross product
inline vec4f cross(const vec4f& lhs, const vec4f& rhs) {
    vec4f r = {lhs.y * rhs.z - lhs.z * rhs.y,
               lhs.z * rhs.x - lhs.x * rhs.z,
               lhs.x * rhs.y - lhs.y * rhs.x,
               lhs.w};
    return r;
}
// length
inline float abs  (const vec4f& lhs) {return sqrt(lhs.x*lhs.x + lhs.y*lhs.y + lhs.z*lhs.z);}
// norm
inline vec4f norm (const vec4f& lhs) {vec4f r = lhs; return r*Q_rsqrt(r.x*r.x + r.y*r.y + r.z*r.z);}

// ----------------------------------
// fixed point 16:16
struct vec4x {
    signed long x;
    signed long y;
    signed long z;
    signed long w;
    
    // add/subtract another vec2f
    inline vec4x& operator+=(const vec4x & rhs) {x += rhs.x; y += rhs.y; z += rhs.z; return *this;}
    inline vec4x& operator-=(const vec4x & rhs) {x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this;}
    
    // multiply/divide by scalar
    inline vec4x& operator*=(const float  rhs) {
        x = imul16(x, (signed long)(rhs*65536.0f));
        y = imul16(y, (signed long)(rhs*65536.0f));
        z = imul16(z, (signed long)(rhs*65536.0f));
        return *this;
    }
    inline vec4x& operator/=(const float  rhs) {
        x = idiv16(x, (signed long)(rhs*65536.0f));
        y = idiv16(y, (signed long)(rhs*65536.0f));
        z = idiv16(z, (signed long)(rhs*65536.0f));
        return *this;
    }
    
    // signs and other stuff
    inline vec4x& operator-() {x = -x; y = -y; z = -z; return *this;}
    
    // add/subtract two vec2f
    inline const vec4x operator+(const vec4x& rhs) {return vec4x(*this) += rhs;}
    inline const vec4x operator-(const vec4x& rhs) {return vec4x(*this) -= rhs;}
    
    // multiply/divide by scalar and throw to another vec2f
    inline const vec4x operator*(const signed rhs) {return vec4x(*this) *= rhs;}
    inline const vec4x operator/(const signed rhs) {return vec4x(*this) /= rhs;}
    inline const vec4x operator*(const float  rhs) {return vec4x(*this) *= rhs;}
    inline const vec4x operator/(const float  rhs) {return vec4x(*this) /= rhs;}
    
    inline operator vec4f();
};

// dot product
inline float dot  (const vec4x& lhs, const vec4x& rhs) {return (imul16(lhs.x, rhs.x) + imul16(lhs.y, rhs.y) + imul16(lhs.z, rhs.z));}
// cross product
inline vec4x cross(const vec4x& lhs, const vec4x& rhs) {
    vec4x r = {imul16(lhs.y, rhs.z) - imul16(lhs.z, rhs.y),
               imul16(lhs.z, rhs.x) - imul16(lhs.x, rhs.z),
               imul16(lhs.x, rhs.y) - imul16(lhs.y, rhs.x),
               lhs.w};
    return r;
}
// length
inline float abs  (const vec4x& lhs) {return sqrt(imul16(lhs.x, lhs.x) + imul16(lhs.y, lhs.y) + imul16(lhs.z, lhs.z));}
// norm
inline vec4x norm(const vec4x& lhs) {vec4x r = lhs; return (r / abs(r));}

// ----------------------------------
// integer
struct vec4i {
    signed long x;
    signed long y;
    signed long z;
    signed long w;
    
    // add/subtract another vec2f
    inline vec4i& operator+=(const vec4i & rhs) {x += rhs.x; y += rhs.y; z += rhs.z; w += rhs.w; return *this;}
    inline vec4i& operator-=(const vec4i & rhs) {x -= rhs.x; y -= rhs.y; z -= rhs.z; w += rhs.w; return *this;}
    
    // multiply/divide by scalar
    inline vec4i& operator*=(const float  rhs) {
        x *= rhs;
        y *= rhs;
        z *= rhs;
        w *= rhs;
        return *this;
    }
    inline vec4i& operator/=(const float  rhs) {
        x /= rhs;
        y /= rhs;
        z /= rhs;
        w /= rhs;
        return *this;
    }
    
    // signs and other stuff
    inline vec4i& operator-() {x = -x; y = -y; z = -z; w = -w; return *this;}
    
    // add/subtract two vec2f
    inline const vec4i operator+(const vec4i& rhs) {return vec4i(*this) += rhs;}
    inline const vec4i operator-(const vec4i& rhs) {return vec4i(*this) -= rhs;}
    
    // multiply/divide by scalar and throw to another vec2f
    inline const vec4i operator*(const signed rhs) {return vec4i(*this) *= rhs;}
    inline const vec4i operator/(const signed rhs) {return vec4i(*this) /= rhs;}
    inline const vec4i operator*(const float  rhs) {return vec4i(*this) *= rhs;}
    inline const vec4i operator/(const float  rhs) {return vec4i(*this) /= rhs;}
    
    inline operator vec4f();
};

// dot product
inline float dot  (const vec4i& lhs, const vec4i& rhs) {return (imul16(lhs.x, rhs.x) + imul16(lhs.y, rhs.y) + imul16(lhs.z, rhs.z));}
// cross product
inline vec4x cross(const vec4i& lhs, const vec4i& rhs) {
    vec4x r = {imul16(lhs.y, rhs.z) - imul16(lhs.z, rhs.y),
               imul16(lhs.z, rhs.x) - imul16(lhs.x, rhs.z),
               imul16(lhs.x, rhs.y) - imul16(lhs.y, rhs.x),
               lhs.w};
    return r;
}
// length
inline float abs  (const vec4i& lhs) {return sqrt(imul16(lhs.x, lhs.x) + imul16(lhs.y, lhs.y) + imul16(lhs.z, lhs.z));}
// norm
inline vec4i norm(const vec4i& lhs) {vec4i r = lhs; return (r / abs(r));}



// type conversions
inline vec4f::operator vec4x() {vec4x r; fist(&r.x, x*65536.0f); fist(&r.y, y*65536.0f); fist(&r.z, z*65536.0f); fist(&r.w, w*65536.0f); return r;}
inline vec4x::operator vec4f() {vec4f r = {((float)x / 65536.0f), ((float)y / 65536.0f), ((float)z / 65536.0f), ((float)w / 65536.0f)}; return r;}



#endif

