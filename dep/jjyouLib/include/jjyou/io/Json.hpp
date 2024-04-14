/***********************************************************************
 * @file	Json.hpp
 * @author	jjyou
 * @date	2024-2-9
 * @brief	This file implements Json class.
***********************************************************************/
#ifndef jjyou_io_Json_hpp
#define jjyou_io_Json_hpp

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <vector>
#include <stack>
#include <map>
#include <variant>
#include <optional>
#include <iterator>
#include <exception>
#include <cmath>
#include <locale>
#include <codecvt>

namespace jjyou {

	namespace io {

		/*============================================================
		 *                    Forward declarations
		 *============================================================*/
		enum class JsonType;
		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy> class Json;
		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy> class JsonIterator;
		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy> class JsonConstIterator;
		template <class StringTy> class JsonInputAdapter;
		enum class JsonTokenType;
		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy> class JsonToken;
		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy> class JsonLexer;
		/*============================================================
		 *                 End of forward declarations
		 *============================================================*/

		 /** @brief	Helper function to convert JsonType to std::string.
		   */
		inline std::string to_string(JsonType type);

		/** @brief	Helper function to write JsonType to output stream.
		  */
		inline std::ostream& operator<<(std::ostream& out, JsonType type);

		/***********************************************************************
		 * @enum JsonType
		 * @brief Enum class used to identify the type of value stored in a json container.
		 ***********************************************************************/
		enum class JsonType {
			Null = 0,
			Integer = 1,
			Floating = 2,
			String = 3,
			Bool = 4,
			Array = 5,
			Object = 6
		};

		/** @brief	Helper function to print Json to output stream.
		  */
		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		inline std::basic_ostream<typename StringTy::value_type>& operator<<(std::basic_ostream<typename StringTy::value_type>& out, const Json<IntegerTy, FloatingTy, StringTy, BoolTy>& json);

		/** @brief	Helper function to print Json to a string.
		  * @note	This function is different from `Json::operator StringType(void) const`.
		  */
		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		inline StringTy to_string(const Json<IntegerTy, FloatingTy, StringTy, BoolTy>& json);

		/** @brief	Compare two Json containers.
		  * @return	`true` the two Json containers are equal (have the same structure and
		  *			the elements in one container are equal to their corresponding ones in the other container).
		  */
		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		bool operator==(const Json<IntegerTy, FloatingTy, StringTy, BoolTy>& json1, const Json<IntegerTy, FloatingTy, StringTy, BoolTy>& json2);

		/** @brief	Compare two Json containers.
		  * @return	`true` the two Json containers are unequal.
		  */
		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		bool operator!=(const Json<IntegerTy, FloatingTy, StringTy, BoolTy>& json1, const Json<IntegerTy, FloatingTy, StringTy, BoolTy>& json2);

		/***********************************************************************
		 * @class	Json
		 * @brief	Json class for reading/writing json files.
		 *
		 * @tparam	IntegerTy	The integer type. Default is `int`.
		 * @tparam	FloatingTy	The floating point type. Default is `float`.
		 * @tparam	StringTy	The string type. Default is `std::string`. It must meet
		 *						these requirements:
		 *						 - strict weak orderable (i.e. StringTy::operator< is properly implemented.)
		 *						 - defines `value_type` as its character type
		 * @tparam	BoolTy		The boolean type. Default is `bool`.
		 ***********************************************************************/
		template <
			class IntegerTy = int,
			class FloatingTy = float,
			class StringTy = std::string,
			class BoolTy = bool
		>
		class Json {

		public:

			/** @name	Type definitions and inline constants.
			  */
			//@{
			using value_type = Json;
			using size_type = std::size_t;
			using reference = value_type&;
			using const_reference = const value_type&;
			using pointer = value_type*;
			using const_pointer = const value_type*;
			using iterator = JsonIterator<IntegerTy, FloatingTy, StringTy, BoolTy>;
			using const_iterator = JsonConstIterator<IntegerTy, FloatingTy, StringTy, BoolTy>;
			using IntegerType = IntegerTy;
			using FloatingType = FloatingTy;
			using StringType = StringTy;
			using BoolType = BoolTy;
			using ArrayType = std::vector<Json>;
			using ObjectType = std::map<StringTy, Json>;
			using CharType = StringType::value_type;
			//@}

		public:

			/** @brief	Parse Json.
			  */
			template <class T>
			static Json parse(T&& src);

			/** @brief	Default constructor. Create a "null" json container.
			  */
			Json(void) : _type(JsonType::Null), _dummy{} {}

			/** @brief	Construct a json container from an integer.
			  */
			Json(IntegerType integer) : _type(JsonType::Integer), _integer(integer) {}

			/** @brief	Construct a json container from a floating point.
			  */
			Json(FloatingType floating) : _type(JsonType::Floating), _floating(floating) {}

			/** @brief	Construct a json container from a string.
			  */
			Json(const StringType& string) : _type(JsonType::String), _string(string) {}

			/** @brief	Construct a json container from a string.
			  */
			Json(StringType&& string) : _type(JsonType::String), _string(std::move(string)) {}

			/** @brief	Delegating constructor. Construct a string json.
			  * @note	Without this function, Json("123") will result in a Bool json.
			  */
			Json(const CharType* string) : _type(JsonType::String), _string(string) {}


			/** @brief	Construct a json container from a boolean.
			  */
			Json(BoolType boolean) : _type(JsonType::Bool), _bool(boolean) {}

			/** @brief	Construct a json container from an array.
			  */
			Json(const ArrayType& array) : _type(JsonType::Array), _array(array) {}

			/** @brief	Construct a json container from an array.
			  */
			Json(ArrayType&& array) : _type(JsonType::Array), _array(std::move(array)) {}

			/** @brief	Construct a json container from an initializer list.
			  */
			template <class T>
			Json(std::initializer_list<T> array) : _type(JsonType::Array), _array{} {
				this->_array.reserve(array.size());
				for (const T& v : array)
					this->_array.emplace_back(v);
			}
			
			/** @brief	Construct a json container from an object.
			  */
			Json(const ObjectType& object) : _type(JsonType::Object), _object(object) {}

			/** @brief	Construct a json container from an object.
			  */
			Json(ObjectType&& object) : _type(JsonType::Object), _object(std::move(object)) {}
			
			/** @brief	Construct a json container from an initializer list of pairs.
			  */
			template <class S, class T>
			Json(std::initializer_list<std::pair<S, T>> object) : _type(JsonType::Object), _object{} {
				for (const std::pair<S, T>& p : object)
					this->_object.emplace(p.first, p.second);
			}
			
			/** @brief	Construct a json container of the specified type with default value.
			  */
			explicit Json(JsonType type) : _type(JsonType::Null), _dummy{} {
				this->_create(type);
			}

			/** @brief	Copy constructor.
			  */
			Json(const Json& json) : _type(JsonType::Null), _dummy{} {
				this->_assign(json);
			}

			/** @brief	Move constructor.
			  */
			Json(Json&& json) : _type(JsonType::Null), _dummy{} {
				this->_assign(std::move(json));
			}

			/** @brief	Copy assignment.
			  */
			Json& operator=(const Json& json) {
				if (this != &json) {
					this->_reset();
					this->_assign(json);
				}
				return *this;
			}

			/** @brief	Move assignment.
			  */
			Json& operator=(Json&& json) {
				if (this != &json) {
					this->_reset();
					this->_assign(std::move(json));
				}
				return *this;
			}
			
			/** @brief	Destructor
			  */
			~Json(void) {
				this->_reset();
			}

			/** @brief	Swap with another Json container.
			  */
			void swap(Json& other) {
				std::swap(*this, other);
			}

			/** @brief	Create a json container of the specified type with default value.
			  *			The old values in this container will be cleared.
			  */
			void create(JsonType type) {
				this->_reset();
				this->_create(type);
			}

			/** @brief	Reset to null.
			  */
			void reset(void) {
				this->_reset();
			}

			/** @brief	Get the value type stored in the json container.
			  * @return	The value type of type `jjyou::io::JsonType`.
			  */
			JsonType type(void) const { return this->_type; }

			/** @brief	Check whether the Json container is Null.
			  * @return `true` if the Json container is Null.
			  */
			bool isNull(void) const { return this->_type == JsonType::Null; }

			/** @brief	Check whether the Json container is Null.
			  * @return `true` if the Json container is Null.
			  */
			bool empty(void) const { return this->_type == JsonType::Null; }

			/** @brief	Convert the Json to an integer.
			  *
			  *			This function is valid only if the Json's type is `JsonType::Integer`,
			  *			`JsonType::Floating`, or `JsonType::Bool`.
			  *			Otherwise, an exception of type std::out_of_range is thrown.
			  */
			explicit operator IntegerType(void) const;

			/** @brief	Convert the Json to a floating point number.
			  *
			  *			This function is valid only if the Json's type is `JsonType::Integer`,
			  *			`JsonType::Floating`, or `JsonType::Bool`.
			  *			Otherwise, an exception of type std::out_of_range is thrown.
			  */
			explicit operator FloatingType(void) const;

			/** @brief	Convert the Json to a string.
			  *
			  *			This function is valid only if the Json's type is `JsonType::String`.
			  *			Otherwise, an exception of type std::out_of_range is thrown.
			  */
			explicit operator StringType(void) const;

			/** @brief	Convert the Json to a bool.
			  *
			  *			This function is valid only if the Json's type is `JsonType::Integer`,
			  *			`JsonType::Floating`, or `JsonType::Bool`.
			  *			Otherwise, an exception of type std::out_of_range is thrown.
			  */
			explicit operator BoolType(void) const;

			/** @brief	Convert the Json to a std::vector.
			  *
			  *			This function is valid only if the Json's type is `JsonType::Array`,
			  *			and all the elements in the array (whose type are also Json) can be
			  *			converted to the template type `T`.
			  *			Otherwise, an exception of type std::out_of_range is thrown.
			  */
			template <class T>
			explicit operator std::vector<T>(void) const;

