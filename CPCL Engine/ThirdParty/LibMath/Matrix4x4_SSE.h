#pragma once
#include <xmmintrin.h>
#include <stdexcept>
#include <limits>
#include <cmath>
#include "Vec3_SSE.h"
#include "Vec4_SSE.h"
#include <array>
#include <assert.h>
#include <softintrin.h>

namespace Maths
{

	class Matrix4x4_SSE
	{
	public: 

		__m128 rows[4];

		Matrix4x4_SSE();
		Matrix4x4_SSE(const std::array<float, 16>& elements);
		static Matrix4x4_SSE Identity();
		static Matrix4x4_SSE Zero();

		float& operator()(std::size_t row, std::size_t column);
		const float& operator()(std::size_t row, std::size_t column) const;

		Matrix4x4_SSE operator+(const Matrix4x4_SSE& other) const;
		Matrix4x4_SSE operator-(const Matrix4x4_SSE& other) const;
		Matrix4x4_SSE operator*(const Matrix4x4_SSE& other) const;
		Vec4_SSE operator*(const Vec4_SSE& other) const;
		template <typename T>
		Matrix4x4_SSE operator*(T scalar) const;
		Matrix4x4_SSE& operator*=(const Matrix4x4_SSE& other);
		bool operator==(const Matrix4x4_SSE& other) const;
		bool operator!=(const Matrix4x4_SSE& other) const;

		Matrix4x4_SSE Transpose() const;
		float Determinant() const;
		Matrix4x4_SSE Inverse() const;

		static Matrix4x4_SSE Scale(const Vec3_SSE& scale);
		static Matrix4x4_SSE RotationX(float radians);
		static Matrix4x4_SSE RotationY(float radians);
		static Matrix4x4_SSE RotationZ(float radians);
		static Matrix4x4_SSE Translation(const Vec3_SSE& offset);

		static Matrix4x4_SSE Perspective(float fovYRadians, float aspect, float zNear, float zFar);
		static Matrix4x4_SSE LookAt(const Vec3_SSE& eye, const Vec3_SSE& target, const Vec3_SSE& up);

		Vec3_SSE TransformPoint(const Vec3_SSE& point) const; // Affine only, w = 1.
		Vec3_SSE TransformDirection(const Vec3_SSE& direction) const; // Affine only, w = 0.
		

		
	};


