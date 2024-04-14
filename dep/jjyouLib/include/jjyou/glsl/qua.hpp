/***********************************************************************
 * @file	qua.hpp
 * @author	jjyou
 * @date	2024-2-2
 * @brief	This file implements jjyou::glsl::qua<T> class.
***********************************************************************/

#ifndef jjyou_glsl_qua_hpp
#define jjyou_glsl_qua_hpp

#include <array>
#include <exception>
#include <stdexcept>

namespace jjyou{

	namespace glsl {

		template <class T>
		class qua {

		public:

			/** @name	Type definitions and inline constants.
			  */
			//@{
			using value_type = T;
			using length_type = std::size_t;
			using reference = value_type&;
			using const_reference = const value_type&;
			static inline length_type length = 4;
			//@}

		public:

			/** @name	Data storage.
			  */
			//@{
			union {
				std::array<value_type, 4> data;
				struct {
					value_type x, y, z, w;
				};
			};
			//@}

		public:

			/** @name	Constructors.
			  */
			//@{
			constexpr qua(void) : data() {}
			qua(const qua& q) : data(q.data) {}
			qua(qua&& q) : data(std::move(q.data)) {}
			qua(value_type x, value_type y, value_type z, value_type w) : x(x), y(y), z(z), w(w) {}
			/** @brief	Initialize each component of the vector to a scalar.
			  */
			
			//@}

			/** @name	Conversion to matrix.
			  */
			//@{
			operator mat<T, 3, 3>(void) const {
				constexpr T one = static_cast<T>(1.0);
				constexpr T two = static_cast<T>(2.0);
				T qxx(this->x * this->x);
				T qxy(this->x * this->y);
				T qxz(this->x * this->z);
				T qyy(this->y * this->y);
				T qyz(this->y * this->z);
				T qzz(this->z * this->z);
				T qwx(this->w * this->x);
				T qwy(this->w * this->y);
				T qwz(this->w * this->z);
				T qww(this->w * this->w);
				T s = one / (qxx + qyy + qzz + qww);
				mat<T, 3, 3> res(
					one - s * two * (qyy + qzz), two * s * (qxy + qwz), two * s * (qxz - qwy),
					two * s * (qxy - qwz), one - two * s * (qxx + qzz), two * s * (qyz + qwx),
					two * s * (qxz + qwy), two * s * (qyz - qwx), one - two * s * (qxx + qyy)
				);
				return res;
			}
			operator mat<T, 4, 4>(void) const {
				return mat<T, 4, 4>(this->operator mat<T, 3, 3>());
			}
			//@}

			/** @name	Public methods.
			  */
			//@{
			template <class U> constexpr qua<U> cast(void) const {
				qua<U> ret;
				for (int i = 0; i < length; ++i)
					ret.data[i] = static_cast<U>(this->data[i]);
				return ret;
			}
			reference operator[](length_type pos) { return this->data[pos]; }
			const_reference operator[](length_type pos) const { return this->data[pos]; }
			reference at(length_type pos) { if (pos >= length) throw std::out_of_range("Index out of range"); return this->data[pos]; }
			const_reference at(length_type pos) const { if (pos >= length) throw std::out_of_range("Index out of range"); return this->data[pos]; }
			qua& operator=(const qua&) = default;
			qua& operator=(qua&&) = default;
			qua& operator=(value_type scalar) {
				for (int i = 0; i < length; ++i)
					this->data[i] = scalar;
				return *this;
			}
			qua& operator+=(value_type scalar) {
				for (int i = 0; i < length; ++i)
					this->data[i] += scalar;
				return *this;
			}
			qua& operator+=(const qua& q) {
				for (int i = 0; i < length; ++i)
					this->data[i] += q.data[i];
				return *this;
			}
			qua& operator-=(value_type scalar) {
				for (int i = 0; i < length; ++i)
					this->data[i] -= scalar;
				return *this;
			}
			qua& operator-=(const qua& q) {
				for (int i = 0; i < length; ++i)
					this->data[i] -= q.data[i];
				return *this;
			}
			qua& operator*=(value_type scalar) {
				for (int i = 0; i < length; ++i)
					this->data[i] *= scalar;
				return *this;
			}
			qua& operator*=(const qua& q) {
				for (int i = 0; i < length; ++i)
					this->data[i] *= q.data[i];
				return *this;
			}
			qua& operator/=(value_type scalar) {
				for (int i = 0; i < length; ++i)
					this->data[i] /= scalar;
				return *this;
			}
			qua& operator/=(const qua& q) {
				for (int i = 0; i < length; ++i)
					this->data[i] /= q.data[i];
				return *this;
			}
			//@}

		};

