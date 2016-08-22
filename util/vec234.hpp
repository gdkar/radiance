/*
    Copyright (c) 2015, Gunnar Sletta <gunnar@sletta.org>
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
    ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
    Matrix inversion code is taken from MESA
 */

/*!
    Q: Why duplicate this? There are plenty of libraries which provide basic math?

    A: True, but I want this library and this file in particular to be a
    selfcontained, no-hassle-to-deploy suite of strictly needed math things.
    It isn't complete or perhaps not fully optimal either (due to no sse and such),
    but it is all inline, comes at no deployment cost and the file can be included
    in any project by simply dumping a single header file in there.
 */

#pragma once

#include <cmath>
#include <math.h>
#include <numeric>
#include <algorithm>
#include <utility>
#include <iterator>
#include <type_traits>
#include <functional>
#include <ostream>
#include <cassert>

    //RENGINE_BEGIN_NAMESPACE
template<class T, size_t N>
struct cexpr_array {
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = T&;
    using const_reference = const T&;
    using iterator = T*;
    using const_iterator = const T*;

    T m_d[N] = { T{}};
    constexpr cexpr_array() = default;
    constexpr cexpr_array(const cexpr_array&) = default;
    constexpr cexpr_array(cexpr_array&&)noexcept = default;
    cexpr_array&operator=(const cexpr_array&) = default;
    cexpr_array&operator=(cexpr_array&&)noexcept = default;

    constexpr iterator begin() { return m_d;}
    constexpr iterator end()   { return begin() + N;}
    constexpr size_type size() const { return N;}
    constexpr const_iterator begin() const { return m_d;}
    constexpr const_iterator end() const  { return begin() + N;}
    constexpr const_iterator cbegin() const { return m_d;}
    constexpr const_iterator cend() const  { return begin() + N;}
    constexpr reference operator[](size_type x) { return m_d[x];}
    constexpr const_reference operator[](size_type x) const { return m_d[x];}
    constexpr const_reference at(size_type x) const { return m_d[x];}
    friend constexpr const_iterator begin(const cexpr_array &a) { return a.begin();}
    friend constexpr const_iterator end  (const cexpr_array &a) { return a.end();}
    friend constexpr iterator begin(cexpr_array &a) { return a.begin();}
    friend constexpr iterator end  (cexpr_array &a) { return a.end();}

};

template<class T = float>
struct gvec2 {
    constexpr gvec2(T x, T y) : x(x), y(y) { }
    constexpr gvec2(T v = 0.0f) : x(v), y(v) { }
    constexpr gvec2(const gvec2&) = default;
    constexpr gvec2(gvec2 &&) noexcept = default;
    gvec2&operator=(gvec2 &&)noexcept = default;
    gvec2&operator=(const gvec2&)noexcept = default;

    template<class Y>
    constexpr gvec2(const gvec2<Y>&o) : x(o.x) , y(o.y){}
    template<class Y>
    constexpr gvec2(gvec2<Y>&o) noexcept : x(o.x) , y(o.y){}

    template<class Y>
    gvec2&operator = (const gvec2<Y>&o) {x=o.x;y=o.y;return *this;}
    template<class Y>
    gvec2&operator = (gvec2<Y>&o) noexcept {x=o.x;y=o.y;return *this;}

    constexpr gvec2 operator*(const gvec2 &v) const { return gvec2(x*v.x, y*v.y); }
    constexpr gvec2 operator/(const gvec2 &v) const { return gvec2(x/v.x, y/v.y); }
    constexpr gvec2 operator+(const gvec2 &o) const { return gvec2(x+o.x, y+o.y); }
    constexpr gvec2 operator-(const gvec2 &o) const { return gvec2(x-o.x, y-o.y); }
    constexpr gvec2 operator-() const { return gvec2(-x, -y); }
    gvec2 &operator+=(const gvec2 &o) {
        x += o.x;
        y += o.y;
        return *this;
    }
    gvec2 &operator-=(const gvec2 &o) {
        x -= o.x;
        y -= o.y;
        return *this;
    }
    gvec2 &operator=(T v) {
        x = v;
        y = v;
        return *this;
    }
    constexpr bool operator==(const gvec2 &o) const { return o.x == x && o.y == y; }

