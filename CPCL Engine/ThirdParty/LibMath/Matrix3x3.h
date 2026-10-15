#pragma once

#include "Vec3.h"
#include <array>
#include <limits>
#include <utility>

namespace Maths
{
    // Row-major storage, column vectors: result = matrix * vector.
    // A * B applies B first. Angles are radians, right-handed rotations.
    template <std::floating_point T = float>
    class Matrix3x3
    {
    public:
        T values[3][3];

        Matrix3x3(); // Identity.
        explicit Matrix3x3(const std::array<T, 9>& elements);
        static Matrix3x3 Identity();
        static Matrix3x3 Zero();
        T& operator()(std::size_t row, std::size_t column);
        const T& operator()(std::size_t row, std::size_t column) const;
        Matrix3x3 operator+(const Matrix3x3& rhs) const;
        Matrix3x3 operator-(const Matrix3x3& rhs) const;
        Matrix3x3 operator*(const Matrix3x3& rhs) const;
        Vec3<T> operator*(const Vec3<T>& rhs) const;
        Matrix3x3 operator*(T scalar) const;
        Matrix3x3& operator*=(const Matrix3x3& rhs);
        bool operator==(const Matrix3x3& rhs) const;
        bool operator!=(const Matrix3x3& rhs) const;
        Matrix3x3 Transpose() const;
        T Determinant() const;
        // Gauss-Jordan with scaled partial pivoting. Rejects singular/ill-conditioned input.
        Matrix3x3 Inverse(T relativeTolerance = T{64} * std::numeric_limits<T>::epsilon()) const;
        static Matrix3x3 Scale(const Vec3<T>& scale);
        static Matrix3x3 RotationX(T radians);
        static Matrix3x3 RotationY(T radians);
        static Matrix3x3 RotationZ(T radians);
    };

    template <std::floating_point T>
    Matrix3x3<T>::Matrix3x3() : values{}
    {
        for (std::size_t i = 0; i < 3; ++i)
        {
            values[i][i] = T{1};
        }
    }

    template <std::floating_point T>
    Matrix3x3<T>::Matrix3x3(const std::array<T, 9>& elements) : values{}
    {
        for (std::size_t row = 0; row < 3; ++row)
        {
            for (std::size_t column = 0; column < 3; ++column)
            {
                values[row][column] = elements[row * 3 + column];
            }
        }
    }

    template <std::floating_point T>
    Matrix3x3<T> Matrix3x3<T>::Identity()
    {
        return Matrix3x3();
    }

    template <std::floating_point T>
    Matrix3x3<T> Matrix3x3<T>::Zero()
    {
        return Matrix3x3(std::array<T, 9>{});
    }

    template <std::floating_point T>
    T& Matrix3x3<T>::operator()(std::size_t row, std::size_t column)
    {
        if (row >= 3 || column >= 3)
        {
            throw std::out_of_range("Matrix index is out of range");
        }
        return values[row][column];
    }

    template <std::floating_point T>
    const T& Matrix3x3<T>::operator()(std::size_t row, std::size_t column) const
    {
        if (row >= 3 || column >= 3)
        {
            throw std::out_of_range("Matrix index is out of range");
        }
        return values[row][column];
    }

    template <std::floating_point T>
    Matrix3x3<T> Matrix3x3<T>::operator+(const Matrix3x3& rhs) const
    {
        Matrix3x3 result = Zero();
        for (std::size_t row = 0; row < 3; ++row)
        {
            for (std::size_t column = 0; column < 3; ++column)
            {
                result.values[row][column] = values[row][column] + rhs.values[row][column];
            }
        }
        return result;
    }

    template <std::floating_point T>
    Matrix3x3<T> Matrix3x3<T>::operator-(const Matrix3x3& rhs) const
    {
        Matrix3x3 result = Zero();
        for (std::size_t row = 0; row < 3; ++row)
        {
            for (std::size_t column = 0; column < 3; ++column)
            {
                result.values[row][column] = values[row][column] - rhs.values[row][column];
            }
        }
        return result;
    }

