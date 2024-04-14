/***********************************************************************
 * @file	mat.hpp
 * @author	jjyou
 * @date	2024-2-2
 * @brief	This file implements jjyou::glsl::mat<T, Cols, Rows> class.
***********************************************************************/

#ifndef jjyou_glsl_mat_hpp
#define jjyou_glsl_mat_hpp

#include <array>
#include <exception>
#include <stdexcept>

namespace jjyou {

	namespace glsl {

		/***********************************************************************
		 * @class mat
		 * @brief GLSL matrix class.
		 *
		 * Note that the first template parameter is the number of columns, which
		 * is different from mathematical conventions. Also, the matrix is stored
		 * in column-major order.
		 *
		 * @tparam	T		Value type of the vector.
		 * @tparam	Cols	Number of columns.
		 * @tparam	Rows	Number of rows.
		 ***********************************************************************/
		template <class T, int Cols, int Rows>
		class mat {

		public:

			/** @name	Type definitions and inline constants.
			  */
			//@{
			using value_type = T;
			using length_type = std::size_t;
			using reference = value_type&;
			using const_reference = const value_type&;
			using col_type = vec<T, Rows>;
			using row_type = vec<T, Cols>;
			using transpose_type = mat<T, Rows, Cols>;
			static inline length_type cols = Cols;
			static inline length_type rows = Rows;
			static inline length_type size = Cols * Rows;
			//@}

		public:

			/** @name	Common constructors.
			  */
			//@{
			constexpr mat(void) : data() {}
			mat(const mat& m) : data(m.data) {}
			mat(mat&& m) noexcept : data(std::move(m.data)) {}

			/** @brief	Construct from another matrix of different size.
			  * 
			  *			The common submatrix will be copied. Other elements
			  *			will be initialized to the identity matrix.
			  */
			template <int _Cols, int _Rows>
			mat(const mat<T, _Cols, _Rows>& m) : data() {
				constexpr int commonCols = std::min(Cols, _Cols);
				constexpr int commonRows = std::min(Rows, _Rows);
				for (int c = 0; c < commonCols; ++c)
					for (int r = 0; r < commonRows; ++r)
						this->col[c][r] = m.col[c][r];
				constexpr int minCommonDim = std::min(commonCols, commonRows);
				constexpr int minDim = std::min(Cols, Rows);
				for (int i = minCommonDim; i < minDim; ++i)
					this->col[i][i] = static_cast<value_type>(1.0);
			}
			/** @brief	Construct from a scalar.
			  *
			  *			The diagonal elements will be initialized to the scalar.
			  *			Other elements will be initialized to zero.
			  */
			constexpr mat(value_type scalar) : data() {
				constexpr int minDim = std::min(Cols, Rows);
				for (int i = 0; i < minDim; ++i)
					this->col[i][i] = scalar;
			}
			//@}

			/** @name	Constructors for mat2xX.
			  */
			//@{
			mat(
				const col_type& col0,
				const col_type& col1
			) requires (Cols == 2) : col{ {col0, col1} } {}
			//@}

			/** @name	Constructors for mat3xX.
			  */
			//@{
			mat(
				const col_type& col0,
				const col_type& col1,
				const col_type& col2
			) requires (Cols == 3) : col{ {col0, col1, col2} } {}
			//@}

			/** @name	Constructors for mat4xX.
			  */
			//@{
			mat(
				const col_type& col0,
				const col_type& col1,
				const col_type& col2,
				const col_type& col3
			) requires (Cols == 4) : col{ {col0, col1, col2, col3} } {}
			//@}
			
			/** @name	Constructors for mat2x2.
			  */
			//@{
			constexpr mat(value_type m00, value_type m10,
				value_type m01, value_type m11
			) requires (Cols == 2 && Rows == 2) : col{ {col_type(m00, m10), col_type(m01, m11)} } {}
			//@}

			/** @name	Constructors for mat2x3.
			  */
			//@{
			constexpr mat(value_type m00, value_type m10, value_type m20,
				value_type m01, value_type m11, value_type m21
			) requires (Cols == 2 && Rows == 3) : col{ {col_type(m00, m10, m20), col_type(m01, m11, m21)} } {}
			//@}

			/** @name	Constructors for mat2x4.
			  */
			//@{
			constexpr mat(value_type m00, value_type m10, value_type m20, value_type m30,
				value_type m01, value_type m11, value_type m21, value_type m31
			) requires (Cols == 2 && Rows == 4) : col{ {col_type(m00, m10, m20, m30), col_type(m01, m11, m21, m31)} } {}
			//@}