    friend std::ostream &operator<<(std::ostream &o, const gvec2 &v) {
        o << "gvec2(" << v.x << ", " << v.y << ")";
        return o;
    }
    union {
        struct {
            T x;
            T y;
        };
        struct {
            T s;
            T t;
        };
        struct {
            T start;
            T end;
        };
    };
};
using ivec2 = gvec2<int32_t>;
using uvec2 = gvec2<uint32_t>;
using vec2 = gvec2<float>;
using dvec2 = gvec2<double>;

template<class T = float>
struct gvec3 {

    constexpr gvec3(T x, T y, T z = 0) : x(x), y(y), z(z) { }
    constexpr gvec3(T v = 0.0f) : x(v), y(v), z(v) { }
    template<class Y>
    constexpr gvec3(const gvec2<Y> &v, T z = 0) : x(v.x), y(v.y), z(z) { }

    constexpr gvec3(gvec3 &&)noexcept = default;
    constexpr gvec3(const gvec3 &)= default;

    gvec3&operator=(gvec3&&)noexcept = default;
    gvec3&operator=(const gvec3&)= default;

    template<class Y>
    constexpr gvec3(gvec3<Y> &&o)noexcept : x(o.x),y(o.y),z(o.z){}
    template<class Y>
    constexpr gvec3(const gvec3<Y> && o): x(o.x),y(o.y),z(o.z){}

    template<class Y>
    gvec3&operator=(gvec3<Y>&&o)noexcept {x=o.x;y=o.y;z=o.z;return *this;}
    template<class Y>
    gvec3&operator=(const gvec3<Y>&o){x=o.x;y=o.y;z=o.z;return *this;}


    constexpr gvec2<T> project2D(T farPlane) const {
        auto  zScale = (farPlane - z) / farPlane;
        return gvec2<T>(x / zScale, y / zScale);
    }
    constexpr gvec3 operator*(const gvec3 &v) const { return gvec3(x*v.x, y*v.y, z*v.z); }
    constexpr gvec3 operator/(const gvec3 &v) const { return gvec3(x/v.x, y/v.y, z/v.z); }
    constexpr gvec3 operator+(const gvec3 &o) const { return gvec3(x+o.x, y+o.y, z+o.z); }
    constexpr gvec3 operator-(const gvec3 &o) const { return gvec3(x-o.x, y-o.y, z+o.z); }
    constexpr gvec3 operator-() const { return gvec3(-x, -y, -z); }
    gvec3 &operator+=(const gvec3 &o) {
        x += o.x;
        y += o.y;
        z += o.z;
        return *this;
    }
    gvec3 &operator-=(const gvec3 &o) {
        x -= o.x;
        y -= o.y;
        z -= o.z;
        return *this;
    }
    gvec3 &operator=(T v) {
        x = v; y = v; z = v; return *this;
    }
    constexpr bool operator==(const gvec3 &o) const {
        return o.x == x && o.y == y && o.z == z;
    }

    friend std::ostream &operator<<(std::ostream &o, const gvec3 &v) {
        return (o << "gvec3(" << v.x << ", " << v.y << ", " << v.z << ")");
    }
    union {
        struct { T x; T y; T z; };
        struct { T r; T g; T b; };
        struct { T red; T green; T blue; };
    };
};
using ivec3 = gvec3<int32_t>;
using uvec3 = gvec3<uint32_t>;
using vec3 = gvec3<float>;
using dvec3 = gvec3<double>;

template<class T = float>
struct gvec4 {

    constexpr gvec4(const gvec4&) = default;
    constexpr gvec4(gvec4&&)noexcept = default;
    gvec4&operator=(gvec4&&)noexcept = default;
    gvec4&operator=(const gvec4&)noexcept = default;

    template<class Y>
    constexpr gvec4(gvec4<Y> &&o)noexcept : x(o.x),y(o.y),z(o.z),w(o.w){}
    template<class Y>
    constexpr gvec4(const gvec4<Y> && o): x(o.x),y(o.y),z(o.z),w(o.w){}

