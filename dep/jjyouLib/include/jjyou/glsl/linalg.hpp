/***********************************************************************
 * @file	linalg.hpp
 * @author	jjyou
 * @date	2024-2-3
 * @brief	This file implements linear algebra functions.
***********************************************************************/

#ifndef jjyou_glsl_linalg_hpp
#define jjyou_glsl_linalg_hpp

#include <cmath>

namespace jjyou {

	namespace glsl {

		template <class T, int Cols, int Rows> inline constexpr mat<T, Cols, Rows> zeros(void) {
			return mat<T, Cols, Rows>();
		}

		template <class T, int Length> inline constexpr vec<T, Length> zeros(void) {
			return vec<T, Length>();
		}

		template <class T, int Cols, int Rows> inline constexpr mat<T, Cols, Rows> ones(void) {
			mat<T, Cols, Rows> res;
			res += static_cast<T>(1.0);
			return res;
		}

		template <class T, int Length> inline constexpr vec<T, Length> ones(void) {
			return vec<T, Length>(static_cast<T>(1.0));
		}

		template <class T, int Length> inline constexpr vec<T, Length> max(const vec<T, Length>& v1, const vec<T, Length>& v2) {
			vec<T, Length> res;
			for (int i = 0; i < Length; ++i)
				res[i] = std::max(v1[i], v2[i]);
			return res;
		}

		template <class T, int Cols, int Rows> inline constexpr mat<T, Cols, Rows> max(const mat<T, Cols, Rows>& m1, const mat<T, Cols, Rows>& m2) {
			mat<T, Cols, Rows> res;
			for (int c = 0; c < Cols; ++c)
				for (int r = 0; r < Rows; ++r)
					res[c][r] = std::max(m1[c][r], m2[c][r]);
			return res;
		}

		template <class T, int Length> inline constexpr vec<T, Length> min(const vec<T, Length>& v1, const vec<T, Length>& v2) {
			vec<T, Length> res;
			for (int i = 0; i < Length; ++i)
				res[i] = std::min(v1[i], v2[i]);
			return res;
		}

		template <class T, int Cols, int Rows> inline constexpr mat<T, Cols, Rows> min(const mat<T, Cols, Rows>& m1, const mat<T, Cols, Rows>& m2) {
			mat<T, Cols, Rows> res;
			for (int c = 0; c < Cols; ++c)
				for (int r = 0; r < Rows; ++r)
					res[c][r] = std::min(m1[c][r], m2[c][r]);
			return res;
		}

		template <class T, int Cols, int Rows> inline constexpr mat<T, Cols, Rows> eye(void) {
			return mat<T, Cols, Rows>(static_cast<T>(1.0));
		}

		template <class T, int Cols> inline constexpr mat<T, Cols, Cols> identity(void) {
			return mat<T, Cols, Cols>(static_cast<T>(1.0));
		}

		template <class T, int Length> inline T dot(const vec<T, Length>& v1, const vec<T, Length>& v2) {
			T res{};
			for (int i = 0; i < Length; ++i)
				res += v1[i] * v2[i];
			return res;
		}

		template <class T, int Length> inline constexpr T squaredNorm(const vec<T, Length>& v) {
			return dot(v, v);
		}

		template <class T> inline constexpr T squaredNorm(const qua<T>& q) {
			return q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
		}

		template <class T, int Length> inline constexpr T norm(const vec<T, Length>& v) {
			return std::sqrt(squaredNorm(v));
		}

		template <class T> inline constexpr T norm(const qua<T>& q) {
			return std::sqrt(squaredNorm(q));
		}

		template <class T, int Length> inline void normalize(vec<T, Length>& v) {
			v /= norm(v);
			return;
		}

		template <class T, int Length> inline vec<T, Length> normalized(const vec<T, Length>& v) {
			return v / norm(v);
		}

		template <class T, int Cols, int Rows> inline T trace(const mat<T, Cols, Rows>& m) {
			T res{};
			constexpr int minDim = std::min(Cols, Rows);
			for (int i = 0; i < minDim; ++i)
				res += m[i][i];
			return res;
		}

