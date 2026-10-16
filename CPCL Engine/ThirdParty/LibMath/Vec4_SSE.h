#pragma once
#include <xmmintrin.h>
#include <stdexcept>
#include <limits>

namespace Maths
{
	
	
	class Vec4_SSE
	{
	public:

		
		__m128 reg;
		
		Vec4_SSE()
		{
			reg = _mm_setzero_ps();
		};
		Vec4_SSE(__m128 value) : reg(value) {}
		
		Vec4_SSE operator+(const Vec4_SSE& other) const;
		Vec4_SSE operator-(const Vec4_SSE& other) const;
		Vec4_SSE operator*(const Vec4_SSE& other) const;
		Vec4_SSE operator/(const Vec4_SSE& other) const;
		Vec4_SSE operator-() const;
		template <typename T>
		Vec4_SSE operator/(T scalar) const;
		template <typename T>
		Vec4_SSE operator*(T scalar) const;
		Vec4_SSE& operator+= (const Vec4_SSE& other);
		Vec4_SSE& operator-= (const Vec4_SSE& other);
		Vec4_SSE& operator*= (const Vec4_SSE& other);
		Vec4_SSE& operator/= (const Vec4_SSE& other);
		template <typename T>
		Vec4_SSE& operator*= (T scalar);
		template <typename T>
		Vec4_SSE& operator/= (T scalar);
		bool operator==(const Vec4_SSE & other) const;
		bool operator!=(const Vec4_SSE & other) const;
		
		float Dot(const Vec4_SSE& rhs) const;
		__m128 Dot128(const Vec4_SSE& rhs) const;
		float MagnitudeSquared() const;
		float Magnitude() const;
		Vec4_SSE Normalize() const;
		float DistanceSquared(const Vec4_SSE& other) const;
		float Distance(const Vec4_SSE& other) const;
		float Angle(const Vec4_SSE& other) const; 
		Vec4_SSE FromPoint(const Vec3_SSE& point) const;

		template <typename T>
		static Vec4_SSE Lerp(const Vec4_SSE& a, const Vec4_SSE& b, T t);
		static Vec4_SSE Min(const Vec4_SSE& a, const Vec4_SSE& b);
		static Vec4_SSE Max(const Vec4_SSE& a, const Vec4_SSE& b);

		static const Vec4_SSE Zero;
		static const Vec4_SSE One;
		static const Vec4_SSE UnitX;
		static const Vec4_SSE UnitY;
		static const Vec4_SSE UnitZ;
		static const Vec4_SSE UnitW;
	};


	 
	inline Vec4_SSE Vec4_SSE::operator+(const Vec4_SSE& other) const
	{
		return Vec4_SSE(_mm_add_ps(reg, other.reg));
	}

	inline Vec4_SSE Vec4_SSE::operator-(const Vec4_SSE& other) const
	{
		return Vec4_SSE(_mm_sub_ps(reg, other.reg));
	}

	inline Vec4_SSE Vec4_SSE::operator*(const Vec4_SSE& other) const
	{
		return Vec4_SSE(_mm_mul_ps(reg, other.reg));
	}

	inline Vec4_SSE Vec4_SSE::operator/(const Vec4_SSE& other) const
	{
		const __m128 zero = _mm_setzero_ps();

		// Vérifie si au moins une composante du diviseur vaut 0
		const __m128 comparison = _mm_cmpeq_ps(other.reg, zero);

		if (_mm_movemask_ps(comparison) != 0)
		{
			throw std::domain_error("Cannot divide by zero");
		}

		return Vec4_SSE(_mm_div_ps(reg, other.reg));
	}

	inline Vec4_SSE Vec4_SSE::operator-() const
	{
		const __m128 signeMask = _mm_set1_ps(-0.0f);
		return Vec4_SSE(_mm_xor_ps(reg, signeMask));
	}

	template <typename  T>
	inline Vec4_SSE Vec4_SSE::operator/(T scalar) const
	{		
		if (scalar == 0)
		{
			throw std::domain_error("Cannot divide by zero");
		}
		return Vec4_SSE(_mm_div_ps(reg, _mm_set1_ps(scalar)));
	}

	template <typename  T>
	inline Vec4_SSE Vec4_SSE::operator*(T scalar) const
	{
		return Vec4_SSE(_mm_mul_ps(reg, _mm_set1_ps(scalar)));
	}

	inline Vec4_SSE& Vec4_SSE::operator+=(const Vec4_SSE& other)
	{
		*this = _mm_add_ps(reg, other.reg);
		return *this;
	}
	inline Vec4_SSE& Vec4_SSE::operator-=(const Vec4_SSE& other)
	{
		*this = _mm_sub_ps(reg, other.reg);
		return *this;
	}
	inline Vec4_SSE& Vec4_SSE::operator*=(const Vec4_SSE& other)
	{
		*this = _mm_mul_ps(reg, other.reg);
		return *this;
	}
	inline Vec4_SSE& Vec4_SSE::operator/=(const Vec4_SSE& other)
	{

		const __m128 zero = _mm_setzero_ps();

		const __m128 comparison = _mm_cmpeq_ps(other.reg, zero);

		if (_mm_movemask_ps(comparison) != 0)
		{
			throw std::domain_error("Cannot divide by zero");
		}

		*this = _mm_div_ps(reg, other.reg);
		return *this;
	}

	template <typename  T>
	inline Vec4_SSE& Vec4_SSE::operator*=(T scalar)
	{
		*this = _mm_mul_ps(reg, _mm_set1_ps(scalar));
		return *this;
	}

