/***********************************************************************
 * @file	ArgParser.hpp
 * @author	jjyou
 * @date	2024-2-12
 * @brief	This file implements ArgParser class.
***********************************************************************/
#ifndef jjyou_io_ArgParser_hpp
#define jjyou_io_ArgParser_hpp

#include <iostream>
#include <string>

namespace jjyou {

	namespace io {

		/** @brief	ArgParser class for parsing program arguments.
		  *
		  *			The usage is similar to that of Python argparse package.
		  */
		class ArgParser {

		public:

			ArgParser& addArgument() {
				return *this;
			}
		};

	}

}

#endif /* jjyou_io_ArgParser_hpp */