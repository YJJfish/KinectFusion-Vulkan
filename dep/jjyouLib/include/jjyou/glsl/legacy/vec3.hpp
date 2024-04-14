/***********************************************************************
 * @file	vec3.hpp
 * @author	jjyou
 * @date	2024-2-1
 * @brief	This file implements jjyou::glsl::vec<T, 3> class.
***********************************************************************/

#ifndef jjyou_glsl_vec3_hpp
#define jjyou_glsl_vec3_hpp

#include "base.hpp"

namespace jjyou {

	namespace glsl {

		template <class T>
		class vec<T, 3> {

		public:

			/** @name	Type definitions and inline constants.
			  */
			//@{
			using value_type = T;
			using length_type = std::size_t;
			using reference = value_type&;
			using const_reference = const value_type&;
			static inline length_type length = 3ULL;
			//@}

		public:

			/** @name	Constructor.
			  */
			//@{
			vec(void) = default;
			vec(const vec&) = default;
			vec(value_type scalar) : x(scalar), y(scalar), z(scalar) {}
			vec(value_type x, value_type y, value_type z) : x(x), y(y), z(z) {}
			vec(const vec<value_type, 2>& xy, value_type z) : x(xy.x), y(xy.y), z(z) {}
			vec(value_type x, const vec<value_type, 2>& yz) : x(x), y(yz.x), z(yz.y) {}
			vec(const vec<value_type, 4>& v) : x(v.x), y(v.y), z(v.z) {}
			//@}

			/** @name	Public methods.
			  */
			//@{
			template <class U> vec<U, 3> cast(void) const {
				return vec<U, 3>(
					static_cast<U>(this->x),
					static_cast<U>(this->y),
					static_cast<U>(this->z)
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
				return *this;
			}
			vec& operator+=(value_type scalar) {
				this->x += scalar;
				this->y += scalar;
				this->z += scalar;
				return *this;
			}
			vec& operator+=(const vec& v) {
				this->x += v.x;
				this->y += v.y;
				this->z += v.z;
				return *this;
			}
			vec& operator-=(value_type scalar) {
				this->x -= scalar;
				this->y -= scalar;
				this->z -= scalar;
				return *this;
			}
			vec& operator-=(const vec& v) {
				this->x -= v.x;
				this->y -= v.y;
				this->z -= v.z;
				return *this;
			}
			vec& operator*=(value_type scalar) {
				this->x *= scalar;
				this->y *= scalar;
				this->z *= scalar;
				return *this;
			}
			vec& operator*=(const vec& v) {
				this->x *= v.x;
				this->y *= v.y;
				this->z *= v.z;
				return *this;
			}
			vec& operator/=(value_type scalar) {
				this->x /= scalar;
				this->y /= scalar;
				this->z /= scalar;
				return *this;
			}
			vec& operator/=(const vec& v) {
				this->x /= v.x;
				this->y /= v.y;
				this->z /= v.z;
				return *this;
			}
			//@}

		public:

			/** @name	Data storage.
			  */
			//@{
			union {
				std::array<value_type, 3> data;
				struct {
					union { value_type x, r, s; };
					union { value_type y, g, t; };
					union { value_type z, b, p; };
				};
			};
			//@}

		};

		/** @name	Non-member functions.
		  */
		//@{
		template <class T> vec<T, 3> operator+(const vec<T, 3>& v) {
			return vec<T, 3>(+v.x, +v.y, +v.z);
		}
		template <class T> vec<T, 3> operator-(const vec<T, 3>& v) {
			return vec<T, 3>(-v.x, -v.y, -v.z);
		}
		template <class T> vec<T, 3> operator+(const vec<T, 3>& v1, const vec<T, 3>& v2) {
			return vec<T, 3>(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
		}
		template <class T> vec<T, 3> operator-(const vec<T, 3>& v1, const vec<T, 3>& v2) {
			return vec<T, 3>(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
		}
		template <class T> vec<T, 3> operator*(const vec<T, 3>& v1, const vec<T, 3>& v2) {
			return vec<T, 3>(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z);
		}
		template <class T> vec<T, 3> operator/(const vec<T, 3>& v1, const vec<T, 3>& v2) {
			return vec<T, 3>(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z);
		}
		template <class T> bool operator==(const vec<T, 3>& v1, const vec<T, 3>& v2) {
			return (v1.x == v2.x) && (v1.y == v2.y) && (v1.z == v2.z);
		}
		template <class T> bool operator!=(const vec<T, 3>& v1, const vec<T, 3>& v2) {
			return !(v1 == v2);
		}
		//@}

		/** @name	Type definitions for vec<T, 3>.
		  */
		//@{
		using bvec3 = vec<bool, 3>;
		using ivec3 = vec<int, 3>;
		using uvec3 = vec<unsigned int, 3>;
		using vec3 = vec<float, 3>;
		using dvec3 = vec<double, 3>;
		//@}

	}

}

#endif /* jjyou_glsl_vec3_hpp */