	template <typename  T>
	inline Vec4_SSE& Vec4_SSE::operator/=(T scalar)
	{
		if (scalar == 0)
		{
			throw std::domain_error("Cannot divide by zero");
		}

		*this = _mm_div_ps(reg, _mm_set1_ps(scalar));
		return *this;
	}

	inline bool Vec4_SSE::operator==(const Vec4_SSE& other) const
	{
		
		return _mm_movemask_ps(_mm_cmpeq_ps(reg, other.reg)) == 0xF;

	}
	inline bool Vec4_SSE::operator!=(const Vec4_SSE& other) const
	{
		
		return _mm_movemask_ps(_mm_cmpeq_ps(reg, other.reg)) != 0xF;

	}


	inline float Vec4_SSE::Dot(const Vec4_SSE& other) const
	{
		__m128 mult = _mm_mul_ps(reg, other.reg);

		float x = mult.m128_f32[0];
		float y = mult.m128_f32[1];
		float z = mult.m128_f32[2];
		float w = mult.m128_f32[3];

		return x + y + z + w;
	};

	inline __m128 Vec4_SSE::Dot128(const Vec4_SSE& other) const
	{
		__m128 mult = _mm_mul_ps(reg, other.reg);
		float dot = mult.m128_f32[0]
			+ mult.m128_f32[1]
			+ mult.m128_f32[2]
			+ mult.m128_f32[3];

		return _mm_set1_ps(dot);
	}

	inline float Vec4_SSE::MagnitudeSquared() const
	{
		return Dot(*this);
	}


	inline float Vec4_SSE::Magnitude() const
	{
		__m128 sqrt = _mm_sqrt_ss(Dot128(reg));
		
		return _mm_cvtss_f32(sqrt);
		
	}

	inline Vec4_SSE Vec4_SSE::Normalize() const
	{
		const __m128 zero = _mm_setzero_ps();

		// Vérifie NaN
		const __m128 isNotNaN = _mm_cmpeq_ps(reg, reg);

		// Valeur absolue de chaque composante
		const __m128 signMask = _mm_set1_ps(-0.0f);
		const __m128 absolute = _mm_andnot_ps(signMask, reg);

		// Vérifie Infinity
		const __m128 maxFloat =
			_mm_set1_ps(std::numeric_limits<float>::max());

		const __m128 isFinite =
			_mm_cmple_ps(absolute, maxFloat);

		// Toutes les composantes doivent être finies
		const __m128 valid = _mm_and_ps(isNotNaN, isFinite);

		if (_mm_movemask_ps(valid) != 0xF)
		{
			throw std::domain_error(
				"Cannot normalize a vector containing NaN or Infinity"
			);
		}

		__m128 dot = Dot128(reg);

		if (_mm_movemask_ps(_mm_cmpeq_ps(dot, zero)) == 0xF)
		{
			throw std::domain_error(
				"Cannot normalize a zero vector"
			);
		}

		__m128 magn = _mm_sqrt_ps(dot);

		return Vec4_SSE(_mm_div_ps(reg, magn));
	}

	inline float Vec4_SSE::DistanceSquared(const Vec4_SSE& other) const
	{
		Vec4_SSE diff = *this - other;
		return diff.MagnitudeSquared();
	}

	inline float Vec4_SSE::Distance(const Vec4_SSE& other) const
	{
		Vec4_SSE diff = *this - other;
		return diff.Magnitude();
	}

	inline float Vec4_SSE::Angle(const Vec4_SSE& other) const
	{		
		__m128 min_val = _mm_set1_ps(-1.0f);
		__m128 max_val = _mm_set1_ps(1.0f);
	
		__m128 cos128 = Normalize().Dot128(other.Normalize());
		__m128 clamped = _mm_min_ps(_mm_max_ps(cos128, min_val), max_val);
			float cosine = _mm_cvtss_f32(clamped);
			return std::acos(cosine);		
	}

	template <typename  T>
	inline Vec4_SSE Vec4_SSE::Lerp(const Vec4_SSE& a, const Vec4_SSE& b, T t)
	{
		__m128 tVec = _mm_set1_ps(t);
		__m128 oneMinusT = _mm_sub_ps(_mm_set1_ps(1.0f), tVec);
		__m128 result = _mm_add_ps(_mm_mul_ps(a.reg, oneMinusT), _mm_mul_ps(b.reg, tVec));
		return Vec4_SSE(result);
	}

	inline Vec4_SSE Vec4_SSE::Min(const Vec4_SSE& a, const Vec4_SSE& b)
	{
		return Vec4_SSE(_mm_min_ps(a.reg, b.reg));
	}

	inline Vec4_SSE Vec4_SSE::Max(const Vec4_SSE& a, const Vec4_SSE& b)
	{
		return Vec4_SSE(_mm_max_ps(a.reg, b.reg));
	}

	inline const Vec4_SSE Vec4_SSE::Zero{ _mm_setzero_ps() };
	inline const Vec4_SSE Vec4_SSE::One{ _mm_set1_ps(1) };
	inline const Vec4_SSE Vec4_SSE::UnitX{ _mm_setr_ps(1, 0, 0, 0) };
	inline const Vec4_SSE Vec4_SSE::UnitY{ _mm_setr_ps(0, 1, 0, 0) };
	inline const Vec4_SSE Vec4_SSE::UnitZ{ _mm_setr_ps(0, 0, 1, 0) };
	inline const Vec4_SSE Vec4_SSE::UnitW{ _mm_setr_ps(0, 0, 0, 1) };



	inline Vec4_SSE Vec4_SSE::FromPoint(const Vec3_SSE& point) const
	{
		return Vec4_SSE(_mm_blend_ps(point.reg, _mm_set1_ps(1.0f), 0x8)); // lane 3 = 1.0f
	}

}