		/** @name	Non-member element-wise functions.
		  */
		//@{
		template <class T> inline qua<T> operator+(const qua<T>& q) {
			qua<T> ret;
			for (int i = 0; i < 4; ++i)
				ret.data[i] = +q.data[i];
			return ret;
		}
		template <class T> inline qua<T> operator-(const qua<T>& q) {
			qua<T> ret;
			for (int i = 0; i < 4; ++i)
				ret.data[i] = -q.data[i];
			return ret;
		}
		template <class T> inline qua<T> operator+(const qua<T>& q, T s) {
			qua<T> ret;
			for (int i = 0; i < 4; ++i)
				ret.data[i] = q.data[i] + s;
			return ret;
		}
		template <class T> inline qua<T> operator+(T s, const qua<T>& q) {
			return q + s;
		}
		template <class T> inline qua<T> operator+(const qua<T>& q1, const qua<T>& q2) {
			qua<T> ret;
			for (int i = 0; i < 4; ++i)
				ret.data[i] = q1.data[i] + q2.data[i];
			return ret;
		}
		template <class T> inline qua<T> operator-(const qua<T>& q, T s) {
			qua<T> ret;
			for (int i = 0; i < 4; ++i)
				ret.data[i] = q.data[i] - s;
			return ret;
		}
		template <class T> inline qua<T> operator-(const qua<T>& q1, const qua<T>& q2) {
			qua<T> ret;
			for (int i = 0; i < 4; ++i)
				ret.data[i] = q1.data[i] - q2.data[i];
			return ret;
		}
		template <class T> inline qua<T> operator*(const qua<T>& q, T s) {
			qua<T> ret;
			for (int i = 0; i < 4; ++i)
				ret.data[i] = q.data[i] * s;
			return ret;
		}
		template <class T> inline qua<T> operator*(T s, const qua<T>& q) {
			return q * s;
		}
		template <class T> inline qua<T> operator*(const qua<T>& q1, const qua<T>& q2) {
			qua<T> ret;
			for (int i = 0; i < 4; ++i)
				ret.data[i] = q1.data[i] * q2.data[i];
			return ret;
		}
		template <class T> inline qua<T> operator/(const qua<T>& q, T s) {
			qua<T> ret;
			for (int i = 0; i < 4; ++i)
				ret.data[i] = q.data[i] / s;
			return ret;
		}
		template <class T> inline qua<T> operator/(const qua<T>& q1, const qua<T>& q2) {
			qua<T> ret;
			for (int i = 0; i < 4; ++i)
				ret.data[i] = q1.data[i] / q2.data[i];
			return ret;
		}
		template <class T> inline bool operator==(const qua<T>& q1, const qua<T>& q2) {
			return (q1.data == q2.data);
		}
		template <class T> inline bool operator!=(const qua<T>& q1, const qua<T>& q2) {
			return !(q1 == q2);
		}
		//@}

		/** @name	Type definitions for convenience.
		  */
		//@{
		using quat = qua<float>;
		using dquat = qua<double>;
		//@}

	}

}



#endif /* jjyou_glsl_qua_hpp */