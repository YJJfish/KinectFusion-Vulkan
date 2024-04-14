/***********************************************************************
 * @file	vec.hpp
 * @author	jjyou
 * @date	2024-2-3
 * @brief	This file implements jjyou::glsl::vec class.
 * 
 *			In C++ we can use `std::enable_if` (since C++11) or
 *			`requires` (since C++20) to implement conditional member
 *			functions, but there is no way to directly implement
 *			conditional member variables. `[[no_unique_address]]` may
 *			be used to partially implement conditional member variables
 *			by defining unwanted variables as empty structs, but it
 *			depends on the compiler vendors and may act differently
 *			on different machines.
 *			One solution is to use template specialization, just like
 *			what GLM library does. However, this requires to overload
 *			many common member functions as well. In this library, we
 *			define a template base class `jjyou::glsl::vec_base<T, Length>`,
 *			and specialize it to have different member variables for
 *			different lengths. Then we implement an inheritance class
 *			`jjyou::glsl::vec<T, Length>` without specialization.
 *			In this inheritance class we can easily implement common
 *			member functions and use `requires` keyword to "specialize"
 *			noncommon member functions.
***********************************************************************/

#ifndef jjyou_glsl_vec_hpp
#define jjyou_glsl_vec_hpp

#include <array>
#include <exception>
#include <stdexcept>

namespace jjyou {

	namespace glsl {

		/***********************************************************************
		 * @class vec_base
		 * @brief Base class for `jjyou::glsl::vec` class.
		 *
		 * This class defines different member variables for different lengths.
		 * C++ does not provide a way to implement conditional member variables,
		 * therefore we need do template specialization.
		 * 
		 * @tparam	T		Value type of the vector.
		 * @tparam	Length	Vector length.
		 ***********************************************************************/
		template <class T, int Length>
		class vec_base;

		template <class T>
		class vec_base<T, 2> {
		protected:
			constexpr vec_base(void) : data() {}
			constexpr vec_base(T x, T y) : x(x), y(y) {}
			constexpr vec_base(T scalar) : x(scalar), y(scalar) {}
			vec_base(const vec_base& v) : data(v.data) {}
			vec_base(vec_base&& v) : data(std::move(v.data)) {}
			vec_base& operator=(const vec_base& v) { this->data = v.data; return *this; }
			vec_base& operator=(vec_base&& v) { this->data = std::move(v.data); return *this; }
		public:
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
		protected:
			constexpr vec_base(void) : data() {}
			constexpr vec_base(T x, T y, T z) : x(x), y(y), z(z) {}
			constexpr vec_base(T scalar) : x(scalar), y(scalar), z(scalar) {}
			vec_base(const vec_base& v) : data(v.data) {}
			vec_base(vec_base&& v) : data(std::move(v.data)) {}
			vec_base& operator=(const vec_base& v) { this->data = v.data; return *this; }
			vec_base& operator=(vec_base&& v) { this->data = std::move(v.data); return *this; }
		public:
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
		protected:
			constexpr vec_base(void) : data() {}
			constexpr vec_base(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}
			constexpr vec_base(T scalar) : x(scalar), y(scalar), z(scalar), w(scalar) {}
			vec_base(const vec_base& v) : data(v.data) {}
			vec_base(vec_base&& v) : data(std::move(v.data)) {}
			vec_base& operator=(const vec_base& v) { this->data = v.data; return *this; }
			vec_base& operator=(vec_base&& v) noexcept { this->data = std::move(v.data); return *this; }
		public:
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

		/***********************************************************************
		 * @class vec
		 * @brief GLSL vector class.
		 *
		 * This class inherits `jjyou::glsl::vec_base` and implements member functions
		 * for vector math operations. We can use `requires` keyword to implement
		 * different functions for different template parameters, therefore we don't
		 * need to do template specialization.
		 * 
		 * @tparam	T		Value type of the vector.
		 * @tparam	Length	Vector length.
		 ***********************************************************************/
		template <class T, int Length>
		class vec : public vec_base<T, Length> {

		public:

			/** @name	Type definitions and inline constants.
			  */
			//@{
			using value_type = T;
			using length_type = std::size_t;
			using reference = value_type&;
			using const_reference = const value_type&;
			static inline length_type length = Length;
			//@}

		public:

			/** @name	Common constructors.
			  */
			//@{
			constexpr vec(void) = default;
			vec(const vec&) = default;
			vec(vec&&) = default;
			/** @brief	Initialize each component of the vector to a scalar.
			  */
			constexpr vec(value_type scalar) : vec_base<T, Length>(scalar) {}
			//@}