			/** @name	Constructors for mat3x2.
			  */
			//@{
			constexpr mat(value_type m00, value_type m10,
				value_type m01, value_type m11,
				value_type m02, value_type m12
			) requires (Cols == 3 && Rows == 2) : col{ {col_type(m00, m10), col_type(m01, m11), col_type(m02, m12)} } {}
			//@}

			/** @name	Constructors for mat3x3.
			  */
			//@{
			constexpr mat(value_type m00, value_type m10, value_type m20,
				value_type m01, value_type m11, value_type m21,
				value_type m02, value_type m12, value_type m22
			) requires (Cols == 3 && Rows == 3) : col{ {col_type(m00, m10, m20), col_type(m01, m11, m21), col_type(m02, m12, m22)} } {}
			//@}

			/** @name	Constructors for mat3x4.
			  */
			//@{
			constexpr mat(value_type m00, value_type m10, value_type m20, value_type m30,
				value_type m01, value_type m11, value_type m21, value_type m31,
				value_type m02, value_type m12, value_type m22, value_type m32
			) requires (Cols == 3 && Rows == 4) : col{ {col_type(m00, m10, m20, m30), col_type(m01, m11, m21, m31), col_type(m02, m12, m22, m32)} } {}
			//@}

			/** @name	Constructors for mat4x2.
			  */
			//@{
			constexpr mat(value_type m00, value_type m10,
				value_type m01, value_type m11,
				value_type m02, value_type m12,
				value_type m03, value_type m13
			) requires (Cols == 4 && Rows == 2) : col{ {col_type(m00, m10), col_type(m01, m11), col_type(m02, m12), col_type(m03, m13)} } {}
			//@}

			/** @name	Constructors for mat4x3.
			  */
			//@{
			constexpr mat(value_type m00, value_type m10, value_type m20,
				value_type m01, value_type m11, value_type m21,
				value_type m02, value_type m12, value_type m22,
				value_type m03, value_type m13, value_type m23
			) requires (Cols == 4 && Rows == 3) : col{ {col_type(m00, m10, m20), col_type(m01, m11, m21), col_type(m02, m12, m22), col_type(m03, m13, m23)} } {}
			//@}

			/** @name	Constructors for mat4x4.
			  */
			//@{
			constexpr mat(value_type m00, value_type m10, value_type m20, value_type m30,
				value_type m01, value_type m11, value_type m21, value_type m31,
				value_type m02, value_type m12, value_type m22, value_type m32,
				value_type m03, value_type m13, value_type m23, value_type m33
			) requires (Cols == 4 && Rows == 4) : col{ {col_type(m00, m10, m20, m30), col_type(m01, m11, m21, m31), col_type(m02, m12, m22, m32), col_type(m03, m13, m23, m33)} } {}
			//@}

			/** @name	Public methods.
			  */
			//@{
			template <class U> constexpr mat<U, Cols, Rows> cast(void) const {
				mat<U, Cols, Rows> ret;
				for (int i = 0; i < Cols * Rows; ++i)
					ret.data[i] = static_cast<U>(this->data[i]);
				return ret;
			}
			col_type& operator[](length_type col) { return this->col[col]; }
			const col_type& operator[](length_type col) const { return this->col[col]; }
			reference& operator()(length_type col, length_type row) { return this->col[col][row]; }
			const_reference& operator()(length_type col, length_type row) const { return this->col[col][row]; }
			reference& at(length_type col, length_type row) { if (col >= Cols || row >= Rows) throw std::out_of_range("Index out of range"); return this->col[col][row]; }
			const_reference& at(length_type col, length_type row) const { if (col >= Cols || row >= Rows) throw std::out_of_range("Index out of range"); return this->col[col][row]; }
			mat& operator=(const mat& m) { this->data = m.data; return *this; }
			mat& operator=(mat&& m) { this->data = std::move(m.data); return *this; }
			mat& operator+=(value_type scalar) {
				for (int i = 0; i < Cols * Rows; ++i)
					this->data[i] += scalar;
				return *this;
			}
			mat& operator+=(const mat& m) {
				for (int i = 0; i < Cols * Rows; ++i)
					this->data[i] += m.data[i];
				return *this;
			}
			mat& operator-=(value_type scalar) {
				for (int i = 0; i < Cols * Rows; ++i)
					this->data[i] -= scalar;
				return *this;
			}
			mat& operator-=(const mat& m) {
				for (int i = 0; i < Cols * Rows; ++i)
					this->data[i] -= m.data[i];
				return *this;
			}
			mat& operator*=(value_type scalar) {
				for (int i = 0; i < Cols * Rows; ++i)
					this->data[i] *= scalar;
				return *this;
			}
			mat& operator/=(value_type scalar) {
				for (int i = 0; i < Cols * Rows; ++i)
					this->data[i] /= scalar;
				return *this;
			}
			//@}

		public:

