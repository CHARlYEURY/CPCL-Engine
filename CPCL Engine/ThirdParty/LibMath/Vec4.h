#pragma once

#include <algorithm>
#include <cmath>
#include <concepts>
#include <stdexcept>

namespace Maths
{
    // Scalar teaching baseline. No SIMD, custom alignment or forced inlining.
    template <std::floating_point T = float>
    class Vec4
    {
    public:
        T x;
        T y;
        T z;
        T w;

        Vec4();
        Vec4(T _x, T _y, T _z, T _w);
        template <std::floating_point U>
        explicit Vec4(const Vec4<U>& other);

        Vec4 operator+(const Vec4& rhs) const;
        Vec4 operator-(const Vec4& rhs) const;
        Vec4 operator*(const Vec4& rhs) const;
        Vec4 operator/(const Vec4& rhs) const;
        Vec4 operator-() const;
        Vec4 operator*(T scalar) const;
        Vec4 operator/(T scalar) const;
        Vec4& operator+=(const Vec4& rhs);
        Vec4& operator-=(const Vec4& rhs);
        Vec4& operator*=(const Vec4& rhs);
        Vec4& operator/=(const Vec4& rhs);
        Vec4& operator*=(T scalar);
        Vec4& operator/=(T scalar);
        bool operator==(const Vec4& rhs) const;
        bool operator!=(const Vec4& rhs) const;

        T Dot(const Vec4& rhs) const;
        T MagnitudeSquared() const;
        T Magnitude() const;
        // Returns a new vector. Throws domain_error for zero or non-finite input.
        Vec4 Normalize() const;
        T DistanceSquared(const Vec4& rhs) const;
        T Distance(const Vec4& rhs) const;
        T Angle(const Vec4& rhs) const;
        static Vec4 Lerp(const Vec4& a, const Vec4& b, T t);
        static Vec4 Min(const Vec4& a, const Vec4& b);
        static Vec4 Max(const Vec4& a, const Vec4& b);

        static const Vec4 Zero;
        static const Vec4 One;
        static const Vec4 UnitX;
        static const Vec4 UnitY;
        static const Vec4 UnitZ;
        static const Vec4 UnitW;
    };

    template <std::floating_point T>
    Vec4<T>::Vec4() : x(0), y(0), z(0), w(0)
    {

    }

    template <std::floating_point T>
    Vec4<T>::Vec4(T _x, T _y, T _z, T _w) : x(_x), y(_y), z(_z), w(_w)
    {

    }

    template <std::floating_point T>
    template <std::floating_point U>
    Vec4<T>::Vec4(const Vec4<U>& other) : x(static_cast<T>(other.x)), y(static_cast<T>(other.y)), z(static_cast<T>(other.z)), w(static_cast<T>(other.w))
    {

    }

    template <std::floating_point T>
    Vec4<T> Vec4<T>::operator+(const Vec4& rhs) const
    {
        return {x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w};
    }

    template <std::floating_point T>
    Vec4<T> Vec4<T>::operator-(const Vec4& rhs) const
    {
        return {x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w};
    }

    template <std::floating_point T>
    Vec4<T> Vec4<T>::operator*(const Vec4& rhs) const
    {
        return {x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w};
    }

    template <std::floating_point T>
    Vec4<T> Vec4<T>::operator/(const Vec4& rhs) const
    {
        if (rhs.x == T{0} || rhs.y == T{0} || rhs.z == T{0} || rhs.w == T{0})
        {
            throw std::domain_error("Cannot divide by a zero component");
        }

        return {x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w};
    }

    template <std::floating_point T>
    Vec4<T> Vec4<T>::operator-() const
    {
        return {-x, -y, -z, -w};
    }

    template <std::floating_point T>
    Vec4<T> Vec4<T>::operator*(T scalar) const
    {
        return {x * scalar, y * scalar, z * scalar, w * scalar};
    }

    template <std::floating_point T>
    Vec4<T> Vec4<T>::operator/(T scalar) const
    {
        if (scalar == T{0})
        {
            throw std::domain_error("Cannot divide by zero");
        }

        return {x / scalar, y / scalar, z / scalar, w / scalar};
    }

    template <std::floating_point T>
    Vec4<T>& Vec4<T>::operator+=(const Vec4& rhs)
    {
        *this = *this + rhs;
        return *this;
    }