			/** @name	Constructors for vec<T, 2>.
			  */
			//@{
			constexpr vec(value_type x, value_type y) requires (Length == 2) : vec_base<T, Length>(x, y){}
			vec(const vec<value_type, 3>& v) requires (Length == 2) : vec_base<T, Length>(v.x, v.y) {}
			vec(const vec<value_type, 4>& v) requires (Length == 2) : vec_base<T, Length>(v.x, v.y) {}
			//@}

			/** @name	Constructors for vec<T, 3>.
			  */
			//@{
			constexpr vec(value_type x, value_type y, value_type z)  requires (Length == 3) : vec_base<T, Length>(x, y, z) {}
			vec(const vec<value_type, 2>& xy, value_type z)  requires (Length == 3) : vec_base<T, Length>(xy.x, xy.y, z) {}
			vec(value_type x, const vec<value_type, 2>& yz)  requires (Length == 3) : vec_base<T, Length>(x, yz.x, yz.y) {}
			vec(const vec<value_type, 4>& v)  requires (Length == 3) : vec_base<T, Length>(v.x, v.y, v.z) {}
			//@}

			/** @name	Constructors for vec<T, 4>.
			  */
			//@{
			constexpr vec(value_type x, value_type y, value_type z, value_type w) requires (Length == 4) : vec_base<T, Length>(x, y, z, w) {}
			vec(const vec<value_type, 2>& xy, value_type z, value_type w) requires (Length == 4) : vec_base<T, Length>(xy.x, xy.y, z, w) {}
			vec(value_type x, const vec<value_type, 2>& yz, value_type w) requires (Length == 4) : vec_base<T, Length>(x, yz.x, yz.y, w) {}
			vec(value_type x, value_type y, const vec<value_type, 2>& zw) requires (Length == 4) : vec_base<T, Length>(x, y, zw.x, zw.y) {}
			vec(const vec<value_type, 2>& xy, const vec<value_type, 2>& zw) requires (Length == 4) : vec_base<T, Length>(xy.x, xy.y, zw.x, zw.y) {}
			vec(const vec<value_type, 3>& xyz, value_type w) requires (Length == 4) : vec_base<T, Length>(xyz.x, xyz.y, xyz.z, w) {}
			vec(value_type x, const vec<value_type, 3>& yzw) requires (Length == 4) : vec_base<T, Length>(x, yzw.x, yzw.y, yzw.z) {}
			//@}

			/** @name	Public methods.
			  */
			//@{
			template <class U> constexpr vec<U, Length> cast(void) const {
				vec<U, Length> ret;
				for (int i = 0; i < Length; ++i)
					ret.data[i] = static_cast<U>(this->data[i]);
				return ret;
			}
			reference operator[](length_type pos) { return this->data[pos]; }
			const_reference operator[](length_type pos) const { return this->data[pos]; }
			reference at(length_type pos) { if (pos >= Length) throw std::out_of_range("Index out of range"); return this->data[pos]; }
			const_reference at(length_type pos) const { if (pos >= Length) throw std::out_of_range("Index out of range"); return this->data[pos]; }
			vec& operator=(const vec&) = default;
			vec& operator=(vec&&) = default;
			vec& operator=(value_type scalar) {
				for (int i = 0; i < Length; ++i)
					this->data[i] = scalar;
				return *this;
			}
			vec& operator+=(value_type scalar) {
				for (int i = 0; i < Length; ++i)
					this->data[i] += scalar;
				return *this;
			}
			vec& operator+=(const vec& v) {
				for (int i = 0; i < Length; ++i)
					this->data[i] += v.data[i];
				return *this;
			}
			vec& operator-=(value_type scalar) {
				for (int i = 0; i < Length; ++i)
					this->data[i] -= scalar;
				return *this;
			}
			vec& operator-=(const vec& v) {
				for (int i = 0; i < Length; ++i)
					this->data[i] -= v.data[i];
				return *this;
			}
			vec& operator*=(value_type scalar) {
				for (int i = 0; i < Length; ++i)
					this->data[i] *= scalar;
				return *this;
			}
			vec& operator*=(const vec& v) {
				for (int i = 0; i < Length; ++i)
					this->data[i] *= v.data[i];
				return *this;
			}
			vec& operator/=(value_type scalar) {
				for (int i = 0; i < Length; ++i)
					this->data[i] /= scalar;
				return *this;
			}
			vec& operator/=(const vec& v) {
				for (int i = 0; i < Length; ++i)
					this->data[i] /= v.data[i];
				return *this;
			}
			//@}

		};

