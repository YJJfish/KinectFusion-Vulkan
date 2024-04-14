/***********************************************************************
 * @file	base.hpp
 * @author	jjyou
 * @date	2024-2-1
 * @brief	This file is the base file for jjyou::glsl library. It contains
 *			forward declarations of classes and functions.
***********************************************************************/

#ifndef jjyou_glsl_base_hpp
#define jjyou_glsl_base_hpp

namespace jjyou {

	namespace glsl {

		template <class T, int Length>
		class vec;

		template <class T, int Cols, int Rows>
		class mat;

		template <class T>
		class qua;

		template <class T> inline constexpr T radians(const T& v);

		template <class T> inline constexpr T degrees(const T& v);

		template <class T, int Cols, int Rows> inline constexpr mat<T, Cols, Rows> zeros(void);

		template <class T, int Length> inline constexpr vec<T, Length> zeros(void);

		template <class T, int Cols, int Rows> inline constexpr mat<T, Cols, Rows> ones(void);

		template <class T, int Length> inline constexpr vec<T, Length> ones(void);

		template <class T, int Length> inline constexpr vec<T, Length> max(const vec<T, Length>& v1, const vec<T, Length>& v2);

		template <class T, int Cols, int Rows> inline constexpr mat<T, Cols, Rows> max(const mat<T, Cols, Rows>& m1, const mat<T, Cols, Rows>& m2);

		template <class T, int Length> inline constexpr vec<T, Length> min(const vec<T, Length>& v1, const vec<T, Length>& v2);

		template <class T, int Cols, int Rows> inline constexpr mat<T, Cols, Rows> min(const mat<T, Cols, Rows>& m1, const mat<T, Cols, Rows>& m2);

		template <class T, int Cols, int Rows> inline constexpr mat<T, Cols, Rows> eye(void);

		template <class T, int Cols> inline constexpr mat<T, Cols, Cols> identity(void);

		template <class T, int Length> inline T dot(const vec<T, Length>& v1, const vec<T, Length>& v2);

		template <class T, int Length> inline constexpr T squaredNorm(const vec<T, Length>& v);

		template <class T> inline constexpr T squaredNorm(const qua<T>& q);

		template <class T, int Length> inline constexpr T norm(const vec<T, Length>& v);

		template <class T> inline constexpr T norm(const qua<T>& q);

		template <class T, int Length> inline void normalize(vec<T, Length>& v);

		template <class T, int Length> inline vec<T, Length> normalized(const vec<T, Length>& v);

		template <class T, int Cols, int Rows> inline T trace(const mat<T, Cols, Rows>& m);

		template <class T, int Cols, int Rows> inline mat<T, Rows, Cols> transpose(const mat<T, Cols, Rows>& m);

		template <class T, int Cols, int Rows> inline mat<T, Cols, Rows> outer(const vec<T, Rows>& v1, const vec<T, Cols>& v2);

		template <class T> inline vec<T, 3> cross(const vec<T, 3>& v1, const vec<T, 3>& v2);

		template <class T> inline mat<T, 3, 3> cross(const vec<T, 3>& v);

		template <class T, int Cols> T determinant(const mat<T, Cols, Cols>& m);

		template <class T, int Cols> mat<T, Cols, Cols> inverse(const mat<T, Cols, Cols>& m);

		template <class T, int Dim1, int Dim2, int Dim3> mat<T, Dim3, Dim1> operator*(const mat<T, Dim2, Dim1>& m1, const mat<T, Dim3, Dim2>& m2);

		template <class T, int Dim1, int Dim2> vec<T, Dim1> operator*(const mat<T, Dim2, Dim1>& m, const vec<T, Dim2>& v);

		template <class T, int Dim2, int Dim3> vec<T, Dim3> operator*(const vec<T, Dim2>& v, const mat<T, Dim3, Dim2>& m);

	}

}

#endif /* jjyou_glsl_base_hpp */