    template<class Y>
    gvec4&operator=(gvec4<Y>&&o)noexcept {x=o.xey=o.y;z=o.z;w=o.w;return *this;}
    template<class Y>
    gvec4&operator=(const gvec4<Y>&o)noexcept{x=o.x;y=o.y;z=o.z;w=o.w;return *this;}


    constexpr gvec4(T x, T y, T z=0, T w=0) : x(x), y(y), z(z), w(w) { }
    constexpr gvec4(T v = 0.0f) : x(v), y(v), z(v), w(v) { }
    template<class Y>
    constexpr gvec4(const gvec2<Y> &v, T z = 0, T w = 0) : x(v.x), y(v.y), z(z), w(w) { }
    template<class Y>
    constexpr gvec4(const gvec3<Y> &v, T w = 0) : x(v.x), y(v.y), z(v.z), w(w) { }

    constexpr gvec4 operator*(const gvec4 &v) const { return gvec4(x*v.x, y*v.y, z*v.z, w*v.w); }
    constexpr gvec4 operator/(const gvec4 &v) const { return gvec4(x/v.x, y/v.y, z/v.z, w/v.w); }
    constexpr gvec4 operator+(const gvec4 &o) const { return gvec4(x+o.x, y+o.y, z+o.z, w+o.w); }
    constexpr gvec4 operator-(const gvec4 &o) const { return gvec4(x-o.x, y-o.y, z-o.z, w-o.w); }
    constexpr gvec4 operator-() const { return gvec4(-x, -y, -z, -w); }
    gvec4 &operator+=(const gvec4 &o) {
        x += o.x;
        y += o.y;
        z += o.z;
        w += o.w;
        return *this;
    }
    gvec4 &operator-=(const gvec4 &o) {
        x -= o.x;
        y -= o.y;
        z -= o.z;
        w -= o.w;
        return *this;
    }
    gvec4 &operator=(T v) {
        x = v;
        y = v;
        z = v;
        w = v;
        return *this;
    }
    constexpr bool operator==(const gvec4 &o) const {
        return o.x == x && o.y == y && o.z == z && o.w == w;
    }
    friend std::ostream &operator<<(std::ostream &o, const gvec4 &v) {
        o << "gvec4(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
        return o;
    }
    union{
        struct{ T x; T y; T z; T w; };
        struct{ T r; T g; T b; T a; };
        struct{ T s; T t; T u; T v; };
        struct{ T red; T green; T blue;T alpha; };
        struct { T vstart;T vcount;T istart;T icount; };
        struct { T left; T top; T width; T height; };
    };
};
using ivec4 = gvec4<int32_t>;
using uvec4 = gvec4<uint32_t>;
using vec4 = gvec4<float>;
using dvec4 = gvec4<double>;


template<class T>
constexpr gvec2<T> floor(const gvec2<T> &v) { return gvec2<T>(std::floor(v.x), std::floor(v.y)); }
template<class T>
constexpr gvec3<T> floor(const gvec3<T> &v) { return gvec3<T>(std::floor(v.x), std::floor(v.y), std::floor(v.z)); }
template<class T>
constexpr gvec4<T> floor(const gvec4<T> &v) { return gvec4<T>(std::floor(v.x), std::floor(v.y), std::floor(v.z), std::floor(v.w)); }

template<class T>
constexpr gvec2<T> ceil(const gvec2<T> &v) { return gvec2<T>(std::ceil(v.x), std::ceil(v.y)); }
template<class T>
constexpr gvec3<T> ceil(const gvec3<T> &v) { return gvec3<T>(std::ceil(v.x), std::ceil(v.y), std::ceil(v.z)); }
template<class T>
constexpr gvec4<T> ceil(const gvec4<T> &v) { return gvec4<T>(std::ceil(v.x), std::ceil(v.y), std::ceil(v.z), std::ceil(v.w)); }

template<class T>
constexpr gvec2<T> round(const gvec2<T> &v) { return gvec2<T>(std::round(v.x), std::round(v.y)); }
template<class T>
constexpr gvec3<T> round(const gvec3<T> &v) { return gvec3<T>(std::round(v.x), std::round(v.y), std::round(v.z)); }
template<class T>
constexpr gvec4<T> round(const gvec4<T> &v) { return gvec4<T>(std::round(v.x), std::round(v.y), std::round(v.z), std::round(v.w)); }