    template <std::floating_point T>
    Matrix3x3<T> Matrix3x3<T>::operator*(const Matrix3x3& rhs) const
    {
        Matrix3x3 result = Zero();
        for (std::size_t row = 0; row < 3; ++row)
        {
            for (std::size_t column = 0; column < 3; ++column)
            {
                for (std::size_t k = 0; k < 3; ++k)
                {
                    result.values[row][column] += values[row][k] * rhs.values[k][column];
                }
            }
        }
        return result;
    }

    template <std::floating_point T>
    Vec3<T> Matrix3x3<T>::operator*(const Vec3<T>& rhs) const
    {
        return {
            values[0][0] * rhs.x + values[0][1] * rhs.y + values[0][2] * rhs.z,
            values[1][0] * rhs.x + values[1][1] * rhs.y + values[1][2] * rhs.z,
            values[2][0] * rhs.x + values[2][1] * rhs.y + values[2][2] * rhs.z
        };
    }

    template <std::floating_point T>
    Matrix3x3<T> Matrix3x3<T>::operator*(T scalar) const
    {
        Matrix3x3 result = Zero();
        for (std::size_t row = 0; row < 3; ++row)
        {
            for (std::size_t column = 0; column < 3; ++column)
            {
                result.values[row][column] = values[row][column] * scalar;
            }
        }
        return result;
    }

    template <std::floating_point T>
    Matrix3x3<T>& Matrix3x3<T>::operator*=(const Matrix3x3& rhs)
    {
        *this = *this * rhs;
        return *this;
    }

    template <std::floating_point T>
    bool Matrix3x3<T>::operator==(const Matrix3x3& rhs) const
    {
        for (std::size_t row = 0; row < 3; ++row)
        {
            for (std::size_t column = 0; column < 3; ++column)
            {
                if (values[row][column] != rhs.values[row][column])
                {
                    return false;
                }
            }
        }
        return true;
    }

    template <std::floating_point T>
    bool Matrix3x3<T>::operator!=(const Matrix3x3& rhs) const
    {
        return !(*this == rhs);
    }

    template <std::floating_point T>
    Matrix3x3<T> Matrix3x3<T>::Transpose() const
    {
        Matrix3x3 result = Zero();
        for (std::size_t row = 0; row < 3; ++row)
        {
            for (std::size_t column = 0; column < 3; ++column)
            {
                result.values[column][row] = values[row][column];
            }
        }
        return result;
    }

    template <std::floating_point T>
    T Matrix3x3<T>::Determinant() const
    {
        Matrix3x3 working = *this;
        T determinant = T{1};
        for (std::size_t column = 0; column < 3; ++column)
        {
            std::size_t pivot = column;
            for (std::size_t row = column + 1; row < 3; ++row)
            {
                if (std::abs(working.values[row][column]) > std::abs(working.values[pivot][column]))
                {
                    pivot = row;
                }
            }
            if (working.values[pivot][column] == T{0})
            {
                return T{0};
            }
            if (pivot != column)
            {
                for (std::size_t j = 0; j < 3; ++j)
                {
                    std::swap(working.values[pivot][j], working.values[column][j]);
                }
                determinant = -determinant;
            }
            const T diagonal = working.values[column][column];
            determinant *= diagonal;
            for (std::size_t row = column + 1; row < 3; ++row)
            {
                const T factor = working.values[row][column] / diagonal;
                for (std::size_t j = column + 1; j < 3; ++j)
                {
                    working.values[row][j] -= factor * working.values[column][j];
                }
            }
        }
        return determinant;
    }