			/** @brief	Convert the Json to a std::map.
			  *
			  *			This function is valid only if the Json's type is `JsonType::Object`,
			  *			and all the elements in the object (whose type are also Json) can be
			  *			converted to the template type `T`.
			  *			Otherwise, an exception of type std::out_of_range is thrown.
			  */
			template <class T>
			explicit operator std::map<StringType, T>(void) const;

			/** @brief	Get the integer stored in the container. If the type does not match,
			  *			the behavior is undefined.
			  */
			IntegerType& integer(void) {
				return this->_integer;
			}
			const IntegerType& integer(void) const {
				return this->_integer;
			}

			/** @brief	Get the floating stored in the container. If the type does not match,
			  *			the behavior is undefined.
			  */
			FloatingType& floating(void) {
				return this->_floating;
			}
			const FloatingType& floating(void) const {
				return this->_floating;
			}

			/** @brief	Get the string stored in the container. If the type does not match,
			  *			the behavior is undefined.
			  */
			StringType& string(void) {
				return this->_string;
			}
			const StringType& string(void) const {
				return this->_string;
			}

			/** @brief	Get the bool stored in the container. If the type does not match,
			  *			the behavior is undefined.
			  */
			BoolType& boolean(void) {
				return this->_bool;
			}
			const BoolType& boolean(void) const {
				return this->_bool;
			}

			/** @brief	Get the array stored in the container. If the type does not match,
			  *			the behavior is undefined.
			  */
			ArrayType& array(void) {
				return this->_array;
			}
			const ArrayType& array(void) const {
				return this->_array;
			}

			/** @brief	Get the array stored in the container. If the type does not match,
			  *			the behavior is undefined.
			  */
			ObjectType& object(void) {
				return this->_object;
			}
			const ObjectType& object(void) const {
				return this->_object;
			}

			/** @brief	Get the reference to the value at a given position.
			  * 
			  * The index starts from 0. This function is valid only if the Json's
			  * type is `JsonType::Array`. Otherwise, the behavior is undefined.
			  * @return	The reference to the value at a given position.
			  */
			reference operator[](size_type pos) {
				return this->_array[pos];
			}

			/** @brief	Get the const reference to the value at a given position.
			  *
			  * The index starts from 0. This function is valid only if the Json's
			  * type is `JsonType::Array`. Otherwise, the behavior is undefined.
			  * @return	The const reference to the value at a given position.
			  */
			const_reference operator[](size_type pos) const {
				return this->_array[pos];
			}

			/** @brief	Get the reference to the value that is mapped to the given key,
			  *			performing an insertion if such key does not already exist.
			  *
			  * This function is valid only if the Json's type is `JsonType::Object`.
			  * Otherwise, the behavior is undefined.
			  * @return	The reference to the value that is mapped to the given key.
			  */
			reference operator[](const StringType& key) {
				return this->_object[key];
			}
			template <class T>
			reference operator[](const T* key) requires (std::is_same_v<T, CharType>) {
				return this->_object[StringType(key)];
			}

			/** @brief	Get the const reference to the value that is mapped to the given key,
			  *
			  * This function is valid only if the Json's type is `JsonType::Object`, and the Json
			  * contains the given key.
			  * Otherwise, the behavior is undefined.
			  * @return	The const reference to the value that is mapped to the given key.
			  */
			const_reference operator[](const StringType& key) const {
				return this->_object.find(key)->second;
			}
			template <class T>
			const_reference operator[](const T* key) const requires (std::is_same_v<T, CharType>) {
				return (*this)[StringType(key)];
			}

			/** @brief	Get the reference to the value at a given position, with bounds checking.
			  *
			  * The index starts from 0. This function is valid only if the Json's
			  * type is `JsonType::Array`. Otherwise, an exception of type std::out_of_range is thrown.
			  * @return	The reference to the value at a given position.
			  */
			reference at(size_type pos) {
				if (this->_type != JsonType::Array)
					throw std::out_of_range("`Json& Json::at(size_type)` is valid only if the Json container is an array.");
				return this->_array.at(pos);
			}

			/** @brief	Get the const reference to the value at a given position, with bounds checking.
			  *
			  * The index starts from 0. This function is valid only if the Json's
			  * type is `JsonType::Array`. Otherwise, an exception of type std::out_of_range is thrown.
			  * @return	The const reference to the value at a given position.
			  */
			const_reference at(size_type pos) const {
				if (this->_type != JsonType::Array)
					throw std::out_of_range("`const Json& Json::at(size_type) const` is valid only if the Json container is an array.");
				return this->_array.at(pos);
			}

			/** @brief	Get the reference to the value that is mapped to the given key, with bounds checking.
			  *
			  * This function is valid only if the Json's type is `JsonType::Object`.
			  * Otherwise, an exception of type std::out_of_range is thrown.
			  * @return	The reference to the value that is mapped to the given key.
			  */
			reference at(const StringType& key) {
				if (this->_type != JsonType::Object)
					throw std::out_of_range("`Json& Json::at(const StringType&)` is valid only if the Json container is an object.");
				return this->_object.at(key);
			}

			/** @brief	Get the const reference to the value that is mapped to the given key, with bounds checking.
			  *
			  * This function is valid only if the Json's type is `JsonType::Object`.
			  * Otherwise, an exception of type std::out_of_range is thrown.
			  * @return	The const reference to the value that is mapped to the given key.
			  */
			const_reference at(const StringType& key) const {
				if (this->_type != JsonType::Object)
					throw std::out_of_range("`const Json& Json::at(const StringType&) const` is valid only if the Json container is an object.");
				return this->_object.at(key);
			}

			/** @brief	Get the size of the json container.
			  *
			  * For null, the size is always 0.
			  * For integer/floating/string/bool, the size is always 1.
			  * For array, the size is the array length.
			  * For object, the size is the number of key&value pairs.
			  * @return	The size of the json container.
			  */
			std::size_t size(void) const;

			/** @name	Iterator-related methods.
			  * @brief	Get the iterator pointing to the specified position.
			  */
			//@{
			iterator begin(void);
			const_iterator begin(void) const;
			const_iterator cbegin(void) const;
			iterator end(void);
			const_iterator end(void) const;
			const_iterator cend(void) const;
			//@}

			/** @brief	Find the value that is mapped to the given key.
			  *
			  * This function is valid only if the Json's type is `JsonType::Object`.
			  * Otherwise, an exception of type std::out_of_range is thrown.
			  * @return	The iterator to the value that is mapped to the given key. If not found,
			  *			an iterator pointing to the end of container will be returned.
			  */
			iterator find(const StringType& key);

			/** @brief	Find the value that is mapped to the given key.
			  *
			  * This function is valid only if the Json's type is `JsonType::Object`.
			  * Otherwise, an exception of type std::out_of_range is thrown.
			  * @return	The iterator to the value that is mapped to the given key. If not found,
			  *			an iterator pointing to the end of container will be returned.
			  */
			const_iterator find(const StringType& key) const;

			friend std::basic_ostream<CharType>& operator<< <IntegerTy, FloatingTy, StringTy, BoolTy>(std::basic_ostream<CharType>& out, const Json& json);

			friend StringTy to_string<IntegerTy, FloatingTy, StringTy, BoolTy>(const Json& json);
			
			friend bool operator==<IntegerTy, FloatingTy, StringTy, BoolTy>(const Json& json1, const Json& json2);
			
			friend bool operator!=<IntegerTy, FloatingTy, StringTy, BoolTy>(const Json& json1, const Json& json2);

			template <class _IntegerTy, class _FloatingTy, class _StringTy, class _BoolTy>
			friend class Json;

		private:
			using InputAdapter = JsonInputAdapter<StringType>;
			using Token = JsonToken<IntegerType, FloatingType, StringType, BoolType>;
			using Lexer = JsonLexer<IntegerType, FloatingType, StringType, BoolType>;
			void _reset(void);
			void _assign(const Json& json);
			void _assign(Json&& json);
			void _create(JsonType type);
			void _print(std::basic_ostream<CharType>& out, int indent) const;
			static Json _parse(Lexer& lexer);
			JsonType _type;
			struct _Dummy {};
			union {
				_Dummy _dummy;
				IntegerType _integer;
				FloatingType _floating;
				StringType _string;
				BoolType _bool;
				ArrayType _array;
				ObjectType _object;
			};

		};

		/** @brief	Compare two iterators.
		  * @note	DO NOT compare two iterators belonging to different Json instances.
		  * @return	`true` if two iterators are considered to be equal.
		  */
		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		inline bool operator==(const JsonIterator<IntegerTy, FloatingTy, StringTy, BoolTy>& iter1, const JsonIterator<IntegerTy, FloatingTy, StringTy, BoolTy>& iter2);
		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		inline bool operator==(const JsonIterator<IntegerTy, FloatingTy, StringTy, BoolTy>& iter1, const JsonConstIterator<IntegerTy, FloatingTy, StringTy, BoolTy>& iter2);
		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		inline bool operator==(const JsonConstIterator<IntegerTy, FloatingTy, StringTy, BoolTy>& iter1, const JsonIterator<IntegerTy, FloatingTy, StringTy, BoolTy>& iter2);
		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		inline bool operator==(const JsonConstIterator<IntegerTy, FloatingTy, StringTy, BoolTy>& iter1, const JsonConstIterator<IntegerTy, FloatingTy, StringTy, BoolTy>& iter2);