template<class T>
constexpr gvec2<T> min(const gvec2<T> &a, const gvec2<T> &b) { return gvec2<T>(std::min(a.x, b.x), std::min(a.y, b.y)); }
template<class T>
constexpr gvec3<T> min(const gvec3<T> &a, const gvec3<T> &b) { return gvec3<T>(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z)); }
template<class T>
constexpr gvec4<T> min(const gvec4<T> &a, const gvec4<T> &b) { return gvec4<T>(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z), std::min(a.w, b.w)); }

template<class T>
constexpr gvec2<T> max(const gvec2<T> &a, const gvec2<T> &b) { return gvec2<T>(std::max(a.x, b.x), std::max(a.y, b.y)); }
template<class T>
constexpr gvec3<T> max(const gvec3<T> &a, const gvec3<T> &b) { return gvec3<T>(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z)); }
template<class T>
constexpr gvec4<T> max(const gvec4<T> &a, const gvec4<T> &b) { return gvec4<T>(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z), std::max(a.w, b.w)); }

struct mat4 {
    enum Type {
        Identity         = 0x00,
        Translation2D    = 0x01,
        Scale2D          = 0x02,
        Rotation2D       = 0x04,
        ScaleAndRotate2D = 0x07, // all of the above
        Generic          = 0xff,
    };

    constexpr mat4()
        : m{ 1, 0, 0, 0,
             0, 1, 0, 0,
             0, 0, 1, 0,
             0, 0, 0, 1 }
        , type(Identity)
    { }

    constexpr mat4(float m11, float m12, float m13, float m14,
         float m21, float m22, float m23, float m24,
         float m31, float m32, float m33, float m34,
         float m41, float m42, float m43, float m44,
         unsigned type = Generic)
        : m{ m11, m12, m13, m14,
             m21, m22, m23, m24,
             m31, m32, m33, m34,
             m41, m42, m43, m44 }
        , type(type)
    {
    }

    constexpr bool operator==(const mat4 &o) const {
        return    m[ 0] == o.m[ 0]
               && m[ 1] == o.m[ 1]
               && m[ 2] == o.m[ 2]
               && m[ 3] == o.m[ 3]
               && m[ 4] == o.m[ 4]
               && m[ 5] == o.m[ 5]
               && m[ 6] == o.m[ 6]
               && m[ 7] == o.m[ 7]
               && m[ 8] == o.m[ 8]
               && m[ 9] == o.m[ 9]
               && m[10] == o.m[10]
               && m[11] == o.m[11]
               && m[12] == o.m[12]
               && m[13] == o.m[13]
               && m[14] == o.m[14]
               && m[15] == o.m[15];
    }

