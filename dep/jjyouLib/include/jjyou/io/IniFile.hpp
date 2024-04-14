/***********************************************************************
 * @file	IniFile.hpp
 * @author	jjyou
 * @date	2023-5-22
 * @brief	This file implements IniFile class.
***********************************************************************/
#ifndef jjyou_io_IniFile_hpp
#define jjyou_io_IniFile_hpp

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include "../utils.hpp"

namespace jjyou {
	namespace io {

		/***********************************************************************
		 * @class IniFile
		 * @brief Ini file class.
		 *
		 * This class provides C++ API for reading and writing ini file.
		 ***********************************************************************/
		class IniFile {
		public:
			/** @brief Construct an empty ini file.
			  */
			IniFile(void);

			/** @brief Destructor.
			  */
			~IniFile(void);

			/** @brief Construct and load from the specified file.
			  *
			  * @param fileName The name of the file.
			  */
			IniFile(const std::string& fileName);

			/** @brief Load from the specified file.
			  *
			  * The instance will NOT be cleared before loading new contents.
			  * Please call IniFile::clear if you want to clear this instance.
			  *
			  * @param fileName The name of the file.
			  * @return `true` if the file has been successfully opened and read.
			  */
			bool loadIni(const std::string& fileName);

			/** @brief Clear all content in the instance.
			  */
			void clear(void);

			/** @brief Add a new element.
			  *
			  * If the element already exists,
			  * its value will be replaced by the new value.
			  *
			  * @param key		The key of the element.
			  * @param value	The value of the element.
			  */
			template<class T> void set(const std::string& key, const T& value);

			/** @brief Add a new `bool` element.
			  *
			  * If the element already exists,
			  * its value will be replaced by the new value.
			  *
			  * @param key		The key of the element.
			  * @param value	The value of the element.
			  */
			template<> void set<bool>(const std::string& key, const bool& value);

			/** @brief Get the specified element.
			  *
			  * If the element does not exists, this function will simply
			  * return \p defaultValue
			  *
			  * @param key			The key of the element.
			  * @param defaultValue	Default value.
			  * @return				The value of the specified element if it exists,
			  *						otherwise \p defaultValue.
			  */
			template<class T> T get(const std::string& key, const T& defaultValue) const;

			/** @brief Get the specified `string` element.
			  *
			  * If the element does not exists, this function will simply
			  * return \p defaultValue
			  *
			  * @param key			The key of the element.
			  * @param defaultValue	Default value.
			  * @return				The value of the specified element if it exists,
			  *						otherwise \p defaultValue.
			  */
			template<> std::string get<std::string>(const std::string& key, const std::string& defaultValue) const;

			/** @brief Get the specified `bool` element.
			  *
			  * If the element does not exists, this function will simply
			  * return \p defaultValue
			  *
			  * @param key			The key of the element.
			  * @param defaultValue	Default value.
			  * @return				The value of the specified element if it exists,
			  *						otherwise \p defaultValue.
			  */
			template<> bool get<bool>(const std::string& key, const bool& defaultValue) const;

			/** @brief Get the specified `const char*` element.
			  *
			  * If the element does not exists, this function will simply
			  * return \p defaultValue
			  *
			  * @param key			The key of the element.
			  * @param defaultValue	Default value.
			  * @return				The value of the specified element if it exists,
			  *						otherwise \p defaultValue.
			  */
			template<> const char* get<const char*>(const std::string& key, const char* const& defaultValue) const;

		private:
			std::map<std::string, std::string> content;
			friend inline std::ostream& operator<<(std::ostream& out, const jjyou::io::IniFile& iniFile);
		};

		/** @brief Write jjyou::io::IniFile to text stream.
		  */
		inline std::ostream& operator<<(std::ostream& out, const jjyou::io::IniFile& iniFile);

	}
}

/*======================================================================
 | Implementation
 ======================================================================*/
/// @cond

namespace jjyou {
	namespace io {

		inline IniFile::IniFile(void) : content() {}

		inline IniFile::~IniFile(void) {}

		inline IniFile::IniFile(const std::string& fileName) : content() {
			this->loadIni(fileName);
		}

		inline bool IniFile::loadIni(const std::string& fileName) {
			std::ifstream in(fileName);
			if (!in.is_open()) {
				std::cout << "[jjyou::io::IniFile] File \"" << fileName << "\" not found, use default parameters" << std::endl;
				return false;
			}
			while (in) {
				std::string line;
				std::getline(in, line);
				if (line.empty() || line[0] == '#' || line[0] == ';') continue;

				size_t eqPos = line.find_first_of("=");
				if (eqPos == std::string::npos || eqPos == 0) {
					continue;
				}

				std::string key, value;
				key = line.substr(0, eqPos);
				utils::trim(key);
				if (!key.empty() && key.front() == '\"' && key.back() == '\"')
					key = key.substr(1, key.length() - 2);
				value = line.substr(eqPos + 1);
				utils::trim(value);
				if (!value.empty() && value.front() == '\"' && value.back() == '\"')
					value = value.substr(1, value.length() - 2);
				this->content[key] = value;
			}
			return true;
		}

		inline void IniFile::clear(void) {
			this->content.clear();
		}

		template<class T> inline void IniFile::set(const std::string& key, const T& value) {
			std::stringstream ss;
			ss << value;
			this->content[key] = ss.str();
		}

		template<> inline void IniFile::set<bool>(const std::string& key, const bool& value) {
			std::stringstream ss;
			ss << std::boolalpha << value;
			std::string valueStr;
			ss >> valueStr;
			this->content[key] = valueStr;
		}

		template<class T> inline T IniFile::get(const std::string& key, const T& defaultValue) const {
			std::map<std::string, std::string>::const_iterator itr = this->content.find(key);
			if (itr != this->content.end()) {
				std::stringstream ss;
				ss << itr->second;
				T ret;
				ss >> ret;
				return ret;
			}
			return defaultValue;
		}

		template<> inline std::string IniFile::get<std::string>(const std::string& key, const std::string& defaultValue) const {
			std::map<std::string, std::string>::const_iterator itr = this->content.find(key);
			if (itr != this->content.end()) {
				return itr->second;
			}
			return defaultValue;
		}

		template<> inline bool IniFile::get<bool>(const std::string& key, const bool& defaultValue) const {
			std::map<std::string, std::string>::const_iterator itr = this->content.find(key);
			if (itr != this->content.end()) {
				std::stringstream ss;
				ss << itr->second;
				bool ret;
				ss >> std::boolalpha >> ret;
				return ret;
			}
			return defaultValue;
		}

		template<> inline const char* IniFile::get<const char*>(const std::string& key, const char* const& defaultValue) const {
			std::map<std::string, std::string>::const_iterator itr = this->content.find(key);
			if (itr != this->content.end()) {
				return itr->second.c_str();
			}
			return defaultValue;
		}
	
		inline std::ostream& operator<<(std::ostream& out, const jjyou::io::IniFile& iniFile) {
			std::map<std::string, std::string>::const_iterator itr = iniFile.content.cbegin();
			out << "========jjyou::io::IniFile========" << std::endl;
			while (itr != iniFile.content.cend()) {
				out << "    " << itr->first << " = " << itr->second << std::endl;
				itr++;
			}
			out << "==================================" << std::endl;
			return out;
		}
	}
}


/// @endcond

#endif /* jjyou_io_IniFile_hpp */