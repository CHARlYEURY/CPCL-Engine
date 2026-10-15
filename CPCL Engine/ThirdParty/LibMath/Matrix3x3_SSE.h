#pragma once
#include "Vec3_SSE.h"
#include <xmmintrin.h>
#include <stdexcept>
#include <limits>
#include <array>
#include <cmath>


namespace Maths
{
	class Matrix3x3_SSE {
	public:

		__m128 rows[3];

		Matrix3x3_SSE();
		Matrix3x3_SSE(const std::array<float, 9>& elements); 
		
		static Matrix3x3_SSE Identity();
		static Matrix3x3_SSE Zero();

		float& operator()(std::size_t row, std::size_t column);
		const float& operator()(std::size_t row, std::size_t column) const;
		Matrix3x3_SSE operator+(const Matrix3x3_SSE& rhs) const;
		Matrix3x3_SSE operator-(const Matrix3x3_SSE& rhs) const;
		Matrix3x3_SSE operator*(const Matrix3x3_SSE& rhs) const;
		Vec3_SSE operator*(const Vec3_SSE& rhs) const;
		template <typename T>
		Matrix3x3_SSE operator*(T scalar) const;
		Matrix3x3_SSE& operator*=(const Matrix3x3_SSE& rhs);
		bool operator==(const Matrix3x3_SSE& rhs) const;
		bool operator!=(const Matrix3x3_SSE& rhs) const;

		Matrix3x3_SSE Transpose() const;
		float Determinant() const;
		Matrix3x3_SSE Inverse(float relativeTolerance) const;
		static Matrix3x3_SSE Scale(const Vec3_SSE& scale);

		template <typename T>
		static Matrix3x3_SSE RotationX(T radians);
		template <typename T>
		static Matrix3x3_SSE RotationY(T radians);
		template <typename T>
		static Matrix3x3_SSE RotationZ(T radians);
	};