    constexpr mat4 operator*(const mat4 &o) const {

        if (type == Translation2D && o.type == Translation2D) {
            return mat4(1, 0, 0, m[3]+o.m[3],
                        0, 1, 0, m[7]+o.m[7],
                        0, 0, 1, 0,
                        0, 0, 0, 1,
                        Translation2D);

        } else if (type == Translation2D) {
            return mat4(o.m[ 0] + m[ 3] * o.m[12],
                        o.m[ 1] + m[ 3] * o.m[13],
                        o.m[ 2] + m[ 3] * o.m[14],
                        o.m[ 3] + m[ 3] * o.m[15],
                        o.m[ 4] + m[ 7] * o.m[12],
                        o.m[ 5] + m[ 7] * o.m[13],
                        o.m[ 6] + m[ 7] * o.m[14],
                        o.m[ 7] + m[ 7] * o.m[15],
                        o.m[ 8],
                        o.m[ 9],
                        o.m[10],
                        o.m[11],
                        o.m[12],
                        o.m[13],
                        o.m[14],
                        o.m[15],
                        type | o.type);

        } else if (o.type == Translation2D) {
            return mat4(m[ 0], m[ 1], m[ 2], m[ 0] * o.m[3] + m[ 1] * o.m[7] + m[ 3],
                        m[ 4], m[ 5], m[ 6], m[ 4] * o.m[3] + m[ 5] * o.m[7] + m[ 7],
                        m[ 8], m[ 9], m[10], m[ 8] * o.m[3] + m[ 9] * o.m[7] + m[11],
                        m[12], m[13], m[14], m[12] * o.m[3] + m[13] * o.m[7] + m[15],
                        Type(type | o.type));

        } else if (type <= ScaleAndRotate2D && o.type <= ScaleAndRotate2D) {
            return mat4(m[ 0] * o.m[0] + m[ 1] * o.m[4],
                        m[ 0] * o.m[1] + m[ 1] * o.m[5],
                        0,
                        m[ 0] * o.m[3] + m[ 1] * o.m[7] + m[ 3],

                        m[ 4] * o.m[0] + m[ 5] * o.m[4],
                        m[ 4] * o.m[1] + m[ 5] * o.m[5],
                        0,
                        m[ 4] * o.m[3] + m[ 5] * o.m[7] + m[ 7],

                        0, 0, 1, 0,
                        0, 0, 0, 1,
                        Type(type | o.type)) ;
            }

        // Genereic full multiplication
        return mat4(m[ 0] * o.m[0] + m[ 1] * o.m[4] + m[ 2] * o.m[ 8] + m[ 3] * o.m[12],
                    m[ 0] * o.m[1] + m[ 1] * o.m[5] + m[ 2] * o.m[ 9] + m[ 3] * o.m[13],
                    m[ 0] * o.m[2] + m[ 1] * o.m[6] + m[ 2] * o.m[10] + m[ 3] * o.m[14],
                    m[ 0] * o.m[3] + m[ 1] * o.m[7] + m[ 2] * o.m[11] + m[ 3] * o.m[15],

                    m[ 4] * o.m[0] + m[ 5] * o.m[4] + m[ 6] * o.m[ 8] + m[ 7] * o.m[12],
                    m[ 4] * o.m[1] + m[ 5] * o.m[5] + m[ 6] * o.m[ 9] + m[ 7] * o.m[13],
                    m[ 4] * o.m[2] + m[ 5] * o.m[6] + m[ 6] * o.m[10] + m[ 7] * o.m[14],
                    m[ 4] * o.m[3] + m[ 5] * o.m[7] + m[ 6] * o.m[11] + m[ 7] * o.m[15],

                    m[ 8] * o.m[0] + m[ 9] * o.m[4] + m[10] * o.m[ 8] + m[11] * o.m[12],
                    m[ 8] * o.m[1] + m[ 9] * o.m[5] + m[10] * o.m[ 9] + m[11] * o.m[13],
                    m[ 8] * o.m[2] + m[ 9] * o.m[6] + m[10] * o.m[10] + m[11] * o.m[14],
                    m[ 8] * o.m[3] + m[ 9] * o.m[7] + m[10] * o.m[11] + m[11] * o.m[15],

                    m[12] * o.m[0] + m[13] * o.m[4] + m[14] * o.m[ 8] + m[15] * o.m[12],
                    m[12] * o.m[1] + m[13] * o.m[5] + m[14] * o.m[ 9] + m[15] * o.m[13],
                    m[12] * o.m[2] + m[13] * o.m[6] + m[14] * o.m[10] + m[15] * o.m[14],
                    m[12] * o.m[3] + m[13] * o.m[7] + m[14] * o.m[11] + m[15] * o.m[15],
                    Type(type | o.type)) ;
    }

    constexpr vec2 operator*(const vec2 &v) const {
        return vec2(m[0] * v.x + m[1] * v.y + m[3],
                    m[4] * v.x + m[5] * v.y + m[7]);
    }

    constexpr vec3 operator*(const vec3 &v) const {
        return vec3(m[0] * v.x + m[1] * v.y + m[ 2] * v.z + m[ 3],
                    m[4] * v.x + m[5] * v.y + m[ 6] * v.z + m[ 7],
                    m[8] * v.x + m[9] * v.y + m[10] * v.z + m[11]);
    }

    constexpr vec4 operator*(const vec4 &v) const {
        return vec4(m[ 0] * v.x + m[ 1] * v.y + m[ 2] * v.z + m[ 3] * v.w,
                    m[ 4] * v.x + m[ 5] * v.y + m[ 6] * v.z + m[ 7] * v.w,
                    m[ 8] * v.x + m[ 9] * v.y + m[10] * v.z + m[11] * v.w,
                    m[12] * v.x + m[13] * v.y + m[14] * v.z + m[15] * v.w);
    }