    template <std::floating_point T>
    Vec4<T>& Vec4<T>::operator-=(const Vec4& rhs)
    {
        *this = *this - rhs;
        return *this;
    }

    template <std::floating_point T>
    Vec4<T>& Vec4<T>::operator*=(const Vec4& rhs)
    {
        *this = *this * rhs;
        return *this;
    }

    template <std::floating_point T>
    Vec4<T>& Vec4<T>::operator/=(const Vec4& rhs)
    {
        *this = *this / rhs;
        return *this;
    }

    template <std::floating_point T>
    Vec4<T>& Vec4<T>::operator*=(T scalar)
    {
        *this = *this * scalar;
        return *this;
    }

    template <std::floating_point T>
    Vec4<T>& Vec4<T>::operator/=(T scalar)
    {
        *this = *this / scalar;
        return *this;
    }

    template <std::floating_point T>
    bool Vec4<T>::operator==(const Vec4& rhs) const
    {
        return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
    }

    template <std::floating_point T>
    bool Vec4<T>::operator!=(const Vec4& rhs) const
    {
        return !(*this == rhs);
    }

    template <std::floating_point T>
    T Vec4<T>::Dot(const Vec4& rhs) const
    {
        return x * rhs.x + y * rhs.y + z * rhs.z + w * rhs.w;
    }

    template <std::floating_point T>
    T Vec4<T>::MagnitudeSquared() const
    {
        return Dot(*this);
    }

    template <std::floating_point T>
    T Vec4<T>::Magnitude() const
    {
        return std::hypot(std::hypot(x, y), std::hypot(z, w));
    }

    template <std::floating_point T>
    Vec4<T> Vec4<T>::Normalize() const
    {
        if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z) || !std::isfinite(w))
        {
            throw std::domain_error("Cannot normalize non-finite components");
        }

        // Scaling avoids overflowing/underflowing the squared magnitude.
        const T scale = std::max({std::abs(x), std::abs(y), std::abs(z), std::abs(w)});
        if (scale == T{0})
        {
            throw std::domain_error("Cannot normalize the zero vector");
        }

        const Vec4 scaled = *this / scale;
        return scaled / scaled.Magnitude();
    }

    template <std::floating_point T>
    T Vec4<T>::DistanceSquared(const Vec4& rhs) const
    {
        return (*this - rhs).MagnitudeSquared();
    }

    template <std::floating_point T>
    T Vec4<T>::Distance(const Vec4& rhs) const
    {
        return (*this - rhs).Magnitude();
    }

    template <std::floating_point T>
    T Vec4<T>::Angle(const Vec4& rhs) const
    {
        const T cosine = Normalize().Dot(rhs.Normalize());
        return std::acos(std::clamp(cosine, T{-1}, T{1}));
    }

    template <std::floating_point T>
    Vec4<T> Vec4<T>::Lerp(const Vec4& a, const Vec4& b, T t)
    {
        return a * (T{1} - t) + b * t;
    }

    template <std::floating_point T>
    Vec4<T> Vec4<T>::Min(const Vec4& a, const Vec4& b)
    {
        return {std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z), std::min(a.w, b.w)};
    }

    template <std::floating_point T>
    Vec4<T> Vec4<T>::Max(const Vec4& a, const Vec4& b)
    {
        return {std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z), std::max(a.w, b.w)};
    }

    template <std::floating_point T>
    const Vec4<T> Vec4<T>::Zero{0, 0, 0, 0};

    template <std::floating_point T>
    const Vec4<T> Vec4<T>::One{1, 1, 1, 1};

    template <std::floating_point T>
    const Vec4<T> Vec4<T>::UnitX{1, 0, 0, 0};

    template <std::floating_point T>
    const Vec4<T> Vec4<T>::UnitY{0, 1, 0, 0};

    template <std::floating_point T>
    const Vec4<T> Vec4<T>::UnitZ{0, 0, 1, 0};

    template <std::floating_point T>
    const Vec4<T> Vec4<T>::UnitW{0, 0, 0, 1};

    template <std::floating_point T>
    Vec4<T> operator*(T scalar, const Vec4<T>& vector)
    {
        return vector * scalar;
    }

    using Vec4f = Vec4<float>;
    using Vec4d = Vec4<double>;
}
