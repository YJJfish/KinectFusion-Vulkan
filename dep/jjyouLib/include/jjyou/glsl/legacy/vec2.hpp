/***********************************************************************
 * @file	vec2.hpp
 * @author	jjyou
 * @date	2024-2-1
 * @brief	This file implements jjyou::glsl::vec<T, 2> class.
***********************************************************************/

#ifndef jjyou_glsl_vec2_hpp
#define jjyou_glsl_vec2_hpp

#include "base.hpp"

namespace jjyou {

	namespace glsl {

		template <class T, int Length>
		class vec_base;
		template <class T>
		class vec_base<T, 2> {
		public:
			vec_base(T x, T y) : x(x), y(y) {}
			vec_base(const vec_base&) = default;
			vec_base(vec_base&&) = default;
			union {
				std::array<T, 2> data;
				struct {
					union { T x, r, s; };
					union { T y, g, t; };
				};
			};
		};
		template <class T>
		class vec_base<T, 3> {
		public:
			vec_base(T x, T y, T z) : x(x), y(y), z(z) {}
			vec_base(const vec_base&) = default;
			vec_base(vec_base&&) = default;
			union {
				std::array<T, 3> data;
				struct {
					union { T x, r, s; };
					union { T y, g, t; };
					union { T z, b, p; };
				};
			};
		};
		template <class T>
		class vec_base<T, 4> {
		public:
			vec_base(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}
			vec_base(const vec_base&) = default;
			vec_base(vec_base&&) = default;
			union {
				std::array<T, 4> data;
				struct {
					union { T x, r, s; };
					union { T y, g, t; };
					union { T z, b, p; };
					union { T w, a, q; };
				};
			};
		};

		template <class T, int Length>
		class vec : public vec_base<T, Length> {

		};

		template <class T>
		class vec<T, 2> {

		public:

			/** @name	Type definitions and inline constants.
			  */
			//@{
			using value_type = T;
			using length_type = std::size_t;
			using reference = value_type&;
			using const_reference = const value_type&;
			static inline length_type length = 2ULL;
			//@}

		public:

			/** @name	Constructor.
			  */
			//@{
			vec(void) = default;
			vec(const vec&) = default;
			vec(value_type scalar) : x(scalar), y(scalar) {}
			vec(value_type x, value_type y) : x(x), y(y) {}
			vec(const vec<value_type, 3>& v) : x(v.x), y(v.y) {}
			vec(const vec<value_type, 4>& v) : x(v.x), y(v.y) {}
			//@}

			/** @name	Public methods.
			  */
			//@{
			template <class U> vec<U, 2> cast(void) const {
				return vec<U, 2>(
					static_cast<U>(this->x),
					static_cast<U>(this->y)
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
				return *this;
			}
			vec& operator+=(value_type scalar) {
				this->x += scalar;
				this->y += scalar;
				return *this;
			}
			vec& operator+=(const vec& v) {
				this->x += v.x;
				this->y += v.y;
				return *this;
			}
			vec& operator-=(value_type scalar) {
				this->x -= scalar;
				this->y -= scalar;
				return *this;
			}
			vec& operator-=(const vec& v) {
				this->x -= v.x;
				this->y -= v.y;
				return *this;
			}
			vec& operator*=(value_type scalar) {
				this->x *= scalar;
				this->y *= scalar;
				return *this;
			}
			vec& operator*=(const vec& v) {
				this->x *= v.x;
				this->y *= v.y;
				return *this;
			}
			vec& operator/=(value_type scalar) {
				this->x /= scalar;
				this->y /= scalar;
				return *this;
			}
			vec& operator/=(const vec& v) {
				this->x /= v.x;
				this->y /= v.y;
				return *this;
			}
			//@}

		public:

			/** @name	Data storage.
			  */
			//@{
			union {
				std::array<value_type, 2> data;
				struct {
					union { value_type x, r, s; };
					union { value_type y, g, t; };
				};
			};
			//@}

		};

		/** @name	Non-member functions.
		  */
		//@{
		template <class T> vec<T, 2> operator+(const vec<T, 2>& v) {
			return vec<T, 2>(+v.x, +v.y);
		}
		template <class T> vec<T, 2> operator-(const vec<T, 2>& v) {
			return vec<T, 2>(-v.x, -v.y);
		}
		template <class T> vec<T, 2> operator+(const vec<T, 2>& v1, const vec<T, 2>& v2) {
			return vec<T, 2>(v1.x + v2.x, v1.y + v2.y);
		}
		template <class T> vec<T, 2> operator-(const vec<T, 2>& v1, const vec<T, 2>& v2) {
			return vec<T, 2>(v1.x - v2.x, v1.y - v2.y);
		}
		template <class T> vec<T, 2> operator*(const vec<T, 2>& v1, const vec<T, 2>& v2) {
			return vec<T, 2>(v1.x * v2.x, v1.y * v2.y);
		}
		template <class T> vec<T, 2> operator/(const vec<T, 2>& v1, const vec<T, 2>& v2) {
			return vec<T, 2>(v1.x / v2.x, v1.y / v2.y);
		}
		template <class T> bool operator==(const vec<T, 2>& v1, const vec<T, 2>& v2) {
			return (v1.x == v2.x) && (v1.y == v2.y);
		}
		template <class T> bool operator!=(const vec<T, 2>& v1, const vec<T, 2>& v2) {
			return !(v1 == v2);
		}
		//@}

		/** @name	Type definitions for vec<T, 2>.
		  */
		//@{
		using bvec2 = vec<bool, 2>;
		using ivec2 = vec<int, 2>;
		using uvec2 = vec<unsigned int, 2>;
		using vec2 = vec<float, 2>;
		using dvec2 = vec<double, 2>;
		//@}
	}

}

#endif /* jjyou_glsl_vec2_hpp */