    constexpr mat4 orthographic(float left, float right, float bottom, float top, float near, float far)
    {
        auto res = mat4();
        res(0,0) = 2.f/(right-left);
        res(3,0) = -(right+left)/(right-left);
        res(1,1) = 2.f/(top-bottom);
        res(3,1) = -(top+bottom)/(top-bottom);
        res(2,2) = 2.f/(near-far);
        res(3,2) = (near+far)/(near-far);
        res(3,3) = 1.f;
        return res;
    }
    constexpr mat4 transposed() const {
        return mat4(m[0], m[4], m[ 8], m[12],
                    m[1], m[5], m[ 9], m[12],
                    m[2], m[6], m[10], m[14],
                    m[3], m[7], m[11], m[15]);
    }

    std::pair<mat4,bool> inverted() const {
        mat4 inv;
        inv.m[0] = m[5]  * m[10] * m[15] -
                   m[5]  * m[11] * m[14] -
                   m[9]  * m[6]  * m[15] +
                   m[9]  * m[7]  * m[14] +
                   m[13] * m[6]  * m[11] -
                   m[13] * m[7]  * m[10];
        inv.m[4] = -m[4]  * m[10] * m[15] +
                    m[4]  * m[11] * m[14] +
                    m[8]  * m[6]  * m[15] -
                    m[8]  * m[7]  * m[14] -
                    m[12] * m[6]  * m[11] +
                    m[12] * m[7]  * m[10];
        inv.m[8] = m[4]  * m[9] * m[15] -
                   m[4]  * m[11] * m[13] -
                   m[8]  * m[5] * m[15] +
                   m[8]  * m[7] * m[13] +
                   m[12] * m[5] * m[11] -
                   m[12] * m[7] * m[9];
        inv.m[12] = -m[4]  * m[9] * m[14] +
                     m[4]  * m[10] * m[13] +
                     m[8]  * m[5] * m[14] -
                     m[8]  * m[6] * m[13] -
                     m[12] * m[5] * m[10] +
                     m[12] * m[6] * m[9];
        inv.m[1] = -m[1]  * m[10] * m[15] +
                    m[1]  * m[11] * m[14] +
                    m[9]  * m[2] * m[15] -
                    m[9]  * m[3] * m[14] -
                    m[13] * m[2] * m[11] +
                    m[13] * m[3] * m[10];
        inv.m[5] = m[0]  * m[10] * m[15] -
                   m[0]  * m[11] * m[14] -
                   m[8]  * m[2] * m[15] +
                   m[8]  * m[3] * m[14] +
                   m[12] * m[2] * m[11] -
                   m[12] * m[3] * m[10];
        inv.m[9] = -m[0]  * m[9] * m[15] +
                    m[0]  * m[11] * m[13] +
                    m[8]  * m[1] * m[15] -
                    m[8]  * m[3] * m[13] -
                    m[12] * m[1] * m[11] +
                    m[12] * m[3] * m[9];
        inv.m[13] = m[0]  * m[9] * m[14] -
                    m[0]  * m[10] * m[13] -
                    m[8]  * m[1] * m[14] +
                    m[8]  * m[2] * m[13] +
                    m[12] * m[1] * m[10] -
                    m[12] * m[2] * m[9];
        inv.m[2] = m[1]  * m[6] * m[15] -
                    m[1]  * m[7] * m[14] -
                    m[5]  * m[2] * m[15] +
                    m[5]  * m[3] * m[14] +
                    m[13] * m[2] * m[7] -
                    m[13] * m[3] * m[6];
        inv.m[6] = -m[0]  * m[6] * m[15] +
                    m[0]  * m[7] * m[14] +
                    m[4]  * m[2] * m[15] -
                    m[4]  * m[3] * m[14] -
                    m[12] * m[2] * m[7] +
                    m[12] * m[3] * m[6];
        inv.m[10] = m[0]  * m[5] * m[15] -
                    m[0]  * m[7] * m[13] -
                    m[4]  * m[1] * m[15] +
                    m[4]  * m[3] * m[13] +
                    m[12] * m[1] * m[7] -
                    m[12] * m[3] * m[5];
        inv.m[14] = -m[0]  * m[5] * m[14] +
                     m[0]  * m[6] * m[13] +
                     m[4]  * m[1] * m[14] -
                     m[4]  * m[2] * m[13] -
                     m[12] * m[1] * m[6] +
                     m[12] * m[2] * m[5];
        inv.m[3] = -m[1] * m[6] * m[11] +
                    m[1] * m[7] * m[10] +
                    m[5] * m[2] * m[11] -
                    m[5] * m[3] * m[10] -
                    m[9] * m[2] * m[7] +
                    m[9] * m[3] * m[6];
        inv.m[7] = m[0] * m[6] * m[11] -
                   m[0] * m[7] * m[10] -
                   m[4] * m[2] * m[11] +
                   m[4] * m[3] * m[10] +
                   m[8] * m[2] * m[7] -
                   m[8] * m[3] * m[6];
        inv.m[11] = -m[0] * m[5] * m[11] +
                     m[0] * m[7] * m[9] +
                     m[4] * m[1] * m[11] -
                     m[4] * m[3] * m[9] -
                     m[8] * m[1] * m[7] +
                     m[8] * m[3] * m[5];
        inv.m[15] = m[0] * m[5] * m[10] -
                    m[0] * m[6] * m[9] -
                    m[4] * m[1] * m[10] +
                    m[4] * m[2] * m[9] +
                    m[8] * m[1] * m[6] -
                    m[8] * m[2] * m[5];

        auto det = m[0] * inv.m[0] + m[1] * inv.m[4] + m[2] * inv.m[8] + m[3] * inv.m[12];
        auto invertible = det != 0;
        using std::begin;
        using std::end;

        det = invertible ? 1.0f / det : 0.f;
        for(auto &x : inv.m) x *= det;
        for (int i = 0; i < 16; i++)
            inv.m[i] *= det;

        return std::make_pair(inv,invertible);
    }
    constexpr static mat4 translate2D(float dx, float dy) {
        return mat4(1, 0, 0, dx,
                    0, 1, 0, dy,
                    0, 0, 1, 0,
                    0, 0, 0, 1, Translation2D);
    }

