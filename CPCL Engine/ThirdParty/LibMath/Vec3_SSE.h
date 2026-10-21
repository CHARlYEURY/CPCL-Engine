#pragma once
#include <xmmintrin.h>
#include <cmath>
#include <stdexcept>

// Sélectionne l'élément pour chaque composante : (z, y, x, w)
#define SHUFFLE_MASK(x, y, z, w) _MM_SHUFFLE(z, y, x, w)

namespace Maths {

	struct UV
	{
		float x, y;
		UV() : x(0), y(0) {}
		UV(float x, float y) : x(x), y(y) {}
	};

	class Vec3_SSE {
	public:

		__m128 reg;

		Vec3_SSE() : reg(_mm_setzero_ps()) {}

		Vec3_SSE(float x, float y, float z) {
			reg = _mm_setr_ps(x, y, z, 0.0f);
		}

		Vec3_SSE(__m128 val) : reg(val) {}

		Vec3_SSE operator+(const Vec3_SSE& rhs) const;
		Vec3_SSE operator-(const Vec3_SSE& rhs) const;
		Vec3_SSE operator*(const Vec3_SSE& rhs) const;
		Vec3_SSE operator/(const Vec3_SSE& rhs) const;
		Vec3_SSE operator-() const;

		template <typename T>
		Vec3_SSE operator*(T scalar) const;
		template <typename T>
		Vec3_SSE operator/(T scalar) const;

		Vec3_SSE& operator+=(const Vec3_SSE& rhs);
		Vec3_SSE& operator-=(const Vec3_SSE& rhs);
		Vec3_SSE& operator*=(const Vec3_SSE& rhs);
		Vec3_SSE& operator/=(const Vec3_SSE& rhs);

		template <typename T>
		Vec3_SSE& operator*=(T scalar);
		template <typename T>
		Vec3_SSE& operator/=(T scalar);

		bool operator==(const Vec3_SSE& rhs) const;
		bool operator!=(const Vec3_SSE& rhs) const;

		float Dot(const Vec3_SSE& rhs) const;
		__m128 Dot128(const Vec3_SSE& rhs) const;
		Vec3_SSE Cross(const Vec3_SSE& rhs) const;
		float MagnitudeSquared() const;
		float Magnitude() const;

		Vec3_SSE Normalize() const;
		float DistanceSquared(const Vec3_SSE& rhs) const;
		float Distance(const Vec3_SSE& rhs) const;
		float Angle(const Vec3_SSE& rhs) const;

		static Vec3_SSE Lerp(const Vec3_SSE& a, const Vec3_SSE& b, float t);
		static Vec3_SSE Min(const Vec3_SSE& a, const Vec3_SSE& b);
		static Vec3_SSE Max(const Vec3_SSE& a, const Vec3_SSE& b);

		static const Vec3_SSE Zero;
		static const Vec3_SSE One;
		static const Vec3_SSE UnitX;
		static const Vec3_SSE UnitY;
		static const Vec3_SSE UnitZ;
	};


	inline Vec3_SSE Vec3_SSE::operator+(const Vec3_SSE& rhs) const {
		return _mm_add_ps(reg, rhs.reg);
	}

	inline Vec3_SSE Vec3_SSE::operator-(const Vec3_SSE& rhs) const {
		return _mm_sub_ps(reg, rhs.reg);
	}

	inline Vec3_SSE Vec3_SSE::operator*(const Vec3_SSE& rhs) const {
		return _mm_mul_ps(reg, rhs.reg);
	}

	inline Vec3_SSE Vec3_SSE::operator/(const Vec3_SSE& rhs) const 
	{
		if (rhs == Vec3_SSE())
		{
			throw std::domain_error("Cannot divide by zero");
		}
		return _mm_div_ps(reg, rhs.reg);
	}

	inline Vec3_SSE Vec3_SSE::operator-() const
	{
		const __m128 signeMask = _mm_set1_ps(-0.0f);
		return (_mm_xor_ps(reg, signeMask));
	}

	template <typename T>
	inline Vec3_SSE Vec3_SSE::operator*(T scalar) const
	{
		return (_mm_mul_ps(reg,_mm_set1_ps(scalar)));
	}

	template <typename T>
	inline Vec3_SSE Vec3_SSE::operator/(T scalar) const
	{
		if (scalar == 0)
		{
			throw std::domain_error("Cannot divide by zero");
		}
		return (_mm_div_ps(reg,_mm_set1_ps(scalar)));
	}

	inline Vec3_SSE& Vec3_SSE::operator+=(const Vec3_SSE& rhs)
	{
		*this = _mm_add_ps(reg, rhs.reg);
		return *this;
	}

	inline Vec3_SSE& Vec3_SSE::operator-=(const Vec3_SSE& rhs)
	{
		*this = _mm_sub_ps(reg, rhs.reg);
		return *this;
	}

	inline Vec3_SSE& Vec3_SSE::operator*=(const Vec3_SSE& rhs)
	{
		*this = _mm_mul_ps(reg, rhs.reg);
		return *this;
	}

	inline Vec3_SSE& Vec3_SSE::operator/=(const Vec3_SSE& rhs)
	{
		if (rhs.reg.m128_f32[0] == 0.0f || rhs.reg.m128_f32[1] == 0.0f || rhs.reg.m128_f32[2] == 0.0f)
		{
			throw std::domain_error("Cannot divide by zero component");
		}
		*this = _mm_div_ps(reg, rhs.reg);
		return *this;
	}