		/** @brief	Compare two iterators.
		  * @note	DO NOT compare two iterators belonging to different Json instances.
		  * @return	`true` if two iterators are considered to be unequal.
		  */
		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		inline bool operator!=(const JsonIterator<IntegerTy, FloatingTy, StringTy, BoolTy>& iter1, const JsonIterator<IntegerTy, FloatingTy, StringTy, BoolTy>& iter2);
		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		inline bool operator!=(const JsonIterator<IntegerTy, FloatingTy, StringTy, BoolTy>& iter1, const JsonConstIterator<IntegerTy, FloatingTy, StringTy, BoolTy>& iter2);
		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		inline bool operator!=(const JsonConstIterator<IntegerTy, FloatingTy, StringTy, BoolTy>& iter1, const JsonIterator<IntegerTy, FloatingTy, StringTy, BoolTy>& iter2);
		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		inline bool operator!=(const JsonConstIterator<IntegerTy, FloatingTy, StringTy, BoolTy>& iter1, const JsonConstIterator<IntegerTy, FloatingTy, StringTy, BoolTy>& iter2);

		/***********************************************************************
		 * @class	JsonIterator
		 * @brief	Iterator type for Json class.
		 ***********************************************************************/
		template <
			class IntegerTy,
			class FloatingTy,
			class StringTy,
			class BoolTy
		>
		class JsonIterator {

		public:

			/** @name	Type definitions and inline constants.
			  */
			//@{
			using iterator_category = std::bidirectional_iterator_tag;
			using value_type = Json<IntegerTy, FloatingTy, StringTy, BoolTy>;
			using difference_type = std::ptrdiff_t;
			using pointer = value_type*;
			using reference = value_type&;
			//@}

			/** @name	Constructors, destructor and assignments.
			  */
			//@{
			JsonIterator(void) = default;
			JsonIterator(const JsonIterator&) = default;
			JsonIterator(JsonIterator&&) = default;
			~JsonIterator(void) = default;
			JsonIterator& operator=(const JsonIterator&) = default;
			JsonIterator& operator=(JsonIterator&&) = default;
			operator JsonConstIterator<IntegerTy, FloatingTy, StringTy, BoolTy>() const;
			//@}

			/** @brief	Increment the iterator.
			  * @return	Copy of the iterator before being incremented.
			  */
			JsonIterator operator++(int);

			/** @brief	Increment the iterator.
			  * @return	Reference of the iterator after being incremented.
			  */
			JsonIterator& operator++(void);

			/** @brief	Decrement the iterator.
			  * @return	Copy of the iterator before being deremented.
			  */
			JsonIterator operator--(int);

			/** @brief	Decrement the iterator.
			  * @return	Reference of the iterator after being decremented.
			  */
			JsonIterator& operator--(void);

			/** @brief	Fetch the current element.
			  * @return	Reference of the current element.
			  */
			reference operator*() const;

			/** @brief	Fetch the current element.
			  * @return	Pointer to the current element.
			  */
			pointer operator->() const;

			/** @brief	Fetch the current element's key. This function is valid only if
			  *			the Json's type is `JsonType::Object`. Otherwise, an exception
			  *			of type std::out_of_range is thrown.
			  * @return	The current element's key.
			  */
			StringTy key() const {
				if (this->pJson->type() == JsonType::Object)
					return this->objectIter->first;
				else
					throw std::out_of_range("`StringType JsonIterator::key() const` is valid only if the Json container is an object.");
			}

			/** @brief	Fetch the current element's value. Same as operator*.
			  * @return	Reference of the current element.
			  */
			reference value() const {
				return (**this);
			}

		private:

			pointer pJson = nullptr;
			int pos = 0;
			typename value_type::ObjectType::iterator objectIter;
			typename value_type::ArrayType::iterator arrayIter;
			JsonIterator(pointer pJson, int pos) : pJson(pJson), pos(pos) {}
			JsonIterator(pointer pJson, typename value_type::ObjectType::iterator objectIter) : pJson(pJson), objectIter(objectIter) {}
			JsonIterator(pointer pJson, typename value_type::ArrayType::iterator arrayIter) : pJson(pJson), arrayIter(arrayIter) {}
			friend class Json<IntegerTy, FloatingTy, StringTy, BoolTy>;
			friend class JsonConstIterator<IntegerTy, FloatingTy, StringTy, BoolTy>;
			friend bool operator==<IntegerTy, FloatingTy, StringTy, BoolTy>(const JsonIterator<IntegerTy, FloatingTy, StringTy, BoolTy>& iter1, const JsonIterator<IntegerTy, FloatingTy, StringTy, BoolTy>& iter2);
			friend bool operator==<IntegerTy, FloatingTy, StringTy, BoolTy>(const JsonIterator<IntegerTy, FloatingTy, StringTy, BoolTy>& iter1, const JsonConstIterator<IntegerTy, FloatingTy, StringTy, BoolTy>& iter2);
			friend bool operator==<IntegerTy, FloatingTy, StringTy, BoolTy>(const JsonConstIterator<IntegerTy, FloatingTy, StringTy, BoolTy>& iter1, const JsonIterator<IntegerTy, FloatingTy, StringTy, BoolTy>& iter2);
			friend bool operator==<IntegerTy, FloatingTy, StringTy, BoolTy>(const JsonConstIterator<IntegerTy, FloatingTy, StringTy, BoolTy>& iter1, const JsonConstIterator<IntegerTy, FloatingTy, StringTy, BoolTy>& iter2);

		};

		/***********************************************************************
		 * @class	JsonConstIterator
		 * @brief	Const iterator type for Json class.
		 ***********************************************************************/
		template <
			class IntegerTy,
			class FloatingTy,
			class StringTy,
			class BoolTy
		>
		class JsonConstIterator {

		public:

			/** @name	Type definitions and inline constants.
			  */
			//@{
			using iterator_category = std::bidirectional_iterator_tag;
			using value_type = Json<IntegerTy, FloatingTy, StringTy, BoolTy>;
			using difference_type = std::ptrdiff_t;
			using pointer = const value_type*;
			using reference = const value_type&;
			//@}

			/** @name	Constructors, destructor and assignments.
			  */
			//@{
			JsonConstIterator(void) = default;
			JsonConstIterator(const JsonConstIterator&) = default;
			JsonConstIterator(JsonConstIterator&&) = default;
			~JsonConstIterator(void) = default;
			JsonConstIterator& operator=(const JsonConstIterator&) = default;
			JsonConstIterator& operator=(JsonConstIterator&&) = default;
			//@}

			/** @brief	Increment the iterator.
			  * @return	Copy of the iterator before being incremented.
			  */
			JsonConstIterator operator++(int);

			/** @brief	Increment the iterator.
			  * @return	Reference of the iterator after being incremented.
			  */
			JsonConstIterator& operator++(void);

			/** @brief	Decrement the iterator.
			  * @return	Copy of the iterator before being deremented.
			  */
			JsonConstIterator operator--(int);

			/** @brief	Decrement the iterator.
			  * @return	Reference of the iterator after being decremented.
			  */
			JsonConstIterator& operator--(void);

			/** @brief	Fetch the current element.
			  * @return	Reference of the current element.
			  */
			reference operator*() const;

			/** @brief	Fetch the current element.
			  * @return	Pointer to the current element.
			  */
			pointer operator->() const;

			/** @brief	Fetch the current element's key. This function is valid only if
			  *			the Json's type is `JsonType::Object`. Otherwise, an exception
			  *			of type std::out_of_range is thrown.
			  * @return	The current element's key.
			  */
			StringTy key() const {
				if (this->pJson->type() == JsonType::Object)
					return this->objectIter->first;
				else
					throw std::out_of_range("`StringType JsonConstIterator::key() const` is valid only if the Json container is an object.");
			}

			/** @brief	Fetch the current element's value. Same as operator*.
			  * @return	Reference of the current element.
			  */
			reference value() const {
				return (**this);
			}

		private:

			pointer pJson = nullptr;
			int pos = 0;
			typename value_type::ObjectType::const_iterator objectIter;
			typename value_type::ArrayType::const_iterator arrayIter;
			JsonConstIterator(pointer pJson, int pos) : pJson(pJson), pos(pos) {}
			JsonConstIterator(pointer pJson, typename value_type::ObjectType::const_iterator objectIter) : pJson(pJson), objectIter(objectIter) {}
			JsonConstIterator(pointer pJson, typename value_type::ArrayType::const_iterator arrayIter) : pJson(pJson), arrayIter(arrayIter) {}
			friend class Json<IntegerTy, FloatingTy, StringTy, BoolTy>;
			friend class JsonIterator<IntegerTy, FloatingTy, StringTy, BoolTy>;
			friend bool operator==<IntegerTy, FloatingTy, StringTy, BoolTy>(const JsonIterator<IntegerTy, FloatingTy, StringTy, BoolTy>& iter1, const JsonIterator<IntegerTy, FloatingTy, StringTy, BoolTy>& iter2);
			friend bool operator==<IntegerTy, FloatingTy, StringTy, BoolTy>(const JsonIterator<IntegerTy, FloatingTy, StringTy, BoolTy>& iter1, const JsonConstIterator<IntegerTy, FloatingTy, StringTy, BoolTy>& iter2);
			friend bool operator==<IntegerTy, FloatingTy, StringTy, BoolTy>(const JsonConstIterator<IntegerTy, FloatingTy, StringTy, BoolTy>& iter1, const JsonIterator<IntegerTy, FloatingTy, StringTy, BoolTy>& iter2);
			friend bool operator==<IntegerTy, FloatingTy, StringTy, BoolTy>(const JsonConstIterator<IntegerTy, FloatingTy, StringTy, BoolTy>& iter1, const JsonConstIterator<IntegerTy, FloatingTy, StringTy, BoolTy>& iter2);

		};

	}

}



/*======================================================================
 | Implementation
 ======================================================================*/
/// @cond

namespace jjyou {

	namespace io {