    constexpr static mat4 translate2D(const vec2 &d) {
        return translate2D(d.x, d.y);
    }


    constexpr static mat4 scale2D(float sx, float sy) {
        return mat4(sx,  0,  0, 0,
                     0, sy,  0, 0,
                     0,  0,  1, 0,
                     0,  0,  0, 1, Scale2D);
    }

    constexpr static mat4 scale2D(const vec2 &s) {
        return scale2D(s.x, s.y);
    }

    constexpr static mat4 rotate2D(float radians) {
        auto s = std::sin(radians);
        auto c = std::cos(radians);
        return mat4(c, -s, 0, 0,
                    s,  c, 0, 0,
                    0,  0, 1, 0,
                    0,  0, 0, 1, Rotation2D);
    }

    constexpr static mat4 translate(float dx, float dy, float dz) {
        return mat4(1, 0, 0, dx,
                    0, 1, 0, dy,
                    0, 0, 1, dz,
                    0, 0, 0, 1, Translation2D);
    }

    constexpr static mat4 rotateAroundZ(float radians) { return rotate2D(radians); }

    constexpr static mat4 rotateAroundX(float radians) {
        float s = sin(radians);
        float c = cos(radians);
        return mat4(1, 0,  0, 0,
                    0, c, -s, 0,
                    0, s,  c, 0,
                    0, 0,  0, 1, Generic);
    }

    constexpr static mat4 rotateAroundY(float radians) {
        float s = sin(radians);
        float c = cos(radians);
        return mat4( c, 0,  s, 0,
                     0, 1,  0, 0,
                    -s, 0,  c, 0,
                     0, 0,  0, 1, Generic);
    }

    constexpr static mat4 scale(float sx, float sy, float sz) {
        return mat4(sx,  0,  0, 0,
                     0, sy,  0, 0,
                     0,  0, sz, 0,
                     0   ,   0, 1, Generic);
    }