	namespace Detail
	{
		// Produit vectoriel xyz (la lane w du résultat est 0 si a.w = b.w = 0)
		inline __m128 Cross3(__m128 a, __m128 b)
		{
			const __m128 a_yzx = _mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 0, 2, 1));
			const __m128 b_yzx = _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 0, 2, 1));
			const __m128 c = _mm_sub_ps(_mm_mul_ps(a, b_yzx), _mm_mul_ps(a_yzx, b));
			return _mm_shuffle_ps(c, c, _MM_SHUFFLE(3, 0, 2, 1));
		}

		// Produit scalaire xyz, diffusé dans les 4 lanes
		inline __m128 Dot3(__m128 a, __m128 b)
		{
			return _mm_dp_ps(a, b, 0x7F);
		}

		inline __m128 Normalize3(__m128 v)
		{
			const __m128 len = _mm_sqrt_ps(Dot3(v, v));
			return _mm_div_ps(v, len);
		}
	}


	inline Matrix4x4_SSE::Matrix4x4_SSE()
	{
		rows[0] = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
		rows[1] = _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);
		rows[2] = _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
		rows[3] = _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f);
	}

	inline Matrix4x4_SSE::Matrix4x4_SSE(const std::array<float, 16>& elements)
	{
		rows[0] = _mm_loadu_ps(&elements[0]);
		rows[1] = _mm_loadu_ps(&elements[4]);
		rows[2] = _mm_loadu_ps(&elements[8]);
		rows[3] = _mm_loadu_ps(&elements[12]);
	}

	inline Matrix4x4_SSE Matrix4x4_SSE::Identity()
	{
		return Matrix4x4_SSE();
	}

	inline Matrix4x4_SSE Matrix4x4_SSE::Zero()
	{
		Matrix4x4_SSE result;
		result.rows[0] = _mm_setzero_ps();
		result.rows[1] = _mm_setzero_ps();
		result.rows[2] = _mm_setzero_ps();
		result.rows[3] = _mm_setzero_ps();
		return result;
	}


	inline float& Matrix4x4_SSE::operator()(std::size_t row, std::size_t column)
	{
		assert(row < 4 && column < 4);
		return reinterpret_cast<float*>(rows)[row * 4 + column];
	}

	inline const float& Matrix4x4_SSE::operator()(std::size_t row, std::size_t column) const
	{
		assert(row < 4 && column < 4);
		return reinterpret_cast<const float*>(rows)[row * 4 + column];
	}


	inline Matrix4x4_SSE Matrix4x4_SSE::operator+(const Matrix4x4_SSE& other) const
	{
		Matrix4x4_SSE result;

		result.rows[0] = _mm_add_ps(rows[0], other.rows[0]);
		result.rows[1] = _mm_add_ps(rows[1], other.rows[1]);
		result.rows[2] = _mm_add_ps(rows[2], other.rows[2]);
		result.rows[3] = _mm_add_ps(rows[3], other.rows[3]);

		return result;
	}

	inline Matrix4x4_SSE Matrix4x4_SSE::operator-(const Matrix4x4_SSE& other) const
	{
		Matrix4x4_SSE result;

		result.rows[0] = _mm_sub_ps(rows[0], other.rows[0]);
		result.rows[1] = _mm_sub_ps(rows[1], other.rows[1]);
		result.rows[2] = _mm_sub_ps(rows[2], other.rows[2]);
		result.rows[3] = _mm_sub_ps(rows[3], other.rows[3]);

		return result;
	}

	inline Matrix4x4_SSE Matrix4x4_SSE::operator*(const Matrix4x4_SSE& other) const
	{
		Matrix4x4_SSE result;

		// Lignes de la matrice B
		__m128 b0 = other.rows[0];
		__m128 b1 = other.rows[1];
		__m128 b2 = other.rows[2];
		__m128 b3 = other.rows[3];

		// Ligne 0 de A
		__m128 a = rows[0];

		__m128 a0 = _mm_shuffle_ps(a, a, 0x00);
		__m128 a1 = _mm_shuffle_ps(a, a, 0x55);
		__m128 a2 = _mm_shuffle_ps(a, a, 0xAA);
		__m128 a3 = _mm_shuffle_ps(a, a, 0xFF);

		result.rows[0] =
			_mm_add_ps(
				_mm_add_ps(
					_mm_mul_ps(a0, b0),
					_mm_mul_ps(a1, b1)
				),
				_mm_add_ps(
					_mm_mul_ps(a2, b2),
					_mm_mul_ps(a3, b3)
				)
			);

		// Ligne 1 de A
		a = rows[1];

		a0 = _mm_shuffle_ps(a, a, 0x00);
		a1 = _mm_shuffle_ps(a, a, 0x55);
		a2 = _mm_shuffle_ps(a, a, 0xAA);
		a3 = _mm_shuffle_ps(a, a, 0xFF);

		result.rows[1] =
			_mm_add_ps(
				_mm_add_ps(
					_mm_mul_ps(a0, b0),
					_mm_mul_ps(a1, b1)
				),
				_mm_add_ps(
					_mm_mul_ps(a2, b2),
					_mm_mul_ps(a3, b3)
				)
			);

		// Ligne 2 de A
		a = rows[2];

		a0 = _mm_shuffle_ps(a, a, 0x00);
		a1 = _mm_shuffle_ps(a, a, 0x55);
		a2 = _mm_shuffle_ps(a, a, 0xAA);
		a3 = _mm_shuffle_ps(a, a, 0xFF);

		result.rows[2] =
			_mm_add_ps(
				_mm_add_ps(
					_mm_mul_ps(a0, b0),
					_mm_mul_ps(a1, b1)
				),
				_mm_add_ps(
					_mm_mul_ps(a2, b2),
					_mm_mul_ps(a3, b3)
				)
			);

		// Ligne 3 de A
		a = rows[3];

		a0 = _mm_shuffle_ps(a, a, 0x00);
		a1 = _mm_shuffle_ps(a, a, 0x55);
		a2 = _mm_shuffle_ps(a, a, 0xAA);
		a3 = _mm_shuffle_ps(a, a, 0xFF);

		result.rows[3] =
			_mm_add_ps(
				_mm_add_ps(
					_mm_mul_ps(a0, b0),
					_mm_mul_ps(a1, b1)
				),
				_mm_add_ps(
					_mm_mul_ps(a2, b2),
					_mm_mul_ps(a3, b3)
				)
			);

		return result;
	}

	inline Vec4_SSE Matrix4x4_SSE::operator*(const Vec4_SSE& rhs) const
	{
		const __m128 v = rhs.reg;

		const __m128 p0 = _mm_mul_ps(rows[0], v);
		const __m128 p1 = _mm_mul_ps(rows[1], v);
		const __m128 p2 = _mm_mul_ps(rows[2], v);
		const __m128 p3 = _mm_mul_ps(rows[3], v);

		// Transposition des produits : c0 = [p0x p1x p2x p3x], c1 = [p0y ...], etc.
		const __m128 t0 = _mm_unpacklo_ps(p0, p1);
		const __m128 t1 = _mm_unpackhi_ps(p0, p1);
		const __m128 t2 = _mm_unpacklo_ps(p2, p3);
		const __m128 t3 = _mm_unpackhi_ps(p2, p3);

		const __m128 c0 = _mm_movelh_ps(t0, t2);
		const __m128 c1 = _mm_movehl_ps(t2, t0);
		const __m128 c2 = _mm_movelh_ps(t1, t3);
		const __m128 c3 = _mm_movehl_ps(t3, t1);

		return Vec4_SSE(_mm_add_ps(_mm_add_ps(c0, c1), _mm_add_ps(c2, c3)));
	}


	template <typename T>
	inline Matrix4x4_SSE Matrix4x4_SSE::operator*(T scalar) const
	{
		Matrix4x4_SSE result;

		__m128 scalarVec = _mm_set1_ps(static_cast<float>(scalar));

		result.rows[0] = _mm_mul_ps(rows[0], scalarVec);
		result.rows[1] = _mm_mul_ps(rows[1], scalarVec);
		result.rows[2] = _mm_mul_ps(rows[2], scalarVec);
		result.rows[3] = _mm_mul_ps(rows[3], scalarVec);
		return result;
	}

	inline Matrix4x4_SSE& Matrix4x4_SSE::operator*=(const Matrix4x4_SSE& other)
	{
		*this = *this * other;
		return *this;
	}

	inline bool Matrix4x4_SSE::operator==(const Matrix4x4_SSE& other) const
	{
		return _mm_movemask_ps(_mm_cmpeq_ps(rows[0], other.rows[0])) == 0xF &&
			_mm_movemask_ps(_mm_cmpeq_ps(rows[1], other.rows[1])) == 0xF &&
			_mm_movemask_ps(_mm_cmpeq_ps(rows[2], other.rows[2])) == 0xF &&
			_mm_movemask_ps(_mm_cmpeq_ps(rows[3], other.rows[3])) == 0xF;
	}

	inline bool Matrix4x4_SSE::operator!=(const Matrix4x4_SSE& other) const
	{
		return !(*this == other);
	}

	inline Matrix4x4_SSE Matrix4x4_SSE::Transpose() const
	{
		Matrix4x4_SSE result;

		__m128 tmp0 = _mm_unpacklo_ps(rows[0], rows[1]);
		__m128 tmp1 = _mm_unpackhi_ps(rows[0], rows[1]);
		__m128 tmp2 = _mm_unpacklo_ps(rows[2], rows[3]);
		__m128 tmp3 = _mm_unpackhi_ps(rows[2], rows[3]);

		result.rows[0] = _mm_movelh_ps(tmp0, tmp2);
		result.rows[1] = _mm_movehl_ps(tmp2, tmp0);
		result.rows[2] = _mm_movelh_ps(tmp1, tmp3);
		result.rows[3] = _mm_movehl_ps(tmp3, tmp1);

		return result;
	}
	
	inline float Matrix4x4_SSE::Determinant() const
	{
		// ---------------------------------------------------------
		// Matrice :
		//
		// [ a b c d ]
		// [ e f g h ]
		// [ i j k l ]
		// [ m n o p ]
		// ---------------------------------------------------------

		const __m128 r0 = rows[0];
		const __m128 r1 = rows[1];
		const __m128 r2 = rows[2];
		const __m128 r3 = rows[3];

		// ---------------------------------------------------------
		// On extrait chaque valeur sous forme de broadcast SSE
		//
		// Exemple :
		// [ e f g h ] -> [ e e e e ]
		// ---------------------------------------------------------

		const __m128 e = _mm_shuffle_ps(r1, r1, 0x00);
		const __m128 f = _mm_shuffle_ps(r1, r1, 0x55);
		const __m128 g = _mm_shuffle_ps(r1, r1, 0xAA);
		const __m128 h = _mm_shuffle_ps(r1, r1, 0xFF);

		const __m128 i = _mm_shuffle_ps(r2, r2, 0x00);
		const __m128 j = _mm_shuffle_ps(r2, r2, 0x55);
		const __m128 k = _mm_shuffle_ps(r2, r2, 0xAA);
		const __m128 l = _mm_shuffle_ps(r2, r2, 0xFF);

		const __m128 m = _mm_shuffle_ps(r3, r3, 0x00);
		const __m128 n = _mm_shuffle_ps(r3, r3, 0x55);
		const __m128 o = _mm_shuffle_ps(r3, r3, 0xAA);
		const __m128 p = _mm_shuffle_ps(r3, r3, 0xFF);

		// ---------------------------------------------------------
		// Mineurs 2x2
		//
		// kp - lo
		// jp - ln
		// jo - kn
		// ip - lm
		// io - km
		// in - jm
		// ---------------------------------------------------------

		const __m128 kp_lo =
			_mm_sub_ps(
				_mm_mul_ps(k, p),
				_mm_mul_ps(l, o)
			);

		const __m128 jp_ln =
			_mm_sub_ps(
				_mm_mul_ps(j, p),
				_mm_mul_ps(l, n)
			);

		const __m128 jo_kn =
			_mm_sub_ps(
				_mm_mul_ps(j, o),
				_mm_mul_ps(k, n)
			);

		const __m128 ip_lm =
			_mm_sub_ps(
				_mm_mul_ps(i, p),
				_mm_mul_ps(l, m)
			);

		const __m128 io_km =
			_mm_sub_ps(
				_mm_mul_ps(i, o),
				_mm_mul_ps(k, m)
			);

		const __m128 in_jm =
			_mm_sub_ps(
				_mm_mul_ps(i, n),
				_mm_mul_ps(j, m)
			);

		// ---------------------------------------------------------
		// Cofacteur C0
		//
		// C0 = f(kp-lo) - g(jp-ln) + h(jo-kn)
		// ---------------------------------------------------------

		const __m128 C0 =
			_mm_add_ps(
				_mm_sub_ps(
					_mm_mul_ps(f, kp_lo),
					_mm_mul_ps(g, jp_ln)
				),
				_mm_mul_ps(h, jo_kn)
			);

		// ---------------------------------------------------------
		// Cofacteur C1
		//
		// C1 = -(e(kp-lo) - g(ip-lm) + h(io-km))
		// ---------------------------------------------------------

		const __m128 C1 =
			_mm_sub_ps(
				_mm_mul_ps(g, ip_lm),
				_mm_add_ps(
					_mm_mul_ps(e, kp_lo),
					_mm_mul_ps(h, io_km)
				)
			);

		// ---------------------------------------------------------
		// Cofacteur C2
		//
		// C2 = e(jp-ln) - f(ip-lm) + h(in-jm)
		// ---------------------------------------------------------

		const __m128 C2 =
			_mm_add_ps(
				_mm_sub_ps(
					_mm_mul_ps(e, jp_ln),
					_mm_mul_ps(f, ip_lm)
				),
				_mm_mul_ps(h, in_jm)
			);

		// ---------------------------------------------------------
		// Cofacteur C3
		//
		// C3 = -(e(jo-kn) - f(io-km) + g(in-jm))
		// ---------------------------------------------------------

		const __m128 C3 =
			_mm_sub_ps(
				_mm_mul_ps(f, io_km),
				_mm_add_ps(
					_mm_mul_ps(e, jo_kn),
					_mm_mul_ps(g, in_jm)
				)
			);

		// ---------------------------------------------------------
		// On rassemble les 4 cofacteurs dans un seul __m128
		//
		// [ C0 C1 C2 C3 ]
		// ---------------------------------------------------------

		__m128 cofactors = _mm_unpacklo_ps(C0, C1);
		cofactors = _mm_movelh_ps(cofactors, _mm_unpacklo_ps(C2, C3));

		// ---------------------------------------------------------
		// Première ligne :
		//
		// [a b c d]
		//
		// Déterminant :
		//
		// a*C0 + b*C1 + c*C2 + d*C3
		// ---------------------------------------------------------

		__m128 result = _mm_mul_ps(r0, cofactors);

		// ---------------------------------------------------------
		// Réduction horizontale :
		//
		// [aC0 bC1 cC2 dC3]
		//
		//              ↓
		//
		// [aC0+cC2 bC1+dC3 ...]
		// ---------------------------------------------------------

		result = _mm_add_ps(
			result,
			_mm_movehl_ps(result, result)
		);

		// Maintenant :
		//
		// [aC0+cC2, bC1+dC3, ...]
		//
		// On additionne les deux premières valeurs.

		result = _mm_add_ss(
			result,
			_mm_shuffle_ps(result, result, 0x01)
		);

		return _mm_cvtss_f32(result);
	}

	inline Matrix4x4_SSE Matrix4x4_SSE::Inverse() const
	{
		const __m128 maskXYZ = _mm_castsi128_ps(_mm_set_epi32(0, -1, -1, -1));

		// Dernière "ligne" (x,y,z,w) de la transposée : la lane w de chaque ligne
		const __m128 x = _mm_shuffle_ps(rows[0], rows[0], 0xFF);
		const __m128 y = _mm_shuffle_ps(rows[1], rows[1], 0xFF);
		const __m128 z = _mm_shuffle_ps(rows[2], rows[2], 0xFF);
		const __m128 w = _mm_shuffle_ps(rows[3], rows[3], 0xFF);

		// Colonnes 3D a,b,c,d (w forcé à 0)
		const __m128 a = _mm_and_ps(rows[0], maskXYZ);
		const __m128 b = _mm_and_ps(rows[1], maskXYZ);
		const __m128 c = _mm_and_ps(rows[2], maskXYZ);
		const __m128 d = _mm_and_ps(rows[3], maskXYZ);

		// Shuffles (y,z,x) partagés entre plusieurs produits vectoriels
		const __m128 a_yzx = _mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 0, 2, 1));
		const __m128 b_yzx = _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 0, 2, 1));
		const __m128 c_yzx = _mm_shuffle_ps(c, c, _MM_SHUFFLE(3, 0, 2, 1));
		const __m128 d_yzx = _mm_shuffle_ps(d, d, _MM_SHUFFLE(3, 0, 2, 1));

		// s = a x b
		__m128 s = _mm_sub_ps(_mm_mul_ps(a, b_yzx), _mm_mul_ps(a_yzx, b));
		s = _mm_shuffle_ps(s, s, _MM_SHUFFLE(3, 0, 2, 1));

		// t = c x d
		__m128 t = _mm_sub_ps(_mm_mul_ps(c, d_yzx), _mm_mul_ps(c_yzx, d));
		t = _mm_shuffle_ps(t, t, _MM_SHUFFLE(3, 0, 2, 1));

		__m128 u = _mm_sub_ps(_mm_mul_ps(a, y), _mm_mul_ps(b, x));
		__m128 v = _mm_sub_ps(_mm_mul_ps(c, w), _mm_mul_ps(d, z));

		// det = s.v + t.u (somme horizontale diffusée dans les 4 lanes)
		__m128 det = _mm_add_ps(_mm_mul_ps(s, v), _mm_mul_ps(t, u));
		det = _mm_add_ps(det, _mm_shuffle_ps(det, det, _MM_SHUFFLE(1, 0, 3, 2)));
		det = _mm_add_ps(det, _mm_shuffle_ps(det, det, _MM_SHUFFLE(2, 3, 0, 1)));

		if (_mm_movemask_ps(_mm_cmpeq_ps(det, _mm_setzero_ps())) != 0)
		{
			throw std::domain_error("Cannot invert a matrix with a zero determinant");
		}

		const __m128 invDet = _mm_div_ps(_mm_set1_ps(1.0f), det);
		s = _mm_mul_ps(s, invDet);
		t = _mm_mul_ps(t, invDet);
		u = _mm_mul_ps(u, invDet);
		v = _mm_mul_ps(v, invDet);

		const __m128 u_yzx = _mm_shuffle_ps(u, u, _MM_SHUFFLE(3, 0, 2, 1));
		const __m128 v_yzx = _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 0, 2, 1));

		// r0 = b x v + t*y
		// r1 = v x a - t*x
		// r2 = d x u + s*w
		// r3 = u x c - s*z
		__m128 r0 = _mm_sub_ps(_mm_mul_ps(b, v_yzx), _mm_mul_ps(b_yzx, v));
		__m128 r1 = _mm_sub_ps(_mm_mul_ps(v, a_yzx), _mm_mul_ps(v_yzx, a));
		__m128 r2 = _mm_sub_ps(_mm_mul_ps(d, u_yzx), _mm_mul_ps(d_yzx, u));
		__m128 r3 = _mm_sub_ps(_mm_mul_ps(u, c_yzx), _mm_mul_ps(u_yzx, c));

		r0 = _mm_shuffle_ps(r0, r0, _MM_SHUFFLE(3, 0, 2, 1));
		r1 = _mm_shuffle_ps(r1, r1, _MM_SHUFFLE(3, 0, 2, 1));
		r2 = _mm_shuffle_ps(r2, r2, _MM_SHUFFLE(3, 0, 2, 1));
		r3 = _mm_shuffle_ps(r3, r3, _MM_SHUFFLE(3, 0, 2, 1));

		r0 = _mm_add_ps(r0, _mm_mul_ps(t, y));
		r1 = _mm_sub_ps(r1, _mm_mul_ps(t, x));
		r2 = _mm_add_ps(r2, _mm_mul_ps(s, w));
		r3 = _mm_sub_ps(r3, _mm_mul_ps(s, z));

		// Dernière ligne de l'inverse : [-b.t, a.t, -d.s, c.s]
		const __m128 sums = _mm_hadd_ps(
			_mm_hadd_ps(_mm_mul_ps(b, t), _mm_mul_ps(a, t)),
			_mm_hadd_ps(_mm_mul_ps(d, s), _mm_mul_ps(c, s)));

		const __m128 lastRow = _mm_xor_ps(sums, _mm_set_ps(0.0f, -0.0f, 0.0f, -0.0f));

		// Transposition finale
		const __m128 tmp0 = _mm_unpacklo_ps(r0, r1);
		const __m128 tmp1 = _mm_unpackhi_ps(r0, r1);
		const __m128 tmp2 = _mm_unpacklo_ps(r2, r3);
		const __m128 tmp3 = _mm_unpackhi_ps(r2, r3);

		Matrix4x4_SSE result;
		result.rows[0] = _mm_movelh_ps(tmp0, tmp2);
		result.rows[1] = _mm_movehl_ps(tmp2, tmp0);
		result.rows[2] = _mm_movelh_ps(tmp1, tmp3);
		result.rows[3] = lastRow;

		return result;
	}



	inline Matrix4x4_SSE Matrix4x4_SSE::Scale(const Vec3_SSE& scale)
	{
		Matrix4x4_SSE result;

		// On copie une lane de scale.reg vers la même position, et on met les autres à 0
		result.rows[0] = _mm_insert_ps(scale.reg, scale.reg, 0x0E); // (sx, 0, 0, 0)
		result.rows[1] = _mm_insert_ps(scale.reg, scale.reg, 0x5D); // (0, sy, 0, 0)
		result.rows[2] = _mm_insert_ps(scale.reg, scale.reg, 0xAB); // (0, 0, sz, 0)
		result.rows[3] = _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f);

		return result;
	}

	inline Matrix4x4_SSE Matrix4x4_SSE::RotationX(float radians)
	{
		const float c = std::cos(radians);
		const float s = std::sin(radians);

		Matrix4x4_SSE result;
		result.rows[0] = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
		result.rows[1] = _mm_setr_ps(0.0f, c, -s, 0.0f);
		result.rows[2] = _mm_setr_ps(0.0f, s, c, 0.0f);
		result.rows[3] = _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f);
		return result;
	}

	inline Matrix4x4_SSE Matrix4x4_SSE::RotationY(float radians)
	{
		const float c = std::cos(radians);
		const float s = std::sin(radians);

		Matrix4x4_SSE result;
		result.rows[0] = _mm_setr_ps(c, 0.0f, s, 0.0f);
		result.rows[1] = _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);
		result.rows[2] = _mm_setr_ps(-s, 0.0f, c, 0.0f);
		result.rows[3] = _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f);
		return result;
	}

	inline Matrix4x4_SSE Matrix4x4_SSE::RotationZ(float radians)
	{
		const float c = std::cos(radians);
		const float s = std::sin(radians);

		Matrix4x4_SSE result;
		result.rows[0] = _mm_setr_ps(c, -s, 0.0f, 0.0f);
		result.rows[1] = _mm_setr_ps(s, c, 0.0f, 0.0f);
		result.rows[2] = _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
		result.rows[3] = _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f);
		return result;
	}

	inline Matrix4x4_SSE Matrix4x4_SSE::Translation(const Vec3_SSE& offset)
	{
		Matrix4x4_SSE result;

		// On copie tx, ty, tz dans la lane w de chaque ligne de l'identité
		result.rows[0] = _mm_insert_ps(_mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f), offset.reg, 0x30); // lane x -> w
		result.rows[1] = _mm_insert_ps(_mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f), offset.reg, 0x70); // lane y -> w
		result.rows[2] = _mm_insert_ps(_mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f), offset.reg, 0xB0); // lane z -> w
		result.rows[3] = _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f);

		return result;
	}



	inline Vec3_SSE Matrix4x4_SSE::TransformPoint(const Vec3_SSE& point) const
	{
		// On force w = 1 : la translation (colonne 3) est ajoutée par le produit scalaire
		const __m128 p = _mm_blend_ps(point.reg, _mm_set1_ps(1.0f), 0x8);

		const __m128 rx = _mm_dp_ps(rows[0], p, 0xF1);
		const __m128 ry = _mm_dp_ps(rows[1], p, 0xF2);
		const __m128 rz = _mm_dp_ps(rows[2], p, 0xF4);

		return Vec3_SSE(_mm_or_ps(_mm_or_ps(rx, ry), rz));
	}

	inline Vec3_SSE Matrix4x4_SSE::TransformDirection(const Vec3_SSE& direction) const
	{
		// On force w = 0 : la translation est ignorée
		const __m128 d = _mm_blend_ps(direction.reg, _mm_setzero_ps(), 0x8);

		const __m128 rx = _mm_dp_ps(rows[0], d, 0xF1);
		const __m128 ry = _mm_dp_ps(rows[1], d, 0xF2);
		const __m128 rz = _mm_dp_ps(rows[2], d, 0xF4);

		return Vec3_SSE(_mm_or_ps(_mm_or_ps(rx, ry), rz));
	}


	inline Matrix4x4_SSE Matrix4x4_SSE::Perspective(float fovYRadians, float aspect,
		float zNear, float zFar)
	{
		assert(aspect > 0.0f && zNear > 0.0f && zFar > zNear);

		const float yScale = 1.0f / std::tan(fovYRadians * 0.5f);
		const float xScale = yScale / aspect;
		const float inv = 1.0f / (zNear - zFar);
		const float zMapA = (zFar + zNear) * inv;
		const float zMapB = 2.0f * zFar * zNear * inv;

		Matrix4x4_SSE result;
		result.rows[0] = _mm_setr_ps(xScale, 0.0f, 0.0f, 0.0f);
		result.rows[1] = _mm_setr_ps(0.0f, yScale, 0.0f, 0.0f);
		result.rows[2] = _mm_setr_ps(0.0f, 0.0f, zMapA, zMapB);
		result.rows[3] = _mm_setr_ps(0.0f, 0.0f, -1.0f, 0.0f);   // w = -z
		return result;
	}

	inline Matrix4x4_SSE Matrix4x4_SSE::LookAt(const Vec3_SSE& eye, const Vec3_SSE& target,
		const Vec3_SSE& up)
	{
		using namespace Detail;

		const __m128 forward = Normalize3(_mm_sub_ps(eye.reg, target.reg));

		__m128 right = Cross3(up.reg, forward);

		// Cas limite : up parallèle à forward -> cross nul -> NaN après normalisation.
		// On retombe sur l'axe X (valable tant que forward n'est pas lui aussi sur X).
		if (_mm_cvtss_f32(Dot3(right, right)) < 1e-12f)
			right = Cross3(_mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f), forward);

		right = Normalize3(right);
		const __m128 camUp = Cross3(forward, right);   // déjà unitaire (right ⟂ forward)

		// Lane w de chaque ligne = -dot(axe, eye), broadcast puis blend dans la lane 3
		const __m128 zero = _mm_setzero_ps();
		const __m128 tx = _mm_sub_ps(zero, Dot3(right, eye.reg));
		const __m128 ty = _mm_sub_ps(zero, Dot3(camUp, eye.reg));
		const __m128 tz = _mm_sub_ps(zero, Dot3(forward, eye.reg));

		Matrix4x4_SSE result;
		result.rows[0] = _mm_blend_ps(right, tx, 0x8);
		result.rows[1] = _mm_blend_ps(camUp, ty, 0x8);
		result.rows[2] = _mm_blend_ps(forward, tz, 0x8);
		result.rows[3] = _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f);
		return result;
	}
}