		template <class T, int Cols, int Rows> inline mat<T, Rows, Cols> transpose(const mat<T, Cols, Rows>& m) {
			mat<T, Rows, Cols> res;
			for (int c = 0; c < Cols; ++c)
				for (int r = 0; r < Rows; ++r)
					res[r][c] = m[c][r];
			return res;
		}

		template <class T, int Cols, int Rows> inline mat<T, Cols, Rows> outer(const vec<T, Rows>& v1, const vec<T, Cols>& v2) {
			mat<T, Cols, Rows> res;
			for (int c = 0; c < Cols; ++c)
				for (int r = 0; r < Rows; ++r)
					res[c][r] = v1[r] * v2[c];
			return res;
		}

		template <class T> inline vec<T, 3> cross(const vec<T, 3>& v1, const vec<T, 3>& v2) {
			return vec<T, 3>(
				v1.y * v2.z - v1.z * v2.y,
				v1.z * v2.x - v1.x * v2.z,
				v1.x * v2.y - v1.y * v2.x
			);
		}

		template <class T> inline mat<T, 3, 3> cross(const vec<T, 3>& v) {
			return mat<T, 3, 3>(
				static_cast<T>(0.0), v.z, -v.y,
				-v.z, static_cast<T>(0.0), v.x,
				v.y, -v.x, static_cast<T>(0.0)
			);
		}

		template <class T> T determinant(const mat<T, 2, 2>& m) {
			return m[0][0] * m[1][1] - m[1][0] * m[0][1];
		}

		template <class T> T determinant(const mat<T, 3, 3>& m) {
			return
				+ m[0][0] * (m[1][1] * m[2][2] - m[2][1] * m[1][2])
				- m[1][0] * (m[0][1] * m[2][2] - m[2][1] * m[0][2])
				+ m[2][0] * (m[0][1] * m[1][2] - m[1][1] * m[0][2]);
		}

		template <class T> T determinant(const mat<T, 4, 4>& m) {
			return
				+ m[0][3] * m[1][2] * m[2][1] * m[3][0]
				- m[0][2] * m[1][3] * m[2][1] * m[3][0]
				- m[0][3] * m[1][1] * m[2][2] * m[3][0]
				+ m[0][1] * m[1][3] * m[2][2] * m[3][0]
				+ m[0][2] * m[1][1] * m[2][3] * m[3][0]
				- m[0][1] * m[1][2] * m[2][3] * m[3][0]
				- m[0][3] * m[1][2] * m[2][0] * m[3][1]
				+ m[0][2] * m[1][3] * m[2][0] * m[3][1]
				+ m[0][3] * m[1][0] * m[2][2] * m[3][1]
				- m[0][0] * m[1][3] * m[2][2] * m[3][1]
				- m[0][2] * m[1][0] * m[2][3] * m[3][1]
				+ m[0][0] * m[1][2] * m[2][3] * m[3][1]
				+ m[0][3] * m[1][1] * m[2][0] * m[3][2]
				- m[0][1] * m[1][3] * m[2][0] * m[3][2]
				- m[0][3] * m[1][0] * m[2][1] * m[3][2]
				+ m[0][0] * m[1][3] * m[2][1] * m[3][2]
				+ m[0][1] * m[1][0] * m[2][3] * m[3][2]
				- m[0][0] * m[1][1] * m[2][3] * m[3][2]
				- m[0][2] * m[1][1] * m[2][0] * m[3][3]
				+ m[0][1] * m[1][2] * m[2][0] * m[3][3]
				+ m[0][2] * m[1][0] * m[2][1] * m[3][3]
				- m[0][0] * m[1][2] * m[2][1] * m[3][3]
				- m[0][1] * m[1][0] * m[2][2] * m[3][3]
				+ m[0][0] * m[1][1] * m[2][2] * m[3][3];
		}

		template <class T> mat<T, 2, 2> inverse(const mat<T, 2, 2>& m) {
			mat<T, 2, 2> res(
				+m[1][1],
				-m[0][1],
				-m[1][0],
				+m[0][0]
			);
			res /= determinant(m);
			return res;
		}