	template <typename T>
	inline Vec3_SSE& Vec3_SSE::operator*=(T scalar)
	{
		*this = _mm_mul_ps(reg, _mm_set1_ps(scalar));
		return *this;
	}
	template <typename T>
	inline Vec3_SSE& Vec3_SSE::operator/=(T scalar)
	{
		if (scalar == static_cast<T>(0))
		{
			throw std::domain_error("Cannot divide by zero");
		}
		*this = _mm_div_ps(reg, _mm_set1_ps(scalar));
		return *this;
	}

	inline bool Vec3_SSE::operator==(const Vec3_SSE& rhs) const
	{
		return  (_mm_movemask_ps(_mm_cmpeq_ps(reg, rhs.reg)) & 0x7) == 0x7;
	}

	inline bool Vec3_SSE::operator!=(const Vec3_SSE& rhs) const
	{
		return  (_mm_movemask_ps(_mm_cmpeq_ps(reg, rhs.reg)) & 0x7) != 0x7;
	}

	inline float Vec3_SSE::Dot(const Vec3_SSE& rhs) const 
	{
		__m128 mult = _mm_mul_ps(reg, rhs.reg);

		float x = mult.m128_f32[0];
		float y = mult.m128_f32[1];
		float z = mult.m128_f32[2];

		return x + y + z;
	}

	inline __m128 Vec3_SSE::Dot128(const Vec3_SSE& rhs) const
	{
		__m128 mult = _mm_mul_ps(reg, rhs.reg);
		float dot = mult.m128_f32[0] + mult.m128_f32[1] + mult.m128_f32[2];
		return _mm_set1_ps(dot);
	}

	inline Vec3_SSE Vec3_SSE::Cross(const Vec3_SSE& rhs) const 
	{
		__m128 a1_yzx = _mm_shuffle_ps(reg, reg, _MM_SHUFFLE(3, 0, 2, 1));

		__m128 b1_zxy = _mm_shuffle_ps(rhs.reg, rhs.reg, _MM_SHUFFLE(3, 1, 0, 2));

		__m128 AB1 = _mm_mul_ps(a1_yzx, b1_zxy);

		__m128 a2_zxy = _mm_shuffle_ps(reg, reg, _MM_SHUFFLE(3, 1, 0, 2));

		__m128 b2_yzx = _mm_shuffle_ps(rhs.reg, rhs.reg, _MM_SHUFFLE(3, 0, 2, 1));

		__m128 AB2 = _mm_mul_ps(a2_zxy, b2_yzx);

		return _mm_sub_ps(AB1, AB2);
	}

	inline float Vec3_SSE::MagnitudeSquared() const 
	{
		return Dot(*this);
	}

	inline float Vec3_SSE::Magnitude() const 
	{
		__m128 sqrt = _mm_sqrt_ss(Dot128(reg));

		return _mm_cvtss_f32(sqrt);
	}

	inline Vec3_SSE Vec3_SSE::Normalize() const 
	{
		float x = reg.m128_f32[0];
		float y = reg.m128_f32[1];
		float z = reg.m128_f32[2];

		if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z))
		{
			throw std::domain_error("Cannot normalize non-finite components");
		}

		float sqrMag = MagnitudeSquared();
		if (sqrMag == 0.0f)
		{
			throw std::domain_error("Cannot normalize zero vector");
		}

		__m128 dot = Dot128(reg);

		__m128 mag = _mm_sqrt_ps(dot);

		return _mm_div_ps(reg, mag);
	}

	inline float Vec3_SSE::DistanceSquared(const Vec3_SSE& rhs) const 
	{
		return (*this - rhs).MagnitudeSquared();
	}

	inline float Vec3_SSE::Distance(const Vec3_SSE& rhs) const 
	{
		return (*this - rhs).Magnitude();
	}

	inline float Vec3_SSE::Angle(const Vec3_SSE& rhs) const 
	{
		__m128 min_val = _mm_set1_ps(-1.0f);
		__m128 max_val = _mm_set1_ps(1.0f);

		__m128 cos128 = Normalize().Dot128(rhs.Normalize());
		__m128 clamped = _mm_min_ps(_mm_max_ps(cos128, min_val), max_val);
		float cosine = _mm_cvtss_f32(clamped);
		return std::acos(cosine);
	}

	inline Vec3_SSE Vec3_SSE::Lerp(const Vec3_SSE& a, const Vec3_SSE& b, float t) 
	{
		__m128 one128 = _mm_set1_ps(1.0f);
		__m128 t128 = _mm_set1_ps(t);

		__m128 mult1 = _mm_mul_ps(a.reg, (_mm_sub_ps(one128, t128)));

		__m128 mult2 = _mm_mul_ps(b.reg, t128);

		return _mm_add_ps(mult1, mult2);
	}

	inline Vec3_SSE Vec3_SSE::Min(const Vec3_SSE& a, const Vec3_SSE& b)
	{
		return _mm_min_ps(a.reg, b.reg);
	}

	inline Vec3_SSE Vec3_SSE::Max(const Vec3_SSE& a, const Vec3_SSE& b)
	{
		return _mm_max_ps(a.reg, b.reg);
	}

	inline const Vec3_SSE Vec3_SSE::Zero{ 0.0f, 0.0f, 0.0f };
	inline const Vec3_SSE Vec3_SSE::One{ 1.0f, 1.0f, 1.0f };
	inline const Vec3_SSE Vec3_SSE::UnitX{ 1.0f, 0.0f, 0.0f };
	inline const Vec3_SSE Vec3_SSE::UnitY{ 0.0f, 1.0f, 0.0f };
	inline const Vec3_SSE Vec3_SSE::UnitZ{ 0.0f, 0.0f, 1.0f };
}