    template <std::floating_point T>
    Matrix3x3<T> Matrix3x3<T>::Inverse(T relativeTolerance) const
    {
        if (!std::isfinite(relativeTolerance) || relativeTolerance < T{0} || relativeTolerance >= T{1})
        {
            throw std::invalid_argument("Inverse tolerance must be finite and in [0, 1)");
        }
        Matrix3x3 left = *this;
        Matrix3x3 right;
        T scales[3]{};
        for (std::size_t row = 0; row < 3; ++row)
        {
            for (std::size_t column = 0; column < 3; ++column)
            {
                if (!std::isfinite(left.values[row][column]))
                {
                    throw std::domain_error("Cannot invert a non-finite matrix");
                }
                scales[row] = std::max(scales[row], std::abs(left.values[row][column]));
            }
            if (scales[row] == T{0})
            {
                throw std::domain_error("Cannot invert a singular matrix");
            }
        }
        for (std::size_t column = 0; column < 3; ++column)
        {
            std::size_t pivot = column;
            for (std::size_t row = column + 1; row < 3; ++row)
            {
                if (std::abs(left.values[row][column]) / scales[row] >
                    std::abs(left.values[pivot][column]) / scales[pivot])
                {
                    pivot = row;
                }
            }
            if (std::abs(left.values[pivot][column]) / scales[pivot] <= relativeTolerance)
            {
                throw std::domain_error("Matrix is singular or too ill-conditioned");
            }
            for (std::size_t j = 0; j < 3; ++j)
            {
                std::swap(left.values[column][j], left.values[pivot][j]);
                std::swap(right.values[column][j], right.values[pivot][j]);
            }
            std::swap(scales[column], scales[pivot]);
            const T diagonal = left.values[column][column];
            for (std::size_t j = 0; j < 3; ++j)
            {
                left.values[column][j] /= diagonal;
                right.values[column][j] /= diagonal;
            }
            for (std::size_t row = 0; row < 3; ++row)
            {
                if (row == column)
                {
                    continue;
                }
                const T factor = left.values[row][column];
                for (std::size_t j = 0; j < 3; ++j)
                {
                    left.values[row][j] -= factor * left.values[column][j];
                    right.values[row][j] -= factor * right.values[column][j];
                }
            }
        }
        for (std::size_t row = 0; row < 3; ++row)
        {
            for (std::size_t column = 0; column < 3; ++column)
            {
                if (!std::isfinite(right.values[row][column]))
                {
                    throw std::domain_error("Inverse is not representable");
                }
            }
        }
        return right;
    }

    template <std::floating_point T>
    Matrix3x3<T> Matrix3x3<T>::Scale(const Vec3<T>& scale)
    {
        Matrix3x3 result;
        result.values[0][0] = scale.x;
        result.values[1][1] = scale.y;
        result.values[2][2] = scale.z;
        return result;
    }

    template <std::floating_point T>
    Matrix3x3<T> Matrix3x3<T>::RotationX(T radians)
    {
        Matrix3x3 result;
        const T cosine = std::cos(radians);
        const T sine = std::sin(radians);
        result.values[1][1] = cosine;
        result.values[2][2] = cosine;
        result.values[1][2] = -sine;
        result.values[2][1] = sine;
        return result;
    }

    template <std::floating_point T>
    Matrix3x3<T> Matrix3x3<T>::RotationY(T radians)
    {
        Matrix3x3 result;
        const T cosine = std::cos(radians);
        const T sine = std::sin(radians);
        result.values[2][2] = cosine;
        result.values[0][0] = cosine;
        result.values[2][0] = -sine;
        result.values[0][2] = sine;
        return result;
    }

    template <std::floating_point T>
    Matrix3x3<T> Matrix3x3<T>::RotationZ(T radians)
    {
        Matrix3x3 result;
        const T cosine = std::cos(radians);
        const T sine = std::sin(radians);
        result.values[0][0] = cosine;
        result.values[1][1] = cosine;
        result.values[0][1] = -sine;
        result.values[1][0] = sine;
        return result;
    }

}