		template <class T> mat<T, 3, 3> inverse(const mat<T, 3, 3>& m) {
			mat<T, 3, 3> res;
			res[0][0] = +(m[1][1] * m[2][2] - m[2][1] * m[1][2]);
			res[1][0] = -(m[1][0] * m[2][2] - m[2][0] * m[1][2]);
			res[2][0] = +(m[1][0] * m[2][1] - m[2][0] * m[1][1]);
			res[0][1] = -(m[0][1] * m[2][2] - m[2][1] * m[0][2]);
			res[1][1] = +(m[0][0] * m[2][2] - m[2][0] * m[0][2]);
			res[2][1] = -(m[0][0] * m[2][1] - m[2][0] * m[0][1]);
			res[0][2] = +(m[0][1] * m[1][2] - m[1][1] * m[0][2]);
			res[1][2] = -(m[0][0] * m[1][2] - m[1][0] * m[0][2]);
			res[2][2] = +(m[0][0] * m[1][1] - m[1][0] * m[0][1]);
			res /= determinant(m);
			return res;
		}

		template <class T> mat<T, 4, 4> inverse(const mat<T, 4, 4>& m) {
			mat<T, 4, 4> res;
			res[0][0] = m[1][2] * m[2][3] * m[3][1] - m[1][3] * m[2][2] * m[3][1] +
				m[1][3] * m[2][1] * m[3][2] - m[1][1] * m[2][3] * m[3][2] -
				m[1][2] * m[2][1] * m[3][3] + m[1][1] * m[2][2] * m[3][3];
			res[0][1] = m[0][3] * m[2][2] * m[3][1] - m[0][2] * m[2][3] * m[3][1] -
				m[0][3] * m[2][1] * m[3][2] + m[0][1] * m[2][3] * m[3][2] +
				m[0][2] * m[2][1] * m[3][3] - m[0][1] * m[2][2] * m[3][3];
			res[0][2] = m[0][2] * m[1][3] * m[3][1] - m[0][3] * m[1][2] * m[3][1] +
				m[0][3] * m[1][1] * m[3][2] - m[0][1] * m[1][3] * m[3][2] -
				m[0][2] * m[1][1] * m[3][3] + m[0][1] * m[1][2] * m[3][3];
			res[0][3] = m[0][3] * m[1][2] * m[2][1] - m[0][2] * m[1][3] * m[2][1] -
				m[0][3] * m[1][1] * m[2][2] + m[0][1] * m[1][3] * m[2][2] +
				m[0][2] * m[1][1] * m[2][3] - m[0][1] * m[1][2] * m[2][3];
			res[1][0] = m[1][3] * m[2][2] * m[3][0] - m[1][2] * m[2][3] * m[3][0] -
				m[1][3] * m[2][0] * m[3][2] + m[1][0] * m[2][3] * m[3][2] +
				m[1][2] * m[2][0] * m[3][3] - m[1][0] * m[2][2] * m[3][3];
			res[1][1] = m[0][2] * m[2][3] * m[3][0] - m[0][3] * m[2][2] * m[3][0] +
				m[0][3] * m[2][0] * m[3][2] - m[0][0] * m[2][3] * m[3][2] -
				m[0][2] * m[2][0] * m[3][3] + m[0][0] * m[2][2] * m[3][3];
			res[1][2] = m[0][3] * m[1][2] * m[3][0] - m[0][2] * m[1][3] * m[3][0] -
				m[0][3] * m[1][0] * m[3][2] + m[0][0] * m[1][3] * m[3][2] +
				m[0][2] * m[1][0] * m[3][3] - m[0][0] * m[1][2] * m[3][3];
			res[1][3] = m[0][2] * m[1][3] * m[2][0] - m[0][3] * m[1][2] * m[2][0] +
				m[0][3] * m[1][0] * m[2][2] - m[0][0] * m[1][3] * m[2][2] -
				m[0][2] * m[1][0] * m[2][3] + m[0][0] * m[1][2] * m[2][3];
			res[2][0] = m[1][1] * m[2][3] * m[3][0] - m[1][3] * m[2][1] * m[3][0] +
				m[1][3] * m[2][0] * m[3][1] - m[1][0] * m[2][3] * m[3][1] -
				m[1][1] * m[2][0] * m[3][3] + m[1][0] * m[2][1] * m[3][3];
			res[2][1] = m[0][3] * m[2][1] * m[3][0] - m[0][1] * m[2][3] * m[3][0] -
				m[0][3] * m[2][0] * m[3][1] + m[0][0] * m[2][3] * m[3][1] +
				m[0][1] * m[2][0] * m[3][3] - m[0][0] * m[2][1] * m[3][3];
			res[2][2] = m[0][1] * m[1][3] * m[3][0] - m[0][3] * m[1][1] * m[3][0] +
				m[0][3] * m[1][0] * m[3][1] - m[0][0] * m[1][3] * m[3][1] -
				m[0][1] * m[1][0] * m[3][3] + m[0][0] * m[1][1] * m[3][3];
			res[2][3] = m[0][3] * m[1][1] * m[2][0] - m[0][1] * m[1][3] * m[2][0] -
				m[0][3] * m[1][0] * m[2][1] + m[0][0] * m[1][3] * m[2][1] +
				m[0][1] * m[1][0] * m[2][3] - m[0][0] * m[1][1] * m[2][3];
			res[3][0] = m[1][2] * m[2][1] * m[3][0] - m[1][1] * m[2][2] * m[3][0] -
				m[1][2] * m[2][0] * m[3][1] + m[1][0] * m[2][2] * m[3][1] +
				m[1][1] * m[2][0] * m[3][2] - m[1][0] * m[2][1] * m[3][2];
			res[3][1] = m[0][1] * m[2][2] * m[3][0] - m[0][2] * m[2][1] * m[3][0] +
				m[0][2] * m[2][0] * m[3][1] - m[0][0] * m[2][2] * m[3][1] -
				m[0][1] * m[2][0] * m[3][2] + m[0][0] * m[2][1] * m[3][2];
			res[3][2] = m[0][2] * m[1][1] * m[3][0] - m[0][1] * m[1][2] * m[3][0] -
				m[0][2] * m[1][0] * m[3][1] + m[0][0] * m[1][2] * m[3][1] +
				m[0][1] * m[1][0] * m[3][2] - m[0][0] * m[1][1] * m[3][2];
			res[3][3] = m[0][1] * m[1][2] * m[2][0] - m[0][2] * m[1][1] * m[2][0] +
				m[0][2] * m[1][0] * m[2][1] - m[0][0] * m[1][2] * m[2][1] -
				m[0][1] * m[1][0] * m[2][2] + m[0][0] * m[1][1] * m[2][2];
			res /= determinant(m);
			return res;
		}

