/***********************************************************************
 * @file	trigonometric.hpp
 * @author	jjyou
 * @date	2024-2-4
 * @brief	This file implements trigonometric related functions.
***********************************************************************/

#ifndef jjyou_glsl_trigonometric_hpp
#define jjyou_glsl_trigonometric_hpp

#include <cmath>
#include <numbers>

namespace jjyou {

	namespace glsl {

		/** @brief	Convert degrees to radians.
		  * @param	scalar	Scalar in degrees.
		  * @return	Scalar in radians.
		  */
		template <class T> inline constexpr T radians(const T& scalar) {
			return scalar / static_cast<T>(180.0) * std::numbers::pi_v<T>;
		}

		/** @brief	Convert degrees to radians.
		  * @param	v		Vector of values in degrees.
		  * @return	Vector of values in radians.
		  */
		template <class T, int Length> inline constexpr vec<T, Length> radians(const vec<T, Length>& v) {
			vec<T, Length> res;
			for (int i = 0; i < Length; ++i)
				res[i] = radians(v[i]);
			return res;
		}

		/** @brief	Convert radians to degrees.
		  * @param	scalar	Scalar in radians.
		  * @return	Scalar in degrees.
		  */
		template <class T> inline constexpr T degrees(const T& scalar) {
			return scalar * static_cast<T>(180.0) / std::numbers::pi_v<T>;
		}

		/** @brief	Convert radians to degrees.
		  * @param	v		Vector of values in radians.
		  * @return	Vector of values in degrees.
		  */
		template <class T, int Length> inline constexpr vec<T, Length> degrees(const vec<T, Length>& v) {
			vec<T, Length> res;
			for (int i = 0; i < Length; ++i)
				res[i] = degrees(v[i]);
			return res;
		}

	}

}

#endif /* jjyou_glsl_trigonometric_hpp */