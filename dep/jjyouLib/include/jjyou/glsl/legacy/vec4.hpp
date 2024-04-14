/***********************************************************************
 * @file	vec4.hpp
 * @author	jjyou
 * @date	2024-2-1
 * @brief	This file implements jjyou::glsl::vec<T, 4> class.
***********************************************************************/

#ifndef jjyou_glsl_vec4_hpp
#define jjyou_glsl_vec4_hpp

#include "base.hpp"

namespace jjyou {

	namespace glsl {

		template <class T>
		class vec<T, 4> {

		public:

			/** @name	Type definitions and inline constants.
			  */
			//@{
			using value_type = T;
			using length_type = std::size_t;
			using reference = value_type&;
			using const_reference = const value_type&;
			static inline length_type length = 4ULL;
			//@}

		public:

			/** @name	Constructor.
			  */
			//@{
			vec(void) = default;
			vec(const vec&) = default;
			vec(value_type scalar) : x(scalar), y(scalar), z(scalar), w(scalar) {}
			vec(value_type x, value_type y, value_type z, value_type w) : x(x), y(y), z(z), w(w) {}
			vec(const vec<value_type, 2>& xy, value_type z, value_type w) : x(xy.x), y(xy.y), z(z), w(w) {}
			vec(value_type x, const vec<value_type, 2>& yz, value_type w) : x(x), y(yz.x), z(yz.y), w(w) {}
			vec(value_type x, value_type y, const vec<value_type, 2>& zw) : x(x), y(y), z(zw.x), w(zw.y) {}
			vec(const vec<value_type, 2>& xy, const vec<value_type, 2>& zw) : x(xy.x), y(xy.y), z(zw.x), w(zw.y) {}
			vec(const vec<value_type, 3>& xyz, value_type w) : x(xyz.x), y(xyz.y), z(xyz.z), w(w) {}
			vec(value_type x, const vec<value_type, 3>& yzw) : x(x), y(yzw.x), z(yzw.y), w(yzw.z) {}
			//@}

			/** @name	Public methods.
			  */
			//@{
			template <class U> vec<U, 4> cast(void) const {
				return vec<U, 4>(
					static_cast<U>(this->x),
					static_cast<U>(this->y),
					static_cast<U>(this->z),
					static_cast<U>(this->w)
				);
			}
			reference operator[](length_type pos) { return this->data[pos]; }
			const_reference operator[](length_type pos) const { return this->data[pos]; }
			reference at(length_type pos) { if (pos >= this->length) throw std::out_of_range("Index out of range"); return this->data[pos]; }
			const_reference at(length_type pos) const { if (pos >= this->length) throw std::out_of_range("Index out of range"); return this->data[pos]; }
			vec& operator=(const vec&) = default;
			vec& operator=(value_type scalar) {
				this->x = scalar;
				this->y = scalar;
				this->z = scalar;
				this->w = scalar;
				return *this;
			}
			vec& operator+=(value_type scalar) {
				this->x += scalar;
				this->y += scalar;
				this->z += scalar;
				this->w += scalar;
				return *this;
			}
			vec& operator+=(const vec& v) {
				this->x += v.x;
				this->y += v.y;
				this->z += v.z;
				this->w += v.w;
				return *this;
			}
			vec& operator-=(value_type scalar) {
				this->x -= scalar;
				this->y -= scalar;
				this->z -= scalar;
				this->w -= scalar;
				return *this;
			}
			vec& operator-=(const vec& v) {
				this->x -= v.x;
				this->y -= v.y;
				this->z -= v.z;
				this->w -= v.w;
				return *this;
			}
			vec& operator*=(value_type scalar) {
				this->x *= scalar;
				this->y *= scalar;
				this->z *= scalar;
				this->w *= scalar;
				return *this;
			}
			vec& operator*=(const vec& v) {
				this->x *= v.x;
				this->y *= v.y;
				this->z *= v.z;
				this->w *= v.w;
				return *this;
			}
			vec& operator/=(value_type scalar) {
				this->x /= scalar;
				this->y /= scalar;
				this->z /= scalar;
				this->w /= scalar;
				return *this;
			}
			vec& operator/=(const vec& v) {
				this->x /= v.x;
				this->y /= v.y;
				this->z /= v.z;
				this->w /= v.w;
				return *this;
			}
			//@}

		public:

			/** @name	Data storage.
			  */
			//@{
			union {
				std::array<T, 4> data;
				struct {
					union { T x, r, s; };
					union { T y, g, t; };
					union { T z, b, p; };
					union { T w, a, q; };
				};
			};
			//@}

		};

		/** @name	Non-member functions.
		  */
		//@{
		template <class T> vec<T, 4> operator+(const vec<T, 4>& v) {
			return vec<T, 4>(+v.x, +v.y, +v.z, +v.w);
		}
		template <class T> vec<T, 4> operator-(const vec<T, 4>& v) {
			return vec<T, 4>(-v.x, -v.y, -v.z, -v.w);
		}
		template <class T> vec<T, 4> operator+(const vec<T, 4>& v1, const vec<T, 4>& v2) {
			return vec<T, 4>(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w);
		}
		template <class T> vec<T, 4> operator-(const vec<T, 4>& v1, const vec<T, 4>& v2) {
			return vec<T, 4>(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w);
		}
		template <class T> vec<T, 4> operator*(const vec<T, 4>& v1, const vec<T, 4>& v2) {
			return vec<T, 4>(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z, v1.w * v2.w);
		}
		template <class T> vec<T, 4> operator/(const vec<T, 4>& v1, const vec<T, 4>& v2) {
			return vec<T, 4>(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z, v1.w / v2.w);
		}
		template <class T> bool operator==(const vec<T, 4>& v1, const vec<T, 4>& v2) {
			return (v1.x == v2.x) && (v1.y == v2.y) && (v1.z == v2.z) && (v1.w == v2.w);
		}
		template <class T> bool operator!=(const vec<T, 4>& v1, const vec<T, 4>& v2) {
			return !(v1 == v2);
		}
		//@}

		/** @name	Type definitions for vec<T, 4>.
		  */
		//@{
		using bvec4 = vec<bool, 4>;
		using ivec4 = vec<int, 4>;
		using uvec4 = vec<unsigned int, 4>;
		using vec4 = vec<float, 4>;
		using dvec4 = vec<double, 4>;
		//@}

	}

}

#endif /* jjyou_glsl_vec4_hpp */