		inline std::string to_string(JsonType type) {
			switch (type) {
			case JsonType::Null:
				return "Null";
			case JsonType::Integer:
				return "Integer";
			case JsonType::Floating:
				return "Floating";
			case JsonType::String:
				return "String";
			case JsonType::Bool:
				return "Bool";
			case JsonType::Array:
				return "Array";
			case JsonType::Object:
				return "Object";
			default:
				return "Unknown";
			}
		}

		inline std::ostream& operator<<(std::ostream& out, JsonType type) {
			return (out << to_string(type));
		}

		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		inline std::basic_ostream<typename StringTy::value_type>& operator<<(std::basic_ostream<typename StringTy::value_type>& out, const Json<IntegerTy, FloatingTy, StringTy, BoolTy>& json) {
			json._print(out, 0);
			return out;
		}

		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		inline StringTy to_string(const Json<IntegerTy, FloatingTy, StringTy, BoolTy>& json) {
			std::basic_stringstream<typename StringTy::value_type> sstream;
			sstream << json;
			return sstream.str();
		}

		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		bool operator==(const Json<IntegerTy, FloatingTy, StringTy, BoolTy>& json1, const Json<IntegerTy, FloatingTy, StringTy, BoolTy>& json2) {
			if (json1._type != json2._type) return false;
			switch (json1._type) {
			case JsonType::Null:
				return true;
			case JsonType::Integer:
				return json1._integer == json2._integer;
			case JsonType::Floating:
				return json1._floating == json2._floating;
			case JsonType::String:
				return json1._string == json2._string;
			case JsonType::Bool:
				return json1._bool == json2._bool;
			case JsonType::Array:
				return json1._array == json2._array;
			case JsonType::Object:
				return json1._object == json2._object;
			default:
				throw std::out_of_range("Invalid Json type.");
			}
		}

		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		bool operator!=(const Json<IntegerTy, FloatingTy, StringTy, BoolTy>& json1, const Json<IntegerTy, FloatingTy, StringTy, BoolTy>& json2) {
			return !(json1 == json2);
		}

		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		inline Json<IntegerTy, FloatingTy, StringTy, BoolTy>::operator IntegerTy(void) const {
			switch (this->_type) {
			case JsonType::Integer:
				return this->_integer;
			case JsonType::Floating:
				return static_cast<IntegerType>(this->_floating);
			case JsonType::Bool:
				return static_cast<IntegerType>(this->_bool);
			case JsonType::Null:
			case JsonType::String:
			case JsonType::Array:
			case JsonType::Object:
				throw std::out_of_range("`Json::operator IntegerType() const` is valid only if the Json container is a/an integer, floating, or bool.");
			default:
				throw std::out_of_range("Invalid Json type.");
			}
		}

		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		inline Json<IntegerTy, FloatingTy, StringTy, BoolTy>::operator FloatingTy(void) const {
			switch (this->_type) {
			case JsonType::Integer:
				return static_cast<FloatingType>(this->_integer);
			case JsonType::Floating:
				return this->_floating;
			case JsonType::Bool:
				return static_cast<FloatingType>(this->_bool);
			case JsonType::Null:
			case JsonType::String:
			case JsonType::Array:
			case JsonType::Object:
				throw std::out_of_range("`Json::operator FloatingType() const` is valid only if the Json container is a/an integer, floating, or bool.");
			default:
				throw std::out_of_range("Invalid Json type.");
			}
		}

		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		inline Json<IntegerTy, FloatingTy, StringTy, BoolTy>::operator StringTy(void) const {
			if (this->_type == JsonType::String)
				return this->_string;
			else
				throw std::out_of_range("`Json::operator StringType() const` is valid only if the Json container is a string.");
		}

		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		inline Json<IntegerTy, FloatingTy, StringTy, BoolTy>::operator BoolTy(void) const {
			switch (this->_type) {
			case JsonType::Integer:
				return static_cast<BoolType>(this->_integer);
			case JsonType::Floating:
				return static_cast<BoolType>(this->_floating);
			case JsonType::Bool:
				return this->_bool;
			case JsonType::Null:
			case JsonType::String:
			case JsonType::Array:
			case JsonType::Object:
				throw std::out_of_range("`Json::operator BoolType() const` is valid only if the Json container is a/an integer, floating, or bool.");
			default:
				throw std::out_of_range("Invalid Json type.");
			}
		}

		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy> template <class T>
		inline Json<IntegerTy, FloatingTy, StringTy, BoolTy>::operator std::vector<T>(void) const {
			if (this->_type == JsonType::Array) {
				std::vector<T> res; res.reserve(this->_array.size());
				for (const Json& v : this->_array)
					res.emplace_back(v);
				return res;
			}
			else
				throw std::out_of_range("`Json::operator std::vector<T>() const` is valid only if the Json container is an array.");
		}

		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy> template <class T>
		inline Json<IntegerTy, FloatingTy, StringTy, BoolTy>::operator std::map<StringTy, T>(void) const {
			if (this->_type == JsonType::Object) {
				std::map<StringType, T> res;
				for (auto cIter = this->_object.cbegin(); cIter != this->_object.cend(); ++cIter)
					res.emplace(cIter->first, cIter->second);
				return res;
			}
			else
				throw std::out_of_range("`Json::operator std::map<StringType, T>() const` is valid only if the Json container is an object.");
		}

		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		inline std::size_t Json<IntegerTy, FloatingTy, StringTy, BoolTy>::size(void) const {
			switch (this->_type) {
			case JsonType::Null:
				return 0ULL;
			case JsonType::Integer:
				return 1ULL;
			case JsonType::Floating:
				return 1ULL;
			case JsonType::String:
				return 1ULL;
			case JsonType::Bool:
				return 1ULL;
			case JsonType::Array:
				return this->_array.size();
			case JsonType::Object:
				return this->_object.size();
			default:
				throw std::out_of_range("Invalid Json type.");
			}
		}

		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		inline typename Json<IntegerTy, FloatingTy, StringTy, BoolTy>::iterator Json<IntegerTy, FloatingTy, StringTy, BoolTy>::begin(void) {
			switch (this->_type) {
			case JsonType::Null:
			case JsonType::Integer:
			case JsonType::Floating:
			case JsonType::String:
			case JsonType::Bool:
				return iterator(this, 0);
			case JsonType::Array:
				return iterator(this, this->_array.begin());
			case JsonType::Object:
				return iterator(this, this->_object.begin());
			default:
				throw std::out_of_range("Invalid Json type.");
			}
		}

		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		inline typename Json<IntegerTy, FloatingTy, StringTy, BoolTy>::const_iterator Json<IntegerTy, FloatingTy, StringTy, BoolTy>::begin(void) const {
			switch (this->_type) {
			case JsonType::Null:
			case JsonType::Integer:
			case JsonType::Floating:
			case JsonType::String:
			case JsonType::Bool:
				return const_iterator(this, 0);
			case JsonType::Array:
				return const_iterator(this, this->_array.cbegin());
			case JsonType::Object:
				return const_iterator(this, this->_object.cbegin());
			default:
				throw std::out_of_range("Invalid Json type.");
			}
		}

		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		inline typename Json<IntegerTy, FloatingTy, StringTy, BoolTy>::const_iterator Json<IntegerTy, FloatingTy, StringTy, BoolTy>::cbegin(void) const {
			return this->begin();
		}

		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		inline typename Json<IntegerTy, FloatingTy, StringTy, BoolTy>::iterator Json<IntegerTy, FloatingTy, StringTy, BoolTy>::end(void) {
			switch (this->_type) {
			case JsonType::Null:
				return iterator(this, 0);
			case JsonType::Integer:
			case JsonType::Floating:
			case JsonType::String:
			case JsonType::Bool:
				return iterator(this, 1);
			case JsonType::Array:
				return iterator(this, this->_array.end());
			case JsonType::Object:
				return iterator(this, this->_object.end());
			default:
				throw std::out_of_range("Invalid Json type.");
			}
		}

		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		inline typename Json<IntegerTy, FloatingTy, StringTy, BoolTy>::const_iterator Json<IntegerTy, FloatingTy, StringTy, BoolTy>::end(void) const {
			switch (this->_type) {
			case JsonType::Null:
				return const_iterator(this, 0);
			case JsonType::Integer:
			case JsonType::Floating:
			case JsonType::String:
			case JsonType::Bool:
				return const_iterator(this, 1);
			case JsonType::Array:
				return const_iterator(this, this->_array.cend());
			case JsonType::Object:
				return const_iterator(this, this->_object.cend());
			default:
				throw std::out_of_range("Invalid Json type.");
			}
		}

		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		inline typename Json<IntegerTy, FloatingTy, StringTy, BoolTy>::const_iterator Json<IntegerTy, FloatingTy, StringTy, BoolTy>::cend(void) const {
			return this->end();
		}

		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		inline typename Json<IntegerTy, FloatingTy, StringTy, BoolTy>::iterator Json<IntegerTy, FloatingTy, StringTy, BoolTy>::find(const StringType& key) {
			if (this->_type != JsonType::Object)
				throw std::out_of_range("`JsonIterator Json::find(const StringType&)` is valid only if the Json container is an object.");
			return iterator(this, this->_object.find(key));
		}

		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		inline typename Json<IntegerTy, FloatingTy, StringTy, BoolTy>::const_iterator Json<IntegerTy, FloatingTy, StringTy, BoolTy>::find(const StringType& key) const {
			if (this->_type != JsonType::Object)
				throw std::out_of_range("`JsonConstIterator Json::find(const StringType&) const` is valid only if the Json container is an object.");
			return const_iterator(this, this->_object.find(key));
		}

		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		inline void Json<IntegerTy, FloatingTy, StringTy, BoolTy>::_reset(void) {
			switch (this->_type) {
			case JsonType::Null:
				break;
			case JsonType::Integer:
				this->_integer.~IntegerType();
				break;
			case JsonType::Floating:
				this->_floating.~FloatingType();
				break;
			case JsonType::String:
				this->_string.~StringType();
				break;
			case JsonType::Bool:
				this->_bool.~BoolType();
				break;
			case JsonType::Array:
				this->_array.~ArrayType();
				break;
			case JsonType::Object:
				this->_object.~ObjectType();
				break;
			default:
				throw std::out_of_range("Invalid Json type.");
			}
			this->_type = JsonType::Null;
		}

		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		inline void Json<IntegerTy, FloatingTy, StringTy, BoolTy>::_assign(const Json& json) {
			switch (json._type) {
			case JsonType::Null:
				break;
			case JsonType::Integer:
				new (&this->_integer) IntegerType(json._integer);
				break;
			case JsonType::Floating:
				new (&this->_floating) FloatingType(json._floating);
				break;
			case JsonType::String:
				new (&this->_string) StringType(json._string);
				break;
			case JsonType::Bool:
				new (&this->_bool) BoolType(json._bool);
				break;
			case JsonType::Array:
				new (&this->_array) ArrayType(json._array);
				break;
			case JsonType::Object:
				new (&this->_object) ObjectType(json._object);
				break;
			default:
				throw std::out_of_range("Invalid Json type.");
			}
			this->_type = json._type;
		}

		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		inline void Json<IntegerTy, FloatingTy, StringTy, BoolTy>::_assign(Json&& json) {
			switch (json._type) {
			case JsonType::Null:
				break;
			case JsonType::Integer:
				new (&this->_integer) IntegerType(json._integer);
				json._integer.~IntegerType();
				break;
			case JsonType::Floating:
				new (&this->_floating) FloatingType(json._floating);
				json._floating.~FloatingType();
				break;
			case JsonType::String:
				new (&this->_string) StringType(std::move(json._string));
				json._string.~StringType();
				break;
			case JsonType::Bool:
				new (&this->_bool) BoolType(json._bool);
				json._bool.~BoolType();
				break;
			case JsonType::Array:
				new (&this->_array) ArrayType(std::move(json._array));
				json._array.~ArrayType();
				break;
			case JsonType::Object:
				new (&this->_object) ObjectType(std::move(json._object));
				json._object.~ObjectType();
				break;
			default:
				throw std::out_of_range("Invalid Json type.");
			}
			this->_type = json._type;
			json._type = JsonType::Null;
		}

		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		inline void Json<IntegerTy, FloatingTy, StringTy, BoolTy>::_create(JsonType type) {
			switch (type) {
			case JsonType::Null:
				break;
			case JsonType::Integer:
				new (&this->_integer) IntegerType();
				break;
			case JsonType::Floating:
				new (&this->_floating) FloatingType();
				break;
			case JsonType::String:
				new (&this->_string) StringType();
				break;
			case JsonType::Bool:
				new (&this->_bool) BoolType();
				break;
			case JsonType::Array:
				new (&this->_array) ArrayType();
				break;
			case JsonType::Object:
				new (&this->_object) ObjectType();
				break;
			default:
				throw std::out_of_range("Invalid Json type.");
			}
			this->_type = type;
		}

		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		void Json<IntegerTy, FloatingTy, StringTy, BoolTy>::_print(std::basic_ostream<CharType>& out, int indent) const {
			switch (this->_type) {
			case JsonType::Null:
				out << StringType(indent, '\t') << "null";
				break;
			case JsonType::Integer:
				out << StringType(indent, '\t') << this->_integer;
				break;
			case JsonType::Floating:
				out << StringType(indent, '\t') << this->_floating;
				break;
			case JsonType::String:
				out << StringType(indent, '\t') << '\"';
				for (const CharType& c : this->_string) {
					switch (c) {
					case static_cast<CharType>('\"'):
						out << '\\' << '\"';
						break;
					case static_cast<CharType>('\\'):
						out << '\\' << '\\';
						break;
					case static_cast<CharType>('/'):
						out << '\\' << '/';
						break;
					case static_cast<CharType>('\b'):
						out << '\\' << 'b';
						break;
					case static_cast<CharType>('\f'):
						out << '\\' << 'f';
						break;
					case static_cast<CharType>('\n'):
						out << '\\' << 'n';
						break;
					case static_cast<CharType>('\r'):
						out << '\\' << 'r';
						break;
					case static_cast<CharType>('\t'):
						out << '\\' << 't';
						break;
					default:
						out << c;
						break;
					}
				}
				out << '\"';
				break;
			case JsonType::Bool:
				out << StringType(indent, '\t') << std::boolalpha << this->_bool;
				break;
			case JsonType::Array:
				out << StringType(indent, '\t') << "[" << std::endl;
				for (auto iter = this->_array.cbegin(); iter != this->_array.cend();) {
					iter->_print(out, indent + 1);
					++iter;
					if (iter != this->_array.cend()) out << ",";
					out << std::endl;
				}
				out << StringType(indent, '\t') << "]";
				break;
			case JsonType::Object:
				out << StringType(indent, '\t') << "{" << std::endl;
				for (auto iter = this->_object.cbegin(); iter != this->_object.cend();) {
					out << StringType(indent + 1, '\t') << '\"' << iter->first << '\"' << " : ";
					if (iter->second._type == JsonType::Array || iter->second._type == JsonType::Object) {
						out << std::endl;
						iter->second._print(out, indent + 2);
					}
					else {
						iter->second._print(out, 0);
					}
					++iter;
					if (iter != this->_object.cend()) out << ",";
					out << std::endl;
				}
				out << StringType(indent, '\t') << "}" << std::endl;
				break;
			default:
				throw std::out_of_range("Invalid Json type.");
			}
		}