		/** @name	Non-member element-wise functions.
		  */
		//@{
		template <class T, int Length> inline vec<T, Length> operator+(const vec<T, Length>& v) {
			vec<T, Length> ret;
			for (int i = 0; i < Length; ++i)
				ret.data[i] = +v.data[i];
			return ret;
		}
		template <class T, int Length> inline vec<T, Length> operator-(const vec<T, Length>& v) {
			vec<T, Length> ret;
			for (int i = 0; i < Length; ++i)
				ret.data[i] = static_cast<T>(0.0) - v.data[i];
			return ret;
		}
		template <class T, int Length> inline vec<T, Length> operator+(const vec<T, Length>& v, T s) {
			vec<T, Length> ret;
			for (int i = 0; i < Length; ++i)
				ret.data[i] = v.data[i] + s;
			return ret;
		}
		template <class T, int Length> inline vec<T, Length> operator+(T s, const vec<T, Length>& v) {
			return v + s;
		}
		template <class T, int Length> inline vec<T, Length> operator+(const vec<T, Length>& v1, const vec<T, Length>& v2) {
			vec<T, Length> ret;
			for (int i = 0; i < Length; ++i)
				ret.data[i] = v1.data[i] + v2.data[i];
			return ret;
		}
		template <class T, int Length> inline vec<T, Length> operator-(const vec<T, Length>& v, T s) {
			vec<T, Length> ret;
			for (int i = 0; i < Length; ++i)
				ret.data[i] = v.data[i] - s;
			return ret;
		}
		template <class T, int Length> inline vec<T, Length> operator-(const vec<T, Length>& v1, const vec<T, Length>& v2) {
			vec<T, Length> ret;
			for (int i = 0; i < Length; ++i)
				ret.data[i] = v1.data[i] - v2.data[i];
			return ret;
		}
		template <class T, int Length> inline vec<T, Length> operator*(const vec<T, Length>& v, T s) {
			vec<T, Length> ret;
			for (int i = 0; i < Length; ++i)
				ret.data[i] = v.data[i] * s;
			return ret;
		}
		template <class T, int Length> inline vec<T, Length> operator*(T s, const vec<T, Length>& v) {
			return v * s;
		}
		template <class T, int Length> inline vec<T, Length> operator*(const vec<T, Length>& v1, const vec<T, Length>& v2) {
			vec<T, Length> ret;
			for (int i = 0; i < Length; ++i)
				ret.data[i] = v1.data[i] * v2.data[i];
			return ret;
		}
		template <class T, int Length> inline vec<T, Length> operator/(const vec<T, Length>& v, T s) {
			vec<T, Length> ret;
			for (int i = 0; i < Length; ++i)
				ret.data[i] = v.data[i] / s;
			return ret;
		}
		template <class T, int Length> inline vec<T, Length> operator/(const vec<T, Length>& v1, const vec<T, Length>& v2) {
			vec<T, Length> ret;
			for (int i = 0; i < Length; ++i)
				ret.data[i] = v1.data[i] / v2.data[i];
			return ret;
		}
		template <class T, int Length> inline bool operator==(const vec<T, Length>& v1, const vec<T, Length>& v2) {
			return (v1.data == v2.data);
		}
		template <class T, int Length> inline bool operator!=(const vec<T, Length>& v1, const vec<T, Length>& v2) {
			return !(v1 == v2);
		}
		//@}

		/** @name	Type definitions for convenience.
		  */
		//@{
		using bvec2 = vec<bool, 2>;
		using ivec2 = vec<int, 2>;
		using uvec2 = vec<unsigned int, 2>;
		using vec2 = vec<float, 2>;
		using dvec2 = vec<double, 2>;

		using bvec3 = vec<bool, 3>;
		using ivec3 = vec<int, 3>;
		using uvec3 = vec<unsigned int, 3>;
		using vec3 = vec<float, 3>;
		using dvec3 = vec<double, 3>;

		using bvec4 = vec<bool, 4>;
		using ivec4 = vec<int, 4>;
		using uvec4 = vec<unsigned int, 4>;
		using vec4 = vec<float, 4>;
		using dvec4 = vec<double, 4>;
		//@}
	}

}

template <class T, int Length>
struct ::std::hash<::jjyou::glsl::vec<T, Length>> {
	using argument_type = ::jjyou::glsl::vec<T, Length>;
	using result_type = size_t;
	result_type operator()(argument_type const& key) const {
		static const ::std::hash<T> h;
		result_type ret = 0;
		for (int i = 0; i < Length; ++i)
			ret ^= h(key[i]) + 0x9e3779b9 + (ret << 6) + (ret >> 2);
		return ret;
	}
};

#endif /* jjyou_glsl_vec_hpp */