			/** @name	Data storage.
			  */
			//@{
			union {
				std::array<value_type, Cols * Rows> data;
				std::array<col_type, Cols> col;
			};
			//@}

		};

		/** @name	Non-member element-wise functions.
		  */
		//@{
		template <class T, int Cols, int Rows> inline mat<T, Cols, Rows> operator+(const mat<T, Cols, Rows>& m) {
			mat<T, Cols, Rows> ret;
			for (int i = 0; i < Cols * Rows; ++i)
				ret.data[i] = +m.data[i];
			return ret;
		}
		template <class T, int Cols, int Rows> inline mat<T, Cols, Rows> operator-(const mat<T, Cols, Rows>& m) {
			mat<T, Cols, Rows> ret;
			for (int i = 0; i < Cols * Rows; ++i)
				ret.data[i] = -m.data[i];
			return ret;
		}
		template <class T, int Cols, int Rows> inline mat<T, Cols, Rows> operator+(const mat<T, Cols, Rows>& m1, const mat<T, Cols, Rows>& m2) {
			mat<T, Cols, Rows> ret;
			for (int i = 0; i < Cols * Rows; ++i)
				ret.data[i] = m1.data[i] + m2.data[i];
			return ret;
		}
		template <class T, int Cols, int Rows> inline mat<T, Cols, Rows> operator-(const mat<T, Cols, Rows>& m1, const mat<T, Cols, Rows>& m2) {
			mat<T, Cols, Rows> ret;
			for (int i = 0; i < Cols * Rows; ++i)
				ret.data[i] = m1.data[i] - m2.data[i];
			return ret;
		}
		template <class T, int Cols, int Rows> inline mat<T, Cols, Rows> operator*(const mat<T, Cols, Rows>& m, T s) {
			mat<T, Cols, Rows> ret;
			for (int i = 0; i < Cols * Rows; ++i)
				ret.data[i] = m.data[i] * s;
			return ret;
		}
		template <class T, int Cols, int Rows> inline mat<T, Cols, Rows> operator*(T s, const mat<T, Cols, Rows>& m) {
			return m * s;
		}
		template <class T, int Cols, int Rows> inline mat<T, Cols, Rows> operator/(const mat<T, Cols, Rows>& m, T s) {
			mat<T, Cols, Rows> ret;
			for (int i = 0; i < Cols * Rows; ++i)
				ret.data[i] = m.data[i] / s;
			return ret;
		}
		template <class T, int Cols, int Rows> inline bool operator==(const mat<T, Cols, Rows>& m1, const mat<T, Cols, Rows>& m2) {
			return (m1.data == m2.data);
		}
		template <class T, int Cols, int Rows> inline bool operator!=(const mat<T, Cols, Rows>& m1, const mat<T, Cols, Rows>& m2) {
			return !(m1 == m2);
		}
		//@}

		/** @name	Type definitions for convenience.
		  */
		//@{
		using mat2x2 = mat<float, 2, 2>;
		using mat2x3 = mat<float, 2, 3>;
		using mat2x4 = mat<float, 2, 4>;
		using mat3x2 = mat<float, 3, 2>;
		using mat3x3 = mat<float, 3, 3>;
		using mat3x4 = mat<float, 3, 4>;
		using mat4x2 = mat<float, 4, 2>;
		using mat4x3 = mat<float, 4, 3>;
		using mat4x4 = mat<float, 4, 4>;

		using dmat2x2 = mat<double, 2, 2>;
		using dmat2x3 = mat<double, 2, 3>;
		using dmat2x4 = mat<double, 2, 4>;
		using dmat3x2 = mat<double, 3, 2>;
		using dmat3x3 = mat<double, 3, 3>;
		using dmat3x4 = mat<double, 3, 4>;
		using dmat4x2 = mat<double, 4, 2>;
		using dmat4x3 = mat<double, 4, 3>;
		using dmat4x4 = mat<double, 4, 4>;

		using mat2 = mat2x2;
		using mat3 = mat3x3;
		using mat4 = mat4x4;

		using dmat2 = dmat2x2;
		using dmat3 = dmat3x3;
		using dmat4 = dmat4x4;

		//@}

	}

}

template <class T, int Cols, int Rows>
struct ::std::hash<::jjyou::glsl::mat<T, Cols, Rows>> {
	using argument_type = ::jjyou::glsl::mat<T, Cols, Rows>;
	using result_type = size_t;
	result_type operator()(argument_type const& key) const {
		static const ::std::hash<T> h;
		result_type ret = 0;
		for (int i = 0; i < Cols * Rows; ++i)
			ret ^= h(key[i]) + 0x9e3779b9 + (ret << 6) + (ret >> 2);
		return ret;
	}
};

#endif /* jjyou_glsl_mat_hpp */