		#define JJYOU_IO_JSON_ITERATOR_EQUAL_IMPL(IterTy1, IterTy2)																							\
		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>																			\
		inline bool operator==(const IterTy1<IntegerTy, FloatingTy, StringTy, BoolTy>& iter1, const IterTy2<IntegerTy, FloatingTy, StringTy, BoolTy>& iter2)\
		{																																					\
			return																																			\
				(iter1.pJson == iter2.pJson &&																												\
					(																																		\
						(iter1.pJson == nullptr) ||																											\
						(iter1.pJson->type() == JsonType::Array && iter1.arrayIter == iter2.arrayIter) ||													\
						(iter1.pJson->type() == JsonType::Object && iter1.objectIter == iter2.objectIter) ||												\
						(iter1.pJson->type() != JsonType::Array && iter1.pJson->type() != JsonType::Object && iter1.pos == iter2.pos)						\
						)																																	\
					);																																		\
		}

		#define JJYOU_IO_JSON_ITERATOR_UNEQUAL_IMPL(IterTy1, IterTy2)																						\
		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>																			\
		inline bool operator!=(const IterTy1<IntegerTy, FloatingTy, StringTy, BoolTy>& iter1, const IterTy2<IntegerTy, FloatingTy, StringTy, BoolTy>& iter2)\
		{																																					\
			return !(iter1 == iter2);																														\
		}

		JJYOU_IO_JSON_ITERATOR_EQUAL_IMPL(JsonIterator, JsonIterator);
		JJYOU_IO_JSON_ITERATOR_EQUAL_IMPL(JsonIterator, JsonConstIterator);
		JJYOU_IO_JSON_ITERATOR_EQUAL_IMPL(JsonConstIterator, JsonIterator);
		JJYOU_IO_JSON_ITERATOR_EQUAL_IMPL(JsonConstIterator, JsonConstIterator);

		JJYOU_IO_JSON_ITERATOR_UNEQUAL_IMPL(JsonIterator, JsonIterator);
		JJYOU_IO_JSON_ITERATOR_UNEQUAL_IMPL(JsonIterator, JsonConstIterator);
		JJYOU_IO_JSON_ITERATOR_UNEQUAL_IMPL(JsonConstIterator, JsonIterator);
		JJYOU_IO_JSON_ITERATOR_UNEQUAL_IMPL(JsonConstIterator, JsonConstIterator);