	inline Matrix3x3_SSE::Matrix3x3_SSE() 
	{
		rows[0] = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
		rows[1] = _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);
		rows[2] = _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
	}

	inline Matrix3x3_SSE::Matrix3x3_SSE(const std::array<float, 9>& elements) 
	{
		rows[0] = _mm_setr_ps(elements[0], elements[1], elements[2], 0.0f);
		rows[1] = _mm_setr_ps(elements[3], elements[4], elements[5], 0.0f);
		rows[2] = _mm_setr_ps(elements[6], elements[7], elements[8], 0.0f);
	}

	inline Matrix3x3_SSE Matrix3x3_SSE::Identity()
	{
		return Matrix3x3_SSE();
	}

	inline Matrix3x3_SSE Matrix3x3_SSE::Zero()
	{
		Matrix3x3_SSE result;
		result.rows[0] = _mm_setzero_ps();
		result.rows[1] = _mm_setzero_ps();
		result.rows[2] = _mm_setzero_ps();
		return result;
	}
	
	inline float& Matrix3x3_SSE::operator()(std::size_t row, std::size_t column)
	{
		if (row >= 3 || column >= 3)
		{
			throw std::out_of_range("Matrix index is out of range");
		}
		return rows[row].m128_f32[column];
	}

	inline const float& Matrix3x3_SSE::operator()(std::size_t row, std::size_t column) const
	{
		if (row >= 3 || column >= 3)
		{
			throw std::out_of_range("Matrix index is out of range");
		}
		return rows[row].m128_f32[column];
	}

	inline Matrix3x3_SSE Matrix3x3_SSE::operator+(const Matrix3x3_SSE& rhs) const {
		Matrix3x3_SSE result;
		result.rows[0] = _mm_add_ps(rows[0], rhs.rows[0]);
		result.rows[1] = _mm_add_ps(rows[1], rhs.rows[1]);
		result.rows[2] = _mm_add_ps(rows[2], rhs.rows[2]);
		return result;
	}

	inline Matrix3x3_SSE Matrix3x3_SSE::operator-(const Matrix3x3_SSE& rhs) const {
		Matrix3x3_SSE result;
		result.rows[0] = _mm_sub_ps(rows[0], rhs.rows[0]);
		result.rows[1] = _mm_sub_ps(rows[1], rhs.rows[1]);
		result.rows[2] = _mm_sub_ps(rows[2], rhs.rows[2]);
		return result;
	}

	inline Matrix3x3_SSE Matrix3x3_SSE::operator*(const Matrix3x3_SSE& rhs) const {
		Matrix3x3_SSE result;

		// --- Ligne 0 ---
		__m128 R0_x = _mm_shuffle_ps(rows[0], rows[0], _MM_SHUFFLE(0, 0, 0, 0));
		__m128 R0_y = _mm_shuffle_ps(rows[0], rows[0], _MM_SHUFFLE(1, 1, 1, 1));
		__m128 R0_z = _mm_shuffle_ps(rows[0], rows[0], _MM_SHUFFLE(2, 2, 2, 2));

		__m128 R0_mX = _mm_mul_ps(R0_x, rhs.rows[0]);
		__m128 R0_my = _mm_mul_ps(R0_y, rhs.rows[1]);
		__m128 R0_mz = _mm_mul_ps(R0_z, rhs.rows[2]);

		result.rows[0] = _mm_add_ps(_mm_add_ps(R0_mX, R0_my), R0_mz);

		// --- Ligne 1 ---
		__m128 R1_x = _mm_shuffle_ps(rows[1], rows[1], _MM_SHUFFLE(0, 0, 0, 0));
		__m128 R1_y = _mm_shuffle_ps(rows[1], rows[1], _MM_SHUFFLE(1, 1, 1, 1));
		__m128 R1_z = _mm_shuffle_ps(rows[1], rows[1], _MM_SHUFFLE(2, 2, 2, 2));

		__m128 R1_mx = _mm_mul_ps(R1_x, rhs.rows[0]);
		__m128 R1_my = _mm_mul_ps(R1_y, rhs.rows[1]);
		__m128 R1_mz = _mm_mul_ps(R1_z, rhs.rows[2]);

		result.rows[1] = _mm_add_ps(_mm_add_ps(R1_mx, R1_my), R1_mz);

		// --- Ligne 2 ---
		__m128 R2_x = _mm_shuffle_ps(rows[2], rows[2], _MM_SHUFFLE(0, 0, 0, 0));
		__m128 R2_y = _mm_shuffle_ps(rows[2], rows[2], _MM_SHUFFLE(1, 1, 1, 1));
		__m128 R2_z = _mm_shuffle_ps(rows[2], rows[2], _MM_SHUFFLE(2, 2, 2, 2));

		__m128 R2_mx = _mm_mul_ps(R2_x, rhs.rows[0]);
		__m128 R2_my = _mm_mul_ps(R2_y, rhs.rows[1]);
		__m128 R2_mz = _mm_mul_ps(R2_z, rhs.rows[2]);

		result.rows[2] = _mm_add_ps(_mm_add_ps(R2_mx, R2_my), R2_mz);

		return result;
	}

	inline Vec3_SSE Matrix3x3_SSE::operator*(const Vec3_SSE& rhs) const
	{
		__m128 Rhs_x = _mm_shuffle_ps(rhs.reg, rhs.reg, _MM_SHUFFLE(0, 0, 0, 0));
		__m128 Rhs_y = _mm_shuffle_ps(rhs.reg, rhs.reg, _MM_SHUFFLE(1, 1, 1, 1));
		__m128 rhs_z = _mm_shuffle_ps(rhs.reg, rhs.reg, _MM_SHUFFLE(2, 2, 2, 2));

		// 2. Si tu transposes mentalement le calcul :
		__m128 col0 = _mm_setr_ps(rows[0].m128_f32[0], rows[1].m128_f32[0], rows[2].m128_f32[0], 0.0f);
		__m128 col1 = _mm_setr_ps(rows[0].m128_f32[1], rows[1].m128_f32[1], rows[2].m128_f32[1], 0.0f);
		__m128 col2 = _mm_setr_ps(rows[0].m128_f32[2], rows[1].m128_f32[2], rows[2].m128_f32[2], 0.0f);

		// 3. Produit scalaire combiné (3 multiplications + 2 additions vectorielles)
		__m128 res = _mm_add_ps(
			_mm_add_ps(_mm_mul_ps(Rhs_x, col0), _mm_mul_ps(Rhs_y, col1)),
			_mm_mul_ps(rhs_z, col2)
		);

		return Vec3_SSE(res);
	}

	template <typename T>
	inline Matrix3x3_SSE Matrix3x3_SSE::operator*(T scalar) const
	{
		Matrix3x3_SSE result;
		__m128 scalar128 = _mm_set1_ps(scalar);
		result.rows[0] = _mm_mul_ps(rows[0], scalar128);
		result.rows[1] = _mm_mul_ps(rows[1], scalar128);
		result.rows[2] = _mm_mul_ps(rows[2], scalar128);
		return result;
	}

	inline Matrix3x3_SSE& Matrix3x3_SSE::operator*=(const Matrix3x3_SSE& rhs)
	{
		*this = *this * rhs;
		return *this;
	}

	inline bool Matrix3x3_SSE::operator==(const Matrix3x3_SSE& rhs) const
	{
		bool L0 = (_mm_movemask_ps(_mm_cmpeq_ps(rows[0], rhs.rows[0])) & 0x7) == 0x7;
		bool L1 = (_mm_movemask_ps(_mm_cmpeq_ps(rows[1], rhs.rows[1])) & 0x7) == 0x7;
		bool L2 = (_mm_movemask_ps(_mm_cmpeq_ps(rows[2], rhs.rows[2])) & 0x7) == 0x7;
		
		return L0 && L1 && L2;
	}

	inline bool Matrix3x3_SSE::operator!=(const Matrix3x3_SSE& rhs) const
	{
		bool L0 = (_mm_movemask_ps(_mm_cmpeq_ps(rows[0], rhs.rows[0])) & 0x7) != 0x7;
		bool L1 = (_mm_movemask_ps(_mm_cmpeq_ps(rows[1], rhs.rows[1])) & 0x7) != 0x7;
		bool L2 = (_mm_movemask_ps(_mm_cmpeq_ps(rows[2], rhs.rows[2])) & 0x7) != 0x7;
		
		return L0 || L1 || L2;
	}

	inline Matrix3x3_SSE Matrix3x3_SSE::Transpose() const
	{
		Matrix3x3_SSE result;
		result.rows[0] = _mm_setr_ps(rows[0].m128_f32[0], rows[1].m128_f32[0], rows[2].m128_f32[0], 0.0f);
		result.rows[1] = _mm_setr_ps(rows[0].m128_f32[1], rows[1].m128_f32[1], rows[2].m128_f32[1], 0.0f);
		result.rows[2] = _mm_setr_ps(rows[0].m128_f32[2], rows[1].m128_f32[2], rows[2].m128_f32[2], 0.0f);
		return result;
	}

	inline float Matrix3x3_SSE::Determinant() const
	{
		// 1. Préparation des paires pour les sous-déterminants 2x2
		__m128 vA = _mm_shuffle_ps(rows[1], rows[1], _MM_SHUFFLE(3, 0, 0, 1));
		__m128 vB = _mm_shuffle_ps(rows[2], rows[2], _MM_SHUFFLE(3, 1, 2, 2));

		__m128 vC = _mm_shuffle_ps(rows[1], rows[1], _MM_SHUFFLE(3, 1, 2, 2));
		__m128 vD = _mm_shuffle_ps(rows[2], rows[2], _MM_SHUFFLE(3, 0, 0, 1));

		// 2. Calcul des cofacteurs : (vA * vB) - (vC * vD)
		// cofacts = [ (m11*m22 - m12*m21), (m10*m22 - m12*m20), (m10*m21 - m11*m20), 0 ]
		__m128 cofacts = _mm_sub_ps(_mm_mul_ps(vA, vB), _mm_mul_ps(vC, vD));

		// 3. Application de l'alternance des signes (+, -, +)
		// signMask = [ 1.0f, -1.0f, 1.0f, 0.0f ]
		alignas(16) static const float signs[4] = { 1.0f, -1.0f, 1.0f, 0.0f };
		__m128 vSigns = _mm_load_ps(signs);

		__m128 r0_signed = _mm_mul_ps(rows[0], vSigns);

		// 4. Produit terme à terme entre la ligne 0 signée et les cofacteurs
		__m128 terms = _mm_mul_ps(r0_signed, cofacts); // [t0, t1, t2, 0]

		// 5. Somme horizontale des 3 termes (t0 + t1 + t2)
		__m128 shuf1 = _mm_shuffle_ps(terms, terms, _MM_SHUFFLE(1, 1, 1, 1));
		__m128 sum1 = _mm_add_ss(terms, shuf1); // t0 + t1

		__m128 shuf2 = _mm_shuffle_ps(terms, terms, _MM_SHUFFLE(2, 2, 2, 2));
		__m128 sum2 = _mm_add_ss(sum1, shuf2);  // (t0 + t1) + t2

		// Extraction de la valeur flottante finale
		return _mm_cvtss_f32(sum2);
	}

	inline Matrix3x3_SSE Matrix3x3_SSE::Inverse(float relativeTolerance) const
	{
		if (!std::isfinite(relativeTolerance) || relativeTolerance < 0.0f || relativeTolerance >= 1.0f)
		{
			throw std::invalid_argument("Inverse tolerance must be finite and in [0, 1)");
		}

		// 2. Calcul des cofacteurs pour chaque ligne
		__m128 r1_shuf1 = _mm_shuffle_ps(rows[1], rows[1], _MM_SHUFFLE(3, 0, 0, 1));
		__m128 r2_shuf1 = _mm_shuffle_ps(rows[2], rows[2], _MM_SHUFFLE(3, 1, 2, 2));
		__m128 r1_shuf2 = _mm_shuffle_ps(rows[1], rows[1], _MM_SHUFFLE(3, 1, 2, 2));
		__m128 r2_shuf2 = _mm_shuffle_ps(rows[2], rows[2], _MM_SHUFFLE(3, 0, 0, 1));
		__m128 cof0 = _mm_sub_ps(_mm_mul_ps(r1_shuf1, r2_shuf1), _mm_mul_ps(r1_shuf2, r2_shuf2));

		__m128 r2_shuf3 = _mm_shuffle_ps(rows[2], rows[2], _MM_SHUFFLE(3, 0, 0, 1));
		__m128 r0_shuf1 = _mm_shuffle_ps(rows[0], rows[0], _MM_SHUFFLE(3, 1, 2, 2));
		__m128 r2_shuf4 = _mm_shuffle_ps(rows[2], rows[2], _MM_SHUFFLE(3, 1, 2, 2));
		__m128 r0_shuf2 = _mm_shuffle_ps(rows[0], rows[0], _MM_SHUFFLE(3, 0, 0, 1));
		__m128 cof1 = _mm_sub_ps(_mm_mul_ps(r2_shuf3, r0_shuf1), _mm_mul_ps(r2_shuf4, r0_shuf2));

		__m128 r0_shuf3 = _mm_shuffle_ps(rows[0], rows[0], _MM_SHUFFLE(3, 0, 0, 1));
		__m128 r1_shuf3 = _mm_shuffle_ps(rows[1], rows[1], _MM_SHUFFLE(3, 1, 2, 2));
		__m128 r0_shuf4 = _mm_shuffle_ps(rows[0], rows[0], _MM_SHUFFLE(3, 1, 2, 2));
		__m128 r1_shuf4 = _mm_shuffle_ps(rows[1], rows[1], _MM_SHUFFLE(3, 0, 0, 1));
		__m128 cof2 = _mm_sub_ps(_mm_mul_ps(r0_shuf3, r1_shuf3), _mm_mul_ps(r0_shuf4, r1_shuf4));

		// 3. Application du masque de signes (+, -, +)
		alignas(16) static const float signs[4] = { 1.0f, -1.0f, 1.0f, 0.0f };
		__m128 vSigns = _mm_load_ps(signs);

		cof0 = _mm_mul_ps(cof0, vSigns);
		cof1 = _mm_mul_ps(cof1, vSigns);
		cof2 = _mm_mul_ps(cof2, vSigns);

		// 4. Calcul du déterminant via la Ligne 0
		__m128 det_terms = _mm_mul_ps(rows[0], cof0);
		__m128 shuf1 = _mm_shuffle_ps(det_terms, det_terms, _MM_SHUFFLE(1, 1, 1, 1));
		__m128 sum1 = _mm_add_ss(det_terms, shuf1);
		__m128 shuf2 = _mm_shuffle_ps(det_terms, det_terms, _MM_SHUFFLE(2, 2, 2, 2));
		__m128 det_vec = _mm_add_ss(sum1, shuf2);

		float det = _mm_cvtss_f32(det_vec);

		// 5. Vérification de la régularité de la matrice
		if (!std::isfinite(det) || std::abs(det) <= relativeTolerance)
		{
			throw std::domain_error("Matrix is singular, non-finite, or too ill-conditioned");
		}

		// 6. Transposition de la comatrice (cof0, cof1, cof2) pour obtenir les lignes de l'inverse
		__m128 inv_r0 = _mm_setr_ps(_mm_cvtss_f32(cof0), _mm_cvtss_f32(cof1), _mm_cvtss_f32(cof2), 0.0f);

		__m128 cof0_1 = _mm_shuffle_ps(cof0, cof0, _MM_SHUFFLE(1, 1, 1, 1));
		__m128 cof1_1 = _mm_shuffle_ps(cof1, cof1, _MM_SHUFFLE(1, 1, 1, 1));
		__m128 cof2_1 = _mm_shuffle_ps(cof2, cof2, _MM_SHUFFLE(1, 1, 1, 1));
		__m128 inv_r1 = _mm_setr_ps(_mm_cvtss_f32(cof0_1), _mm_cvtss_f32(cof1_1), _mm_cvtss_f32(cof2_1), 0.0f);

		__m128 cof0_2 = _mm_shuffle_ps(cof0, cof0, _MM_SHUFFLE(2, 2, 2, 2));
		__m128 cof1_2 = _mm_shuffle_ps(cof1, cof1, _MM_SHUFFLE(2, 2, 2, 2));
		__m128 cof2_2 = _mm_shuffle_ps(cof2, cof2, _MM_SHUFFLE(2, 2, 2, 2));
		__m128 inv_r2 = _mm_setr_ps(_mm_cvtss_f32(cof0_2), _mm_cvtss_f32(cof1_2), _mm_cvtss_f32(cof2_2), 0.0f);

		// 7. Division par le déterminant (1.0 / det)
		__m128 inv_det = _mm_set1_ps(1.0f / det);

		Matrix3x3_SSE result;
		result.rows[0] = _mm_mul_ps(inv_r0, inv_det);
		result.rows[1] = _mm_mul_ps(inv_r1, inv_det);
		result.rows[2] = _mm_mul_ps(inv_r2, inv_det);

		return result;
	}

	inline Matrix3x3_SSE Matrix3x3_SSE::Scale(const Vec3_SSE& scale)
	{
		Matrix3x3_SSE result;
		result.rows[0].m128_f32[0] = scale.reg.m128_f32[0];
		result.rows[1].m128_f32[1] = scale.reg.m128_f32[1];
		result.rows[2].m128_f32[2] = scale.reg.m128_f32[2];
		return result;
	}

	template <typename T>
	inline static Matrix3x3_SSE Matrix3x3_SSE::RotationX(T radians) 
	{
		Matrix3x3_SSE result;
		const T cosine = std::cos(radians);
		const T sine = std::sin(radians);
		result.rows[1].m128_f32[1] = cosine;
		result.rows[2].m128_f32[2] = cosine;
		result.rows[1].m128_f32[2] = -sine;
		result.rows[2].m128_f32[1] = sine;
		return result;
	}

	template <typename T>
	inline static Matrix3x3_SSE Matrix3x3_SSE::RotationY(T radians) 
	{
		Matrix3x3_SSE result;
		const T cosine = std::cos(radians);
		const T sine = std::sin(radians);
		result.rows[2].m128_f32[2] = cosine;
		result.rows[0].m128_f32[0] = cosine;
		result.rows[2].m128_f32[0] = -sine;
		result.rows[0].m128_f32[2] = sine;
		return result;
	}

	template <typename T>
	inline static Matrix3x3_SSE Matrix3x3_SSE::RotationZ(T radians) 
{
		Matrix3x3_SSE result;
		const T cosine = std::cos(radians);
		const T sine = std::sin(radians);
		result.rows[0].m128_f32[0] = cosine;
		result.rows[1].m128_f32[1] = cosine;
		result.rows[0].m128_f32[1] = -sine;
		result.rows[1].m128_f32[0] = sine;
		return result;
	}
}