    constexpr bool isIdentity() const { return type == Identity; }

    constexpr float &operator()(int c, int r) {
        return m[c*4 + r];
    }

    friend std::ostream &operator<<(std::ostream &o, const mat4 &m) {
        using std::begin;
        using std::end;

        o << "mat4(" << m.m[0];
        for(auto x : m.m) o << ", " << x;
        o << ")";
        return o;
    }
    cexpr_array<float,16> m{};
    unsigned type;
};

struct rect2d {
public:
    constexpr rect2d() { }
    constexpr rect2d(const gvec2<float> &tl, const gvec2<float> &br) : tl(tl), br(br) { }
    constexpr rect2d(float x1, float y1, float x2, float y2) : tl(x1, y1), br(x2, y2) { }

    constexpr static rect2d fromPosSize(const gvec2<float> &pos, const gvec2<float> &size) { return rect2d(pos, pos + size); }
    constexpr static rect2d fromXywh(float x, float y, float w, float h) { return rect2d(x, y, x+w, y+h); }
    constexpr static rect2d fromPosSizeCentered(const gvec2<float> &pos, const gvec2<float> &size) { return fromXywhCentered(pos.x, pos.y, size.x, size.y); }
    constexpr static rect2d fromXywhCentered(float cx, float cy, float w, float h) { return fromXywh(cx-w/2.0f, cy-h/2.0f, w, h); }

    constexpr float top() const { return tl.y; }
    constexpr float left() const { return tl.x; }
    constexpr float bottom() const { return br.y; }
    constexpr float right() const { return br.x; }

    constexpr float x() const { return tl.x; }
    constexpr float y() const { return tl.y; }
    constexpr float width() const { return br.x - tl.x; }
    constexpr float height() const { return br.y - tl.y; }

    void setX(float x) { br.x += (x - tl.x); tl.x = x; }
    void setY(float y) { br.y += (y - tl.y); tl.y = y; }
    void setWidth(float w) { br.x = tl.x + w; }
    void setHeight(float h) { br.y = tl.y + h; }

    constexpr gvec2<float> position() const { return tl; }
    void setPosition(const gvec2<float> &position) {
        br = position + gvec2<float>(width(), height());
        tl = position;
    }

    constexpr gvec2<float> size() const { return gvec2<float>(width(), height()); }
    void setSize(const gvec2<float> &size) { br = tl + size; }

    constexpr rect2d normalized() const { return rect2d(min(tl, br), max(tl, br)); }

    constexpr rect2d operator|(const gvec2<float> &p) const {
        return rect2d(p.x < tl.x ? p.x : tl.x,
                      p.y < tl.y ? p.y : tl.y,
                      p.x > br.x ? p.x : br.x,
                      p.y > br.y ? p.y : br.y);
    }

    rect2d &operator|=(const gvec2<float> &p) {
        if (p.x < tl.x) tl.x = p.x;
        if (p.y < tl.y) tl.y = p.y;
        if (p.x > br.x) br.x = p.x;
        if (p.y > br.y) br.y = p.y;
        return *this;
    }

    rect2d &operator|=(const rect2d &r) {
        (*this) |= r.tl;
        (*this) |= r.br;
        return *this;
    }

    constexpr rect2d operator|(const rect2d &r) {
        return (*this) | r.tl | r.br;
    }

    constexpr bool operator==(const rect2d &o) const { return tl == o.tl && br == o.br; }

    constexpr bool contains(const gvec2<float> &p) const {
        assert(tl.x < br.x);
        assert(tl.y < br.y);
        return p.x >= tl.x && p.x <= br.x
            && p.y >= tl.y && p.y <= br.y;
    }

    constexpr rect2d aligned() const {
        return rect2d(std::floor(tl.x), std::floor(tl.y),
                      std::ceil(br.x), std::ceil(br.y));
    }

    constexpr gvec2<float> center() const { return (tl + br) / 2.0; }

    friend std::ostream &operator<<(std::ostream &o, const rect2d &r) {
    o << "rect2d(" << r.tl << ", " << r.br << ")";
    return o;
}
    gvec2<float> tl{};
    gvec2<float> br{};
};






//RENGINE_END_NAMESPACE