		#undef JJYOU_IO_JSON_ITERATOR_EQUAL_IMPL
		#undef JJYOU_IO_JSON_ITERATOR_UNEQUAL_IMPL

		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		inline JsonIterator<IntegerTy, FloatingTy, StringTy, BoolTy>::operator JsonConstIterator<IntegerTy, FloatingTy, StringTy, BoolTy>() const {
			JsonConstIterator<IntegerTy, FloatingTy, StringTy, BoolTy> ret;
			ret.pJson = this->pJson;
			ret.pos = this->pos;
			ret.objectIter = this->objectIter;
			ret.arrayIter = this->arrayIter;
			return ret;
		}

		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		inline JsonIterator<IntegerTy, FloatingTy, StringTy, BoolTy> JsonIterator<IntegerTy, FloatingTy, StringTy, BoolTy>::operator++(int) {
			JsonIterator ret = *this;
			++(*this);
			return ret;
		}

		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		inline JsonIterator<IntegerTy, FloatingTy, StringTy, BoolTy>& JsonIterator<IntegerTy, FloatingTy, StringTy, BoolTy>::operator++(void) {
			if (this->pJson) {
				if (this->pJson->type() == JsonType::Array)
					++this->arrayIter;
				else if (this->pJson->type() == JsonType::Object)
					++this->objectIter;
				else
					++this->pos;
			}
			return *this;
		}

		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		inline JsonIterator<IntegerTy, FloatingTy, StringTy, BoolTy> JsonIterator<IntegerTy, FloatingTy, StringTy, BoolTy>::operator--(int) {
			JsonIterator ret = *this;
			--(*this);
			return ret;
		}

		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		inline JsonIterator<IntegerTy, FloatingTy, StringTy, BoolTy>& JsonIterator<IntegerTy, FloatingTy, StringTy, BoolTy>::operator--(void) {
			if (this->pJson) {
				if (this->pJson->type() == JsonType::Array)
					--this->arrayIter;
				else if (this->pJson->type() == JsonType::Object)
					--this->objectIter;
				else
					--this->pos;
			}
			return *this;
		}

		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		inline typename JsonIterator<IntegerTy, FloatingTy, StringTy, BoolTy>::reference JsonIterator<IntegerTy, FloatingTy, StringTy, BoolTy>::operator*() const {
			if (this->pJson->type() == JsonType::Array)
				return *this->arrayIter;
			else if (this->pJson->type() == JsonType::Object)
				return this->objectIter->second;
			else
				return *this->pJson;
		}

		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		inline typename JsonIterator<IntegerTy, FloatingTy, StringTy, BoolTy>::pointer JsonIterator<IntegerTy, FloatingTy, StringTy, BoolTy>::operator->() const {
			if (this->pJson->type() == JsonType::Array)
				return &*this->arrayIter;
			else if (this->pJson->type() == JsonType::Object)
				return &this->objectIter->second;
			else
				return this->pJson;
		}

		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		inline JsonConstIterator<IntegerTy, FloatingTy, StringTy, BoolTy> JsonConstIterator<IntegerTy, FloatingTy, StringTy, BoolTy>::operator++(int) {
			JsonConstIterator ret = *this;
			++(*this);
			return ret;
		}

		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		inline JsonConstIterator<IntegerTy, FloatingTy, StringTy, BoolTy>& JsonConstIterator<IntegerTy, FloatingTy, StringTy, BoolTy>::operator++(void) {
			if (this->pJson) {
				if (this->pJson->type() == JsonType::Array)
					++this->arrayIter;
				else if (this->pJson->type() == JsonType::Object)
					++this->objectIter;
				else
					++this->pos;
			}
			return *this;
		}

		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		inline JsonConstIterator<IntegerTy, FloatingTy, StringTy, BoolTy> JsonConstIterator<IntegerTy, FloatingTy, StringTy, BoolTy>::operator--(int) {
			JsonConstIterator ret = *this;
			--(*this);
			return ret;
		}

		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		inline JsonConstIterator<IntegerTy, FloatingTy, StringTy, BoolTy>& JsonConstIterator<IntegerTy, FloatingTy, StringTy, BoolTy>::operator--(void) {
			if (this->pJson) {
				if (this->pJson->type() == JsonType::Array)
					--this->arrayIter;
				else if (this->pJson->type() == JsonType::Object)
					--this->objectIter;
				else
					--this->pos;
			}
			return *this;
		}

		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		inline typename JsonConstIterator<IntegerTy, FloatingTy, StringTy, BoolTy>::reference JsonConstIterator<IntegerTy, FloatingTy, StringTy, BoolTy>::operator*() const {
			if (this->pJson->type() == JsonType::Array)
				return *this->arrayIter;
			else if (this->pJson->type() == JsonType::Object)
				return this->objectIter->second;
			else
				return *this->pJson;
		}

		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		inline typename JsonConstIterator<IntegerTy, FloatingTy, StringTy, BoolTy>::pointer JsonConstIterator<IntegerTy, FloatingTy, StringTy, BoolTy>::operator->() const {
			if (this->pJson->type() == JsonType::Array)
				return &*this->arrayIter;
			else if (this->pJson->type() == JsonType::Object)
				return &this->objectIter->second;
			else
				return this->pJson;
		}

		/*============================================================
		 *                        Json Parsing
		 *============================================================*/
		
		template <class StringTy>
		class JsonInputAdapter {
		private:
			using StringType = StringTy;
			using CharType = StringType::value_type;
			using StreamType = std::basic_istream<CharType>;
			JsonInputAdapter(StreamType& stream) : stream(&stream, [](StreamType*) -> void { return; }) {}
			JsonInputAdapter(const StringType& string) : stream(new std::basic_istringstream<CharType>(string)) {}
			JsonInputAdapter(StringType&& string) : stream(new std::basic_istringstream<CharType>(std::move(string))) {}
			JsonInputAdapter(const CharType* cstring) : stream(new std::basic_istringstream<CharType>(std::move(StringType(cstring)))) {}
			JsonInputAdapter(const std::filesystem::path& fileName) : stream(new std::basic_ifstream<CharType>(fileName)) { if (!std::reinterpret_pointer_cast<std::basic_ifstream<CharType>>(this->stream)->is_open()) throw std::runtime_error("Cannot open input json file \"" + fileName.string() + "\"."); }
			CharType get(void) {
				CharType res;
				if (!this->ungets.empty()) {
					res = this->ungets.top();
					this->ungets.pop();
				}
				else {
					res = static_cast<CharType>(this->stream->get());
				}
				return res;
			}
			CharType peek(void) {
				CharType res;
				if (!this->ungets.empty()) {
					res = this->ungets.top();
				}
				else {
					res = static_cast<CharType>(this->stream->peek());
				}
				return res;
			}
			void unget(CharType c) {
				this->ungets.push(c);
			}
			bool good(void) { return this->stream->good(); }
			bool eof(void) { return this->ungets.empty() && this->stream->eof(); }
			bool fail(void) { return this->stream->fail(); }
			bool bad(void) { return this->stream->bad(); }
			static std::string _toStdString(const StringType& str);
			std::shared_ptr<StreamType> stream;
			std::stack<CharType> ungets{};
			template <class _IntegerTy, class _FloatingTy, class _StringTy, class _BoolTy>
			friend class Json;
			template <class _IntegerTy, class _FloatingTy, class _StringTy, class _BoolTy>
			friend class JsonLexer;
		};
		
		template <>
		inline std::string JsonInputAdapter<std::string>::_toStdString(const StringType& str) {
			return str;
		}

		template <>
		inline std::string JsonInputAdapter<std::wstring>::_toStdString(const StringType& str) {
			std::string res; res.reserve(str.length() * sizeof(typename StringType::value_type));
			for (const CharType& c : str) {
				for (std::size_t i = 0; i < sizeof(CharType); ++i)
					if (reinterpret_cast<const char*>(&c)[i] != 0)
						res.push_back(reinterpret_cast<const char*>(&c)[i]);
			}
			return res;
		}

		template <>
		inline std::string JsonInputAdapter<std::u8string>::_toStdString(const StringType& str) {
			return std::string(reinterpret_cast<const char*>(str.c_str()));
		}

		template <>
		inline std::string JsonInputAdapter<std::u16string>::_toStdString(const StringType& str) {
			std::string res; res.reserve(str.length() * sizeof(typename StringType::value_type));
			for (const CharType& c : str) {
				for (std::size_t i = 0; i < sizeof(CharType); ++i)
					if (reinterpret_cast<const char*>(&c)[i] != 0)
						res.push_back(reinterpret_cast<const char*>(&c)[i]);
			}
			return res;
		}

		template <>
		inline std::string JsonInputAdapter<std::u32string>::_toStdString(const StringType& str) {
			std::string res; res.reserve(str.length() * sizeof(typename StringType::value_type));
			for (const CharType& c : str) {
				for (std::size_t i = 0; i < sizeof(CharType); ++i)
					if (reinterpret_cast<const char*>(&c)[i] != 0)
						res.push_back(reinterpret_cast<const char*>(&c)[i]);
			}
			return res;
		}

		enum class JsonTokenType {
			Unexpected = -1,
			End = 0,
			Null,
			Integer,
			Floating,
			String,
			Bool,
			Comma,
			Colon,
			Lbracket,
			Rbracket,
			Lbrace,
			Rbrace
		};

		inline std::string to_string(JsonTokenType type) {
			switch (type) {
			case JsonTokenType::Unexpected: return "Unexpected";
			case JsonTokenType::End: return "End";
			case JsonTokenType::Null: return "Null";
			case JsonTokenType::Integer: return "Integer";
			case JsonTokenType::Floating: return "Floating";
			case JsonTokenType::String: return "String";
			case JsonTokenType::Bool: return "Bool";
			case JsonTokenType::Comma: return "Comma";
			case JsonTokenType::Colon: return "Colon";
			case JsonTokenType::Lbracket: return "Lbracket";
			case JsonTokenType::Rbracket: return "Rbracket";
			case JsonTokenType::Lbrace: return "Lbrace";
			case JsonTokenType::Rbrace: return "Rbrace";
			default:
				throw std::out_of_range("Invalid Json token type.");
			}
		}

		inline std::ostream& operator<<(std::ostream& out, JsonTokenType type) {
			return (out << to_string(type));
		}

		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		class JsonToken {
		private:
			using IntegerType = IntegerTy;
			using FloatingType = FloatingTy;
			using StringType = StringTy;
			using BoolType = BoolTy;
			JsonTokenType type = JsonTokenType::End;
			std::variant<IntegerTy, FloatingTy, StringType, BoolTy> data{};
			std::size_t line = 0U;
			std::size_t col = 0U;
			std::size_t pos = 0U;
			JsonToken(void) = default;
			JsonToken(JsonTokenType type, std::size_t line, std::size_t col, std::size_t pos) : type(type), data(), line(line), col(col), pos(pos) {}
			friend class JsonLexer<IntegerTy, FloatingTy, StringTy, BoolTy>;
			friend class Json<IntegerTy, FloatingTy, StringTy, BoolTy>;
		};