		template <class T, int Dim1, int Dim2, int Dim3> mat<T, Dim3, Dim1> operator*(const mat<T, Dim2, Dim1>& m1, const mat<T, Dim3, Dim2>& m2) {
			mat<T, Dim3, Dim1> res;
			for (int r = 0; r < Dim1; ++r)
				for (int c = 0; c < Dim3; ++c)
					for (int k = 0; k < Dim2; ++k)
						res[c][r] += m1[k][r] * m2[c][k];
			return res;
		}

		template <class T, int Dim1, int Dim2> vec<T, Dim1> operator*(const mat<T, Dim2, Dim1>& m, const vec<T, Dim2>& v) {
			vec<T, Dim1> res;
			for (int r = 0; r < Dim1; ++r)
				for (int k = 0; k < Dim2; ++k)
					res[r] += m[k][r] * v[k];
			return res;
		}

		template <class T, int Dim2, int Dim3> vec<T, Dim3> operator*(const vec<T, Dim2>& v, const mat<T, Dim3, Dim2>& m) {
			vec<T, Dim3> res;
			for (int c = 0; c < Dim3; ++c)
				//for (int k = 0; k < Dim2; ++k)
				//	res[c] += v[k] * m[c][k];
				res[c] = dot(v, m[c]);
			return res;
		}

	}

}

#endif /* jjyou_glsl_linalg_hpp */