		//https://www.json.org/json-en.html
		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		class JsonLexer {
		private:
			using IntegerType = IntegerTy;
			using FloatingType = FloatingTy;
			using StringType = StringTy;
			using BoolType = BoolTy;
			using CharType = StringTy::value_type;
			using InputAdapter = JsonInputAdapter<StringType>;
			using Token = JsonToken<IntegerType, FloatingType, StringType, BoolType>;
			static bool _isWhitespace(CharType c) {
				switch (c) {
				case static_cast<CharType>(' '):
				case static_cast<CharType>('\t'):
				case static_cast<CharType>('\r'):
				case static_cast<CharType>('\n'):
					return true;
				default:
					return false;
				}
			}
			static bool _isDigit(CharType c) {
				switch (c) {
				case static_cast<CharType>('0'):
				case static_cast<CharType>('1'):
				case static_cast<CharType>('2'):
				case static_cast<CharType>('3'):
				case static_cast<CharType>('4'):
				case static_cast<CharType>('5'):
				case static_cast<CharType>('6'):
				case static_cast<CharType>('7'):
				case static_cast<CharType>('8'):
				case static_cast<CharType>('9'):
					return true;
				default:
					return false;
				}
			}
			static bool _isHex(CharType c) {
				switch (c) {
				case static_cast<CharType>('0'):
				case static_cast<CharType>('1'):
				case static_cast<CharType>('2'):
				case static_cast<CharType>('3'):
				case static_cast<CharType>('4'):
				case static_cast<CharType>('5'):
				case static_cast<CharType>('6'):
				case static_cast<CharType>('7'):
				case static_cast<CharType>('8'):
				case static_cast<CharType>('9'):
				case static_cast<CharType>('A'):
				case static_cast<CharType>('a'):
				case static_cast<CharType>('B'):
				case static_cast<CharType>('b'):
				case static_cast<CharType>('C'):
				case static_cast<CharType>('c'):
				case static_cast<CharType>('D'):
				case static_cast<CharType>('d'):
				case static_cast<CharType>('E'):
				case static_cast<CharType>('e'):
				case static_cast<CharType>('F'):
				case static_cast<CharType>('f'):
					return true;
				default:
					return false;
				}
			}
			static std::size_t _hexToDecimal(CharType c) {
				switch (c) {
				case static_cast<CharType>('0'):
				case static_cast<CharType>('1'):
				case static_cast<CharType>('2'):
				case static_cast<CharType>('3'):
				case static_cast<CharType>('4'):
				case static_cast<CharType>('5'):
				case static_cast<CharType>('6'):
				case static_cast<CharType>('7'):
				case static_cast<CharType>('8'):
				case static_cast<CharType>('9'):
					return static_cast<std::size_t>(c - static_cast<CharType>('0'));
				case static_cast<CharType>('A'):
				case static_cast<CharType>('B'):
				case static_cast<CharType>('C'):
				case static_cast<CharType>('D'):
				case static_cast<CharType>('E'):
				case static_cast<CharType>('F'):
					return static_cast<std::size_t>(c - static_cast<CharType>('A') + static_cast<CharType>(10));
				case static_cast<CharType>('a'):
				case static_cast<CharType>('b'):
				case static_cast<CharType>('c'):
				case static_cast<CharType>('d'):
				case static_cast<CharType>('e'):
				case static_cast<CharType>('f'):
					return static_cast<std::size_t>(c - static_cast<CharType>('a') + static_cast<CharType>(10));
				default:
					return static_cast<std::size_t>(c);
				}
			}
			JsonLexer(InputAdapter& input) : input(input) {}
			Token get(void) {
				if (!this->ungets.empty()) {
					Token res = this->ungets.top();
					this->ungets.pop();
					return res;
				}
				while (!this->input.eof()) {
					CharType curr = this->input.peek();
					if (!this->_isWhitespace(curr))
						break;
					this->input.get();
					this->_updateTrace(curr);
				}
				if (this->input.eof())
					return Token(JsonTokenType::End, this->line, this->col, this->pos);
				CharType curr = this->input.peek();
				switch (curr) {
				case static_cast<CharType>('+'):
				case static_cast<CharType>('-'):
				case static_cast<CharType>('.'):
				case static_cast<CharType>('0'):
				case static_cast<CharType>('1'):
				case static_cast<CharType>('2'):
				case static_cast<CharType>('3'):
				case static_cast<CharType>('4'):
				case static_cast<CharType>('5'):
				case static_cast<CharType>('6'):
				case static_cast<CharType>('7'):
				case static_cast<CharType>('8'):
				case static_cast<CharType>('9'):
					return this->_number();
				case static_cast<CharType>('\"'):
					return this->_string();
				case static_cast<CharType>('t'): {
					Token res = this->_forward(JsonTokenType::Bool, 4,
						StringType{
							static_cast<CharType>('t'),
							static_cast<CharType>('r'),
							static_cast<CharType>('u'),
							static_cast<CharType>('e')
						}
					);
					res.data.template emplace<3>(static_cast<BoolType>(true));
					return res;
				}
				case static_cast<CharType>('f'): {
					Token res = this->_forward(JsonTokenType::Bool, 5,
						StringType{
							static_cast<CharType>('f'),
							static_cast<CharType>('a'),
							static_cast<CharType>('l'),
							static_cast<CharType>('s'),
							static_cast<CharType>('e')
						}
					);
					res.data.template emplace<3>(static_cast<BoolType>(false));
					return res;
				}
				case static_cast<CharType>('n'):
					return this->_forward(JsonTokenType::Null, 4,
						StringType{
							static_cast<CharType>('n'),
							static_cast<CharType>('u'),
							static_cast<CharType>('l'),
							static_cast<CharType>('l')
						}
					);
				case static_cast<CharType>(','):
					return this->_forward(JsonTokenType::Comma, 1, std::nullopt);
				case static_cast<CharType>(':'):
					return this->_forward(JsonTokenType::Colon, 1, std::nullopt);
				case static_cast<CharType>('['):
					return this->_forward(JsonTokenType::Lbracket, 1, std::nullopt);
				case static_cast<CharType>(']'):
					return this->_forward(JsonTokenType::Rbracket, 1, std::nullopt);
				case static_cast<CharType>('{'):
					return this->_forward(JsonTokenType::Lbrace, 1, std::nullopt);
				case static_cast<CharType>('}'):
					return this->_forward(JsonTokenType::Rbrace, 1, std::nullopt);
				default:
					return this->_forward(JsonTokenType::Unexpected, 1, std::nullopt);
				}
			}
			const Token& peek(void) {
				if (this->ungets.empty()) {
					this->ungets.push(this->get());
				}
				return this->ungets.top();
			}
			void unget(const Token& token) {
				this->ungets.push(token);
			}
			void unget(Token&& token) {
				this->ungets.push(std::move(token));
			}
			void _updateTrace(CharType c) {
				if (c == static_cast<CharType>('\n')) {
					++this->line;
					this->col = 0ULL;
				}
				else {
					++this->col;
				}
				++this->pos;
			}
			Token _forward(JsonTokenType type, std::size_t length, std::optional<StringType> expected) {
				Token res(type, this->line, this->col, this->pos);
				StringType string{};
				string.reserve(length);
				for (std::size_t i = 0; i < length; ++i) {
					if (this->input.eof()) {
						res.type = JsonTokenType::Unexpected;
						res.data.template emplace<2>(std::move(string));
						return res;
					}
					CharType curr = this->input.get(); this->_updateTrace(curr);
					string.push_back(curr);
					if (expected.has_value() && (*expected)[i] != curr) {
						res.type = JsonTokenType::Unexpected;
						res.data.template emplace<2>(std::move(string));
						return res;
					}
				}
				res.data.template emplace<2>(std::move(string));
				return res;
			}
			Token _number(void) {
				Token res(JsonTokenType::Integer, this->line, this->col, this->pos);
				StringType string{};
				CharType curr{};
				IntegerType integer = static_cast<IntegerType>(0);
				FloatingType floating = static_cast<FloatingType>(0.0);
				constexpr IntegerType baseInteger = static_cast<IntegerType>(10);
				constexpr FloatingType baseFloating = static_cast<FloatingType>(10.0);
				bool positive = true;
				if (this->input.peek() == static_cast<CharType>('-')) {
					curr = this->input.get(); this->_updateTrace(curr);
					string.push_back(curr);
					positive = false;
				}
				else if (this->input.peek() == static_cast<CharType>('+')) {
					curr = this->input.get(); this->_updateTrace(curr);
					string.push_back(curr);
					positive = true;
				}
				std::size_t numBeforeDecimal = 0ULL;
				while (!this->input.eof() && this->_isDigit(this->input.peek())) {
					++numBeforeDecimal;
					curr = this->input.get(); this->_updateTrace(curr);
					string.push_back(curr);
					integer = integer * baseInteger + static_cast<IntegerType>(curr - static_cast<CharType>('0'));
					floating = floating * baseFloating + static_cast<FloatingType>(curr - static_cast<CharType>('0'));
				}
				std::size_t numAfterDecimal = 0ULL;
				if (!this->input.eof() && this->input.peek() == static_cast<CharType>('.')) {
					// '.'
					curr = this->input.get(); this->_updateTrace(curr);
					string.push_back(curr);
					res.type = JsonTokenType::Floating;
					FloatingType decimal = static_cast<FloatingType>(1.0);
					while (!this->input.eof() && this->_isDigit(this->input.peek())) {
						++numAfterDecimal;
						decimal /= baseFloating;
						curr = this->input.get(); this->_updateTrace(curr);
						string.push_back(curr);
						floating += decimal * static_cast<FloatingType>(curr - static_cast<CharType>('0'));
					}
				}
				if (numBeforeDecimal == 0ULL && numAfterDecimal == 0ULL) {
					res.type = JsonTokenType::Unexpected;
					res.data.template emplace<2>(std::move(string));
					return res;
				}
				if (!positive) {
					integer *= static_cast<IntegerType>(-1);
					floating *= static_cast<FloatingType>(-1.0);
				}
				if (this->input.eof() || (
					this->input.peek() != static_cast<CharType>('e') && this->input.peek() != static_cast<CharType>('E'))) {
					if (res.type == JsonTokenType::Integer)
						res.data.template emplace<0>(integer);
					else
						res.data.template emplace<1>(floating);
					return res;
				}
				// 'e' or 'E'
				res.type = JsonTokenType::Floating;
				curr = this->input.get(); this->_updateTrace(curr);
				string.push_back(curr);
				if (this->input.eof()) {
					res.type = JsonTokenType::Unexpected;
					res.data.template emplace<2>(std::move(string));
					return res;
				}
				FloatingType exponential = static_cast<FloatingType>(0.0);
				bool exponentialPositive = true;
				if (this->input.peek() == static_cast<CharType>('-')) {
					curr = this->input.get(); this->_updateTrace(curr);
					string.push_back(curr);
					exponentialPositive = false;
				}
				else if (this->input.peek() == static_cast<CharType>('+')) {
					curr = this->input.get(); this->_updateTrace(curr);
					string.push_back(curr);
					exponentialPositive = true;
				}
				std::size_t numAfterExponentialBeforeDecimal = 0ULL;
				while (!this->input.eof() && this->_isDigit(this->input.peek())) {
					++numAfterExponentialBeforeDecimal;
					curr = this->input.get(); this->_updateTrace(curr);
					string.push_back(curr);
					exponential = exponential * baseFloating + static_cast<FloatingType>(curr - static_cast<CharType>('0'));
				}
				std::size_t numAfterExponentialAfterDecimal = 0ULL;
				if (!this->input.eof() && this->input.peek() == static_cast<CharType>('.')) {
					// '.'
					curr = this->input.get(); this->_updateTrace(curr);
					string.push_back(curr);
					FloatingType decimal = static_cast<FloatingType>(1.0);
					while (!this->input.eof() && this->_isDigit(this->input.peek())) {
						++numAfterExponentialAfterDecimal;
						decimal /= baseFloating;
						curr = this->input.get(); this->_updateTrace(curr);
						string.push_back(curr);
						exponential += decimal * static_cast<FloatingType>(curr - static_cast<CharType>('0'));
					}
				}
				if (numAfterExponentialBeforeDecimal == 0ULL && numAfterExponentialAfterDecimal == 0ULL) {
					res.type = JsonTokenType::Unexpected;
					res.data.template emplace<2>(std::move(string));
					return res;
				}
				if (!exponentialPositive) {
					exponential *= static_cast<FloatingType>(-1.0);
				}
				res.data.template emplace<1>(floating * std::pow<FloatingType>(baseFloating, exponential));
				return res;
			}
			Token _string(void) {
				StringType string{};
				StringType value{};
				bool findEnd = false;
				Token res(JsonTokenType::String, this->line, this->col, this->pos);
				CharType curr = this->input.get(); this->_updateTrace(curr); // '\"'
				string.push_back(curr);
				while (!this->input.eof()) {
					curr = this->input.get(); this->_updateTrace(curr);
					string.push_back(curr);
					if (curr == '\"') {
						findEnd = true;
						break;
					}
					else if (curr == '\\') {
						if (this->input.eof()) {
							res.type = JsonTokenType::Unexpected;
							res.data.template emplace<2>(std::move(string));
							return res;
						}
						curr = this->input.get(); this->_updateTrace(curr);
						string.push_back(curr);
						switch (curr) {
						case static_cast<CharType>('\"'):
						case static_cast<CharType>('\\'):
						case static_cast<CharType>('/'):
							value.push_back(curr);
							break;
						case static_cast<CharType>('b'):
							value.push_back(static_cast<CharType>('\b'));
							break;
						case static_cast<CharType>('f'):
							value.push_back(static_cast<CharType>('\f'));
							break;
						case static_cast<CharType>('n'):
							value.push_back(static_cast<CharType>('\n'));
							break;
						case static_cast<CharType>('r'):
							value.push_back(static_cast<CharType>('\r'));
							break;
						case static_cast<CharType>('t'):
							value.push_back(static_cast<CharType>('\t'));
							break;
						case static_cast<CharType>('u'): {
							CharType hex[4] = {};
							for (std::size_t i = 0; i < 4; ++i) {
								if (this->input.eof()) {
									res.type = JsonTokenType::Unexpected;
									res.data.template emplace<2>(std::move(string));
									return res;
								}
								hex[i] = this->input.get(); this->_updateTrace(hex[i]);
								string.push_back(hex[i]);
								if (!this->_isHex(hex[i])) {
									res.type = JsonTokenType::Unexpected;
									res.data.template emplace<2>(std::move(string));
									return res;
								}
							}
							if constexpr (sizeof(CharType) == 1) {
								value.push_back(static_cast<CharType>(this->_hexToDecimal(hex[0]) * 16 + this->_hexToDecimal(hex[1])));
								value.push_back(static_cast<CharType>(this->_hexToDecimal(hex[2]) * 16 + this->_hexToDecimal(hex[3])));
							}
							else {
								value.push_back(static_cast<CharType>(((this->_hexToDecimal(hex[0]) * 16 + this->_hexToDecimal(hex[1])) * 16 + this->_hexToDecimal(hex[2])) * 16 + this->_hexToDecimal(hex[3])));
							}
							break;
						}
						default:
							res.type = JsonTokenType::Unexpected;
							res.data.template emplace<2>(std::move(string));
							return res;
						}
					}
					else {
						value.push_back(curr);
					}
				}
				if (findEnd) {
					res.data.template emplace<2>(std::move(value));
				}
				else {
					res.type = JsonTokenType::Unexpected;
					res.data.template emplace<2>(std::move(string));
				}
				return res;
			}
			InputAdapter& input;
			std::size_t line = 0UL;
			std::size_t col = 0UL;
			std::size_t pos = 0UL;
			std::stack<Token> ungets{};
			friend class Json<IntegerTy, FloatingTy, StringTy, BoolTy>;
		};

		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy> template <class T>
		inline Json<IntegerTy, FloatingTy, StringTy, BoolTy> Json<IntegerTy, FloatingTy, StringTy, BoolTy>::parse(T&& src) {
			InputAdapter inputAdapter(std::forward<T>(src));
			Lexer lexer(inputAdapter);
			return Json::_parse(lexer);
		}

		template <class IntegerTy, class FloatingTy, class StringTy, class BoolTy>
		Json<IntegerTy, FloatingTy, StringTy, BoolTy> Json<IntegerTy, FloatingTy, StringTy, BoolTy>::_parse(Lexer& lexer) {
			// For throwing exceptions
			auto error = []<class... Args>(const Token & token, Args&&... args) {
				std::basic_stringstream<CharType> sstream;
				sstream << "[Json Parser] ln:" << (token.line + 1U) << ", col:" << (token.col + 1U) << ", pos:" << (token.pos + 1U) << " ";
				((sstream << std::forward<Args>(args)), ...);
				throw std::runtime_error(InputAdapter::_toStdString(sstream.str()));
			};
			Token token = lexer.get();
			switch (token.type) {
			case JsonTokenType::Null:/* Null */
			{
				Json json{};
				return json;
			}
			case JsonTokenType::Integer:/* Integer */
			{
				Json json(std::get<0>(token.data));
				return json;
			}
			case JsonTokenType::Floating:/* Floating */
			{
				Json json(std::get<1>(token.data));
				return json;
			}
			case JsonTokenType::String:/* String */
			{
				Json json(std::get<2>(token.data));
				return json;
			}
			case JsonTokenType::Bool:/* Bool */
			{
				Json json(std::get<3>(token.data));
				return json;
			}
			case JsonTokenType::Lbracket: /* Array */
			{
				Json json(JsonType::Array);
				while ((token = lexer.peek()).type != JsonTokenType::Rbracket) {
					if (token.type == JsonTokenType::End)
						error(token, "Unexpected EOF.");
					if (token.type == JsonTokenType::Unexpected)
						error(token, "Unexpected characters \"", std::get<2>(token.data), "\".");
					if (!json._array.empty()) {
						lexer.get();
						if (token.type != JsonTokenType::Comma)
							error(token, "Missing comma to separate elements in an array.");
					}
					json._array.push_back(Json::_parse(lexer));
				}
				lexer.get();
				return json;
			}
			case JsonTokenType::Lbrace: /* Object */
			{
				Json json(JsonType::Object);
				while ((token = lexer.peek()).type != JsonTokenType::Rbrace) {
					if (token.type == JsonTokenType::End)
						error(token, "Unexpected EOF.");
					if (token.type == JsonTokenType::Unexpected)
						error(token, "Unexpected characters \"", std::get<2>(token.data), "\".");
					if (!json._object.empty()) {
						token = lexer.get();
						if (token.type == JsonTokenType::End)
							error(token, "Unexpected EOF.");
						if (token.type == JsonTokenType::Unexpected)
							error(token, "Unexpected characters \"", std::get<2>(token.data), "\".");
						if (token.type != JsonTokenType::Comma)
							error(token, "Missing comma to separate elements in an object.");
					}
					token = lexer.get(); // Key
					if (token.type == JsonTokenType::End)
						error(token, "Unexpected EOF.");
					if (token.type == JsonTokenType::Unexpected)
						error(token, "Unexpected characters \"", std::get<2>(token.data), "\".");
					if (token.type != JsonTokenType::String)
						error(token, "Object's key must be a string.");
					StringType key = std::get<2>(token.data);
					token = lexer.get();
					if (token.type == JsonTokenType::End)
						error(token, "Unexpected EOF.");
					if (token.type == JsonTokenType::Unexpected)
						error(token, "Unexpected characters \"", std::get<2>(token.data), "\".");
					if (token.type != JsonTokenType::Colon)
						error(token, "Missing colon to separate key and value.");
					json._object.emplace(key, Json::_parse(lexer));
				}
				lexer.get();
				return json;
			}
			case JsonTokenType::End:
			{
				error(token, "Unexpected EOF.");
			}
			case JsonTokenType::Unexpected:
			default:
			{
				error(token, "Unexpected characters \"", std::get<2>(token.data), "\".");
			}
			}
			return Json{};
		}
	}

}

/// @endcond

#endif /* jjyou_io_Json_hpp */