/***********************************************************************
 * @file	PlyFile.hpp
 * @author	jjyou
 * @date	2024-1-14
 * @brief	This file implements PlyFile class.
***********************************************************************/
#ifndef jjyou_io_PlyFile_hpp
#define jjyou_io_PlyFile_hpp

#include <vector>
#include <string>
#include <array>
#include <type_traits>
#include <bit>
#include <Eigen/Eigen>
#include <iostream>
#include <fstream>
#include "../utils.hpp"

namespace jjyou {
	namespace io {

		/** @enum PlyFormat
		  * @brief Format of ply file storage.
		  *
		  * Supported format: ascii, binary little endian, binary big endian.
		  */
		enum class PlyFormat {
			ascii,
			binary_little_endian,
			binary_big_endian,
		};

		/***********************************************************************
		 * @class PlyFile
		 * @brief Ply file class.
		 * 
		 * This class provides C++ API for reading and writing ply file.
		 * @tparam VertexTy	Element type for vertex / vertex normal / face normal.
		 *					It must be one of char, unsigned char, short,
		 *					unsigned short, int, unsigned int, float, double.
		 * @tparam ColorTy	Element type for vertex color / edge color / face color.
		 *					It must be one of char, unsigned char, short,
		 *					unsigned short, int, unsigned int, float, double.
		 *                  For integral types, the valid range of color is 0~255;
		 *                  For floating point types, the valid range of color is 0.0~1.0.
		 * @tparam HasAlpha	Whether there is an alpha channel in 
		 *					vertex color / edge color / face color.
		 ***********************************************************************/
		template <class VertexTy, class ColorTy, bool HasAlpha>
		class PlyFile {

			static_assert(
				std::is_same_v<VertexTy, char> ||
				std::is_same_v<VertexTy, unsigned char> ||
				std::is_same_v<VertexTy, short> ||
				std::is_same_v<VertexTy, unsigned short> ||
				std::is_same_v<VertexTy, int> ||
				std::is_same_v<VertexTy, unsigned int> ||
				std::is_same_v<VertexTy, float> ||
				std::is_same_v<VertexTy, double>,
				"VertexTy must be one of char, unsigned char, short, unsigned short, int, unsigned int, float, double."
			);

			static_assert(
				std::is_same_v<ColorTy, char> ||
				std::is_same_v<ColorTy, unsigned char> ||
				std::is_same_v<ColorTy, short> ||
				std::is_same_v<ColorTy, unsigned short> ||
				std::is_same_v<ColorTy, int> ||
				std::is_same_v<ColorTy, unsigned int> ||
				std::is_same_v<ColorTy, float> ||
				std::is_same_v<ColorTy, double>,
				"ColorTy must be one of char, unsigned char, short, unsigned short, int, unsigned int, float, double."
			);

		public:

			/** @brief Type for vertex / vertex normal / face normal.
			  */
			using VertexType = Eigen::Vector<VertexTy, 3>;

			/** @brief Type for color / edge color / face color.
			  */
			using ColorType = std::conditional_t<
				HasAlpha,
				Eigen::Vector<ColorTy, 4>,
				Eigen::Vector<ColorTy, 3>
			>;

			/** @brief Type for edge.
			  */
			using EdgeType = std::array<int, 2>;

			/** @brief Type for face.
			  */
			using FaceType = std::vector<int>;

			/** @brief Format of file storage.
			  * 
			  * Supported format: ascii, binary little endian, binary big endian.
			  */
			PlyFormat format;

			/** @brief Major version of the file.
			  * 
			  * 1 is the only version currently in use.
			  */
			static inline int majorVersion = 1;

			/** @brief Minor version of the file.
			  * 
			  * 0 is the only version currently in use.
			  */
			static inline int minorVersion = 0;

			/** @brief Comments read from / to write to the file.
			  * 
			  * Each element in the vector is a single-line comment.
			  * Please make sure that there is no line breaks (`"\n"` or `"\r"`) in it before writing to files.
			  */
			std::vector<std::string> comment;

			/** @brief Vertices.
			  */
			std::vector<VertexType> vertex;

			/** @brief Vertex normals.
			  * 
			  * Vertex normals will be exported only if
			  * `vertexNormal.size() == vertex.size()`.
			  * Leave it empty if there is no vertex normal.
			  */
			std::vector<VertexType> vertexNormal;

			/** @brief Vertex colors.
			  *
			  * Vertex colors will be exported only if
			  * `vertexColor.size() == vertex.size()`.
			  * Leave it empty if there is no vertex color.
			  */
			std::vector<ColorType> vertexColor;

			/** @brief Edges.
			  *
			  * Each edge is represented by the indices of
			  * two vertices (starting from 0).
			  * Leave it empty if there is no edge.
			  */
			std::vector<EdgeType> edge;

			/** @brief Edge colors.
			  *
			  * Edge colors will be exported only if
			  * `edgeColor.size() == edge.size()`.
			  * Leave it empty if there is no edge color.
			  */
			std::vector<ColorType> edgeColor;

			/** @brief Faces.
			  *
			  * Each face is represented by a list of indices
			  * of vertices (starting from 0).
			  * Leave it empty if there is no face.
			  */
			std::vector<FaceType> face;

			/** @brief Face normals.
			  *
			  * Face normals will be exported only if
			  * `faceNormal.size() == face.size()`.
			  * Leave it empty if there is no face normal.
			  */
			std::vector<VertexType> faceNormal;

			/** @brief Face colors.
			  *
			  * Face colors will be exported only if
			  * `faceColor.size() == face.size()`.
			  * Leave it empty if there is no face color.
			  */
			std::vector<ColorType> faceColor;

			/** @brief Default constructor.
			  *
			  * PlyFile::format will be set to PlyFormat::ascii.
			  * Others variables will be empty.
			  */
			PlyFile(void);

			/** @brief Destructor.
			  */
			~PlyFile(void);

			/** @brief Export to the specified file.
			  * 
			  * The format of data storage is specified by PlyFile::format.
			  * 
			  * @param fileName	The name of the file.
			  * @return `true` if data has been successfully written to the file.
			  */
			bool write(const std::string& fileName);

			/** @brief Import from the specified file.
			  * 
			  * Accepted element name:
			  *     vertex, edge, face.
			  * Accepted property name for vertex:
			  *     x, y, z, red, green, blue, alpha, nx, ny, nz.
			  * Accepted property name for edge:
			  *     vertex1, vertex2, red, green, blue.
			  * Accepted property name for face:
			  *     vertex_index, vertex_indices, red, green, blue, alpha, nx, ny, nz.
			  * Other elements or properties will be discarded.
			  * If the types of elements in the file does not match
			  * the corresponding template parameters `VertexTy` and `ColorTy`,
			  * type casting will take place. If the colors in the file have
			  * alpha channels while the template argument `HasAlpha` is `false`,
			  * the alpha channels will be discarded. If the colors in the file
			  * do not have alpha channels while the template argument `HasAlpha`
			  * is `true`, the alpha channels will be filled with 1.0 for floating
			  * types or 255 for integer types.
			  * PlyFile::format will be set to the corresponding storage format.
			  *
			  * @param fileName	The name of the file.
			  * @return `true` if the file has been successfully opened and read.
			  */
			bool read(const std::string& fileName);

			/** @brief Reset to default.
			  *
			  * PlyFile::format will be set to PlyFormat::ascii.
			  * Others variables will be empty.
			  */
			void reset(void);

		private:
			bool write(std::ostream& out);
			bool read(std::istream& in);
		};
	}
}

/** @brief Write jjyou::io::PlyFormat to stream.
  */
inline std::ostream& operator<<(
	std::ostream& out,
	const jjyou::io::PlyFormat& format
);

/** @brief Read jjyou::io::PlyFormat from stream.
  * 
  * A runtime exception will be thrown if contents in stream
  * are not recognized.
  */
inline std::istream& operator>>(
	std::istream& in,
	jjyou::io::PlyFormat& format
);


/*======================================================================
 | Implementation
 ======================================================================*/
/// @cond


namespace jjyou {
	namespace io {

		template <class VertexTy, class ColorTy, bool HasAlpha>
		inline PlyFile<VertexTy, ColorTy, HasAlpha>::PlyFile(void) : format(PlyFormat::ascii) {}
		
		template <class VertexTy, class ColorTy, bool HasAlpha>
		inline PlyFile<VertexTy, ColorTy, HasAlpha>::~PlyFile() {}

		template <class VertexTy, class ColorTy, bool HasAlpha>
		inline bool PlyFile<VertexTy, ColorTy, HasAlpha>::write(const std::string& fileName) {
			std::ofstream fout(fileName, std::ios::out | std::ios::binary);
			if (!fout.is_open()) return false;
			if (!this->write(fout)) {
				fout.close();
				return false;
			}
			fout.close();
			return true;
		}

		template <class VertexTy, class ColorTy, bool HasAlpha>
		inline bool PlyFile<VertexTy, ColorTy, HasAlpha>::read(const std::string& fileName) {
			this->reset();
			std::ifstream fin(fileName, std::ios::in | std::ios::binary);
			if (!fin.is_open()) return false;
			if (!this->read(fin)) {
				fin.close();
				return false;
			}
			fin.close();
			return true;
		}

		template <class VertexTy, class ColorTy, bool HasAlpha>
		inline void PlyFile<VertexTy, ColorTy, HasAlpha>::reset(void) {
			this->format = PlyFormat::ascii;
			this->comment.clear();
			this->vertex.clear();
			this->vertexNormal.clear();
			this->vertexColor.clear();
			this->edge.clear();
			this->edgeColor.clear();
			this->face.clear();
			this->faceNormal.clear();
			this->faceColor.clear();
		}

		template <class VertexTy, class ColorTy, bool HasAlpha>
		bool PlyFile<VertexTy, ColorTy, HasAlpha>::write(std::ostream& out) {
			auto getTypeName = []<class T>(void) {
				return std::same_as<T, char> ? "char" :
					std::same_as<T, unsigned char> ? "uchar" :
					std::same_as<T, short> ? "short" :
					std::same_as<T, unsigned short> ? "ushort" :
					std::same_as<T, int> ? "int" :
					std::same_as<T, unsigned int> ? "uint" :
					std::same_as<T, float> ? "float" :
					std::same_as<T, double> ? "double" : "unknown";
			};
			const char* vertexTyName = getTypeName.operator()<VertexTy>();
			const char* colorTyName = getTypeName.operator()<ColorTy>();
			bool hasVertexNormal = this->vertexNormal.size() == this->vertex.size();
			bool hasVertexColor = this->vertexColor.size() == this->vertex.size();
			bool hasEdgeColor = this->edgeColor.size() == this->edge.size();
			bool hasFaceNormal = this->faceNormal.size() == this->face.size();
			bool hasFaceColor = this->faceColor.size() == this->face.size();
			out << "ply" << std::endl;
			out << "format " << this->format << " " << this->majorVersion << "." << this->minorVersion << std::endl;
			for (const std::string& c : this->comment)
				out << "comment " << c << std::endl;
			if (this->vertex.size()) {
				out << "element vertex " << this->vertex.size() << std::endl;
				out << "property " << vertexTyName << " x" << std::endl;
				out << "property " << vertexTyName << " y" << std::endl;
				out << "property " << vertexTyName << " z" << std::endl;
				if (hasVertexNormal) {
					out << "property " << vertexTyName << " nx" << std::endl;
					out << "property " << vertexTyName << " ny" << std::endl;
					out << "property " << vertexTyName << " nz" << std::endl;
				}
				if (hasVertexColor) {
					out << "property " << colorTyName << " red" << std::endl;
					out << "property " << colorTyName << " green" << std::endl;
					out << "property " << colorTyName << " blue" << std::endl;
					if (HasAlpha) out << "property " << colorTyName << " alpha" << std::endl;
				}
			}
			if (this->edge.size()) {
				out << "element edge " << this->edge.size() << std::endl;
				out << "property int vertex1" << std::endl;
				out << "property int vertex2" << std::endl;
				if (hasEdgeColor) {
					out << "property " << colorTyName << " red" << std::endl;
					out << "property " << colorTyName << " green" << std::endl;
					out << "property " << colorTyName << " blue" << std::endl;
					if (HasAlpha) out << "property " << colorTyName << " alpha" << std::endl;
				}
			}
			if (this->face.size()) {
				out << "element face " << this->face.size() << std::endl;
				out << "property list uchar int vertex_index" << std::endl;
				if (hasFaceNormal) {
					out << "property " << vertexTyName << " nx" << std::endl;
					out << "property " << vertexTyName << " ny" << std::endl;
					out << "property " << vertexTyName << " nz" << std::endl;
				}
				if (hasFaceColor) {
					out << "property " << colorTyName << " red" << std::endl;
					out << "property " << colorTyName << " green" << std::endl;
					out << "property " << colorTyName << " blue" << std::endl;
					if (HasAlpha) out << "property " << colorTyName << " alpha" << std::endl;
				}
			}
			out << "end_header" << std::endl;
			if (this->format == PlyFormat::ascii) {
				if (this->vertex.size()) {
					for (int i = 0; i < this->vertex.size(); i++) {
						out << +this->vertex[i][0] << " " << +this->vertex[i][1] << " " << +this->vertex[i][2];
						if (hasVertexNormal)
							out << " " << +this->vertexNormal[i][0] << " " << +this->vertexNormal[i][1] << " " << +this->vertexNormal[i][2];
						if (hasVertexColor) {
							out << " " << +this->vertexColor[i][0] << " " << +this->vertexColor[i][1] << " " << +this->vertexColor[i][2];
							if (HasAlpha) out << " " << +this->vertexColor[i][3];
						}
						out << std::endl;
					}
				}
				if (this->edge.size()) {
					for (int i = 0; i < this->edge.size(); i++) {
						out << this->edge[i][0] << " " << this->edge[i][1];
						if (hasEdgeColor) {
							out << " " << +this->edgeColor[i][0] << " " << +this->edgeColor[i][1] << " " << +this->edgeColor[i][2];
							if (HasAlpha) out << " " << +this->edgeColor[i][3];
						}
						out << std::endl;
					}
				}
				if (this->face.size()) {
					for (int i = 0; i < this->face.size(); i++) {
						out << this->face[i].size();
						for (int idx : this->face[i])
							out << " " << idx;
						if (hasFaceNormal)
							out << " " << +this->faceNormal[i][0] << " " << +this->faceNormal[i][1] << " " << +this->faceNormal[i][2];
						if (hasFaceColor) {
							out << " " << +this->faceColor[i][0] << " " << +this->faceColor[i][1] << " " << +this->faceColor[i][2];
							if (HasAlpha) out << " " << +this->faceColor[i][3];
						}
						out << std::endl;
					}
				}
			}
			else {
				bool needReverse =
					(this->format == PlyFormat::binary_little_endian && std::endian::native == std::endian::big) ||
					(this->format == PlyFormat::binary_big_endian && std::endian::native == std::endian::little);
				std::vector<VertexType>* tempVertex;
				std::vector<VertexType>* tempVertexNormal;
				std::vector<ColorType>* tempVertexColor;
				std::vector<EdgeType>* tempEdge;
				std::vector<ColorType>* tempEdgeColor;
				std::vector<FaceType>* tempFace;
				std::vector<VertexType>* tempFaceNormal;
				std::vector<ColorType>* tempFaceColor;
				if (!needReverse) {
					tempVertex = &this->vertex;
					tempVertexNormal = &this->vertexNormal;
					tempVertexColor = &this->vertexColor;
					tempEdge = &this->edge;
					tempEdgeColor = &this->edgeColor;
					tempFace = &this->face;
					tempFaceNormal = &this->faceNormal;
					tempFaceColor = &this->faceColor;
				}
				else {
					tempVertex = new std::vector<VertexType>(this->vertex);
					tempVertexNormal = new std::vector<VertexType>(this->vertexNormal);
					tempVertexColor = new std::vector<ColorType>(this->vertexColor);
					tempEdge = new std::vector<EdgeType>(this->edge);
					tempEdgeColor = new std::vector<ColorType>(this->edgeColor);
					tempFace = new std::vector<FaceType>(this->face);
					tempFaceNormal = new std::vector<VertexType>(this->faceNormal);
					tempFaceColor = new std::vector<ColorType>(this->faceColor);
					for (VertexType& v : *tempVertex)
						for (int i = 0; i < 3; i++)
							utils::byteswap(v[i]);
					for (VertexType& v : *tempVertexNormal)
						for (int i = 0; i < 3; i++)
							utils::byteswap(v[i]);
					for (ColorType& v : *tempVertexColor)
						for (int i = 0; i < (HasAlpha ? 4 : 3); i++)
							utils::byteswap(v[i]);
					for (EdgeType& v : *tempEdge)
						for (int i = 0; i < 2; i++)
							utils::byteswap(v[i]);
					for (ColorType& v : *tempEdgeColor)
						for (int i = 0; i < (HasAlpha ? 4 : 3); i++)
							utils::byteswap(v[i]);
					for (FaceType& v : *tempFace)
						for (int i = 0; i < v.size(); i++)
							utils::byteswap(v[i]);
					for (VertexType& v : *tempFaceNormal)
						for (int i = 0; i < 3; i++)
							utils::byteswap(v[i]);
					for (ColorType& v : *tempFaceColor)
						for (int i = 0; i < (HasAlpha ? 4 : 3); i++)
							utils::byteswap(v[i]);
				}
				if (this->vertex.size()) {
					for (int i = 0; i < this->vertex.size(); i++) {
						out.write((const char*)(*tempVertex)[i].data(), sizeof(VertexType));
						if (hasVertexNormal)
							out.write((const char*)(*tempVertexNormal)[i].data(), sizeof(VertexType));
						if (hasVertexColor)
							out.write((const char*)(*tempVertexColor)[i].data(), sizeof(ColorType));
					}
				}
				if (this->edge.size()) {
					for (int i = 0; i < this->edge.size(); i++) {
						out.write((const char*)&(*tempEdge)[i][0], sizeof(int));
						out.write((const char*)&(*tempEdge)[i][1], sizeof(int));
						if (hasEdgeColor)
							out.write((const char*)(*tempEdgeColor)[i].data(), sizeof(ColorType));
					}
				}
				if (this->face.size()) {
					for (int i = 0; i < this->face.size(); i++) {
						unsigned char numFaces = this->face[i].size();
						out.write((const char*)&numFaces, sizeof(unsigned char));
						for (int idx : (*tempFace)[i])
							out.write((const char*)&idx, sizeof(int));
						if (hasFaceNormal)
							out.write((const char*)(*tempFaceNormal)[i].data(), sizeof(VertexType));
						if (hasFaceColor)
							out.write((const char*)(*tempFaceColor)[i].data(), sizeof(ColorType));
					}
				}
				if (needReverse) {
					delete tempVertex;
					delete tempVertexNormal;
					delete tempVertexColor;
					delete tempEdge;
					delete tempEdgeColor;
					delete tempFace;
					delete tempFaceNormal;
					delete tempFaceColor;
				}
			}
			return (bool)out;
		}


		template <class VertexTy, class ColorTy, bool HasAlpha>
		bool PlyFile<VertexTy, ColorTy, HasAlpha>::read(std::istream& in) {
			auto getWord = [&]() {
				static std::stringstream lineBuf;
				static bool newLine = false;
				std::string word;
				do {
					while (!(lineBuf >> word)) {
						lineBuf.clear();
						std::string line;
						std::getline(in, line);
						lineBuf << line;
						newLine = true;
					}
					if (word == "comment" && newLine) {
						std::string line;
						std::getline(lineBuf, line);
						utils::trim(line);
						this->comment.push_back(line);
						lineBuf.clear();
						continue;
					}
					else {
						newLine = false;
						break;
					}
				} while (true);
				return word;
			};
			//read header
			if (getWord() != "ply") return false;
			if (getWord() != "format") return false;
			try {
				std::stringstream buf(getWord());
				buf >> this->format;
			}
			catch (...) {
				return false;
			}
			if (getWord() != (std::to_string(this->majorVersion) + "." + std::to_string(this->minorVersion)))
				return false;
			std::string word = getWord();
			struct Type {
				std::string name;
				std::unique_ptr<Type> sizeType; //for list
				std::unique_ptr<Type> contentType; //for list
				size_t sizeOf;
				bool setName(const std::string& name) {
					if (name == "char" || name == "int8") {
						this->name = "char";
						this->sizeOf = 1;
					}
					else if (name == "uchar" || name == "uint8") {
						this->name = "uchar";
						this->sizeOf = 1;
					}
					else if (name == "short" || name == "int16") {
						this->name = "short";
						this->sizeOf = 2;
					}
					else if (name == "ushort" || name == "uint16") {
						this->name = "ushort";
						this->sizeOf = 2;
					}
					else if (name == "int" || name == "int32") {
						this->name = "int";
						this->sizeOf = 4;
					}
					else if (name == "uint" || name == "uint32") {
						this->name = "uint";
						this->sizeOf = 4;
					}
					else if (name == "float" || name == "float32") {
						this->name = "float";
						this->sizeOf = 4;
					}
					else if (name == "double" || name == "float64") {
						this->name = "double";
						this->sizeOf = 8;
					}
					else if (name == "list") {
						this->name = "list";
						this->sizeOf = 0;
					}
					else return false;
					return true;
				}
				bool isInteger(void) const {
					return
						this->name == "char" ||
						this->name == "uchar" ||
						this->name == "short" ||
						this->name == "ushort" ||
						this->name == "int" ||
						this->name == "uint";
				}
				bool isFloating(void) const {
					return
						this->name == "float" ||
						this->name == "double";
				}
				bool isList(void) const {
					return
						this->name == "list";
				}
			};
			struct Property {
				Type type;
				std::string name;
			};
			struct Element {
				std::string name;
				size_t size;
				std::vector<Property> properties;
			};
			std::vector<Element> elements;
			while (word == "element") {
				elements.emplace_back(); Element& ele = elements.back();
				ele.name = getWord();
				ele.size = std::stoull(getWord());
				word = getWord();
				while (word == "property") {
					ele.properties.emplace_back(); Property& pro = ele.properties.back();
					pro.type.setName(getWord());
					if (pro.type.name == "list") {
						pro.type.sizeType.reset(new Type());
						pro.type.sizeType->setName(getWord());
						if (!pro.type.sizeType->isInteger())
							return false;
						pro.type.contentType.reset(new Type());
						pro.type.contentType->setName(getWord());
						if (!pro.type.contentType->isInteger() && !pro.type.contentType->isFloating())
							return false;
					}
					pro.name = getWord();
					word = getWord();
				}
			}
			if (word != "end_header") return false;
			//extract information from meta data
			for (const auto& ele : elements) {
				if (ele.name == "vertex") {
					this->vertex.resize(ele.size);
					for (const auto& pro : ele.properties) {
						if (pro.name == "red" || pro.name == "green" || pro.name == "blue" || pro.name == "alpha" && HasAlpha)
							this->vertexColor.resize(ele.size);
						if (pro.name == "nx" || pro.name == "ny" || pro.name == "nz")
							this->vertexNormal.resize(ele.size);
					}
					if (this->vertexColor.size() && HasAlpha)
						for (auto& c : this->vertexColor)
							c[4] = utils::color_cast<ColorTy, unsigned char>(255);
				}
				if (ele.name == "edge") {
					this->edge.resize(ele.size);
					for (const auto& pro : ele.properties) {
						if (pro.name == "red" || pro.name == "green" || pro.name == "blue" || pro.name == "alpha" && HasAlpha)
							this->edgeColor.resize(ele.size);
					}
					if (this->edgeColor.size() && HasAlpha)
						for (auto& c : this->edgeColor)
							c[4] = utils::color_cast<ColorTy, unsigned char>(255);
				}
				if (ele.name == "face") {
					this->face.resize(ele.size);
					for (const auto& pro : ele.properties) {
						if (pro.name == "red" || pro.name == "green" || pro.name == "blue" || pro.name == "alpha" && HasAlpha)
							this->faceColor.resize(ele.size);
						if (pro.name == "nx" || pro.name == "ny" || pro.name == "nz")
							this->faceNormal.resize(ele.size);
					}
					if (this->faceColor.size() && HasAlpha)
						for (auto& c : this->faceColor)
							c[4] = utils::color_cast<ColorTy, unsigned char>(255);
				}
			}
			//read body
			if (this->format == PlyFormat::ascii) {
				auto typeCast = []<class T, bool ConvertColor>(std::string& data, const Type& type) -> T {
					if (type.name == "char")
						if (ConvertColor)
							return utils::color_cast<char, T>(utils::string2Number<char>(data));
						else
							return static_cast<T>(utils::string2Number<char>(data));
					else if (type.name == "uchar")
						if (ConvertColor)
							return utils::color_cast<unsigned char, T>(utils::string2Number<unsigned char>(data));
						else
							return static_cast<T>(utils::string2Number<unsigned char>(data));
					else if (type.name == "short")
						if (ConvertColor)
							return utils::color_cast<short, T>(utils::string2Number<short>(data));
						else
							return static_cast<T>(utils::string2Number<short>(data));
					else if (type.name == "ushort")
						if (ConvertColor)
							return utils::color_cast<unsigned short, T>(utils::string2Number<unsigned short>(data));
						else
							return static_cast<T>(utils::string2Number<unsigned short>(data));
					else if (type.name == "int")
						if (ConvertColor)
							return utils::color_cast<int, T>(utils::string2Number<int>(data));
						else
							return static_cast<T>(utils::string2Number<int>(data));
					else if (type.name == "uint")
						if (ConvertColor)
							return utils::color_cast<unsigned int, T>(utils::string2Number<unsigned int>(data));
						else
							return static_cast<T>(utils::string2Number<unsigned int>(data));
					else if (type.name == "float")
						if (ConvertColor)
							return utils::color_cast<float, T>(utils::string2Number<float>(data));
						else
							return static_cast<T>(utils::string2Number<float>(data));
					else if (type.name == "double")
						if (ConvertColor)
							return utils::color_cast<double, T>(utils::string2Number<double>(data));
						else
							return static_cast<T>(utils::string2Number<double>(data));
					return T();
				};
				std::string buf;
				for (const auto& ele : elements) {
					for (int cnt = 0; cnt < ele.size; cnt++) {
						for (const auto& pro : ele.properties) {
							if (pro.type.isList()) {
								size_t listSize;
								in >> buf;
								listSize = utils::string2Number<size_t>(buf);
								if (ele.name == "face" && (pro.name == "vertex_indices" || pro.name == "vertex_index")) {
									this->face[cnt].resize(listSize);
									for (size_t i = 0; i < listSize; i++) {
										in >> buf;
										this->face[cnt][i] = utils::string2Number<int>(buf);
									}
								}
								else {
									for (size_t i = 0; i < listSize; i++)
										in >> buf;
								}
							}
							else {
								in >> buf;
								if (ele.name == "vertex" && pro.name == "x")
									this->vertex[cnt].x() = typeCast.operator() < VertexTy, false > (buf, pro.type);
								else if (ele.name == "vertex" && pro.name == "y")
									this->vertex[cnt].y() = typeCast.operator() < VertexTy, false > (buf, pro.type);
								else if (ele.name == "vertex" && pro.name == "z")
									this->vertex[cnt].z() = typeCast.operator() < VertexTy, false > (buf, pro.type);
								else if (ele.name == "vertex" && pro.name == "red")
									this->vertexColor[cnt][0] = typeCast.operator() < ColorTy, true > (buf, pro.type);
								else if (ele.name == "vertex" && pro.name == "green")
									this->vertexColor[cnt][1] = typeCast.operator() < ColorTy, true > (buf, pro.type);
								else if (ele.name == "vertex" && pro.name == "blue")
									this->vertexColor[cnt][2] = typeCast.operator() < ColorTy, true > (buf, pro.type);
								else if (ele.name == "vertex" && pro.name == "alpha" && HasAlpha)
									this->vertexColor[cnt][3] = typeCast.operator() < ColorTy, true > (buf, pro.type);
								else if (ele.name == "vertex" && pro.name == "nx")
									this->vertexNormal[cnt].x() = typeCast.operator() < VertexTy, false > (buf, pro.type);
								else if (ele.name == "vertex" && pro.name == "ny")
									this->vertexNormal[cnt].y() = typeCast.operator() < VertexTy, false > (buf, pro.type);
								else if (ele.name == "vertex" && pro.name == "nz")
									this->vertexNormal[cnt].z() = typeCast.operator() < VertexTy, false > (buf, pro.type);
								else if (ele.name == "edge" && pro.name == "vertex1")
									this->edge[cnt][0] = typeCast.operator() < VertexTy, false > (buf, pro.type);
								else if (ele.name == "edge" && pro.name == "vertex2")
									this->edge[cnt][1] = typeCast.operator() < VertexTy, false > (buf, pro.type);
								else if (ele.name == "edge" && pro.name == "red")
									this->edgeColor[cnt][0] = typeCast.operator() < ColorTy, true > (buf, pro.type);
								else if (ele.name == "edge" && pro.name == "green")
									this->edgeColor[cnt][1] = typeCast.operator() < ColorTy, true > (buf, pro.type);
								else if (ele.name == "edge" && pro.name == "blue")
									this->edgeColor[cnt][2] = typeCast.operator() < ColorTy, true > (buf, pro.type);
								else if (ele.name == "edge" && pro.name == "alpha" && HasAlpha)
									this->edgeColor[cnt][3] = typeCast.operator() < ColorTy, true > (buf, pro.type);
								else if (ele.name == "face" && pro.name == "red")
									this->faceColor[cnt][0] = typeCast.operator() < ColorTy, true > (buf, pro.type);
								else if (ele.name == "face" && pro.name == "green")
									this->faceColor[cnt][1] = typeCast.operator() < ColorTy, true > (buf, pro.type);
								else if (ele.name == "face" && pro.name == "blue")
									this->faceColor[cnt][2] = typeCast.operator() < ColorTy, true > (buf, pro.type);
								else if (ele.name == "face" && pro.name == "alpha" && HasAlpha)
									this->faceColor[cnt][3] = typeCast.operator() < ColorTy, true > (buf, pro.type);
								else if (ele.name == "face" && pro.name == "nx")
									this->faceNormal[cnt].x() = typeCast.operator() < VertexTy, false > (buf, pro.type);
								else if (ele.name == "face" && pro.name == "ny")
									this->faceNormal[cnt].y() = typeCast.operator() < VertexTy, false > (buf, pro.type);
								else if (ele.name == "face" && pro.name == "nz")
									this->faceNormal[cnt].z() = typeCast.operator() < VertexTy, false > (buf, pro.type);
							}
						}
					}
				}
			}
			else {
				auto typeCast = []<class T, bool ConvertColor>(void* data, const Type& type, bool byteswap) -> T {
					if (byteswap)
						utils::byteswap(data, type.sizeOf);
					if (type.name == "char")
						if (ConvertColor)
							return utils::color_cast<char, T>(*static_cast<const char*>(data));
						else
							return static_cast<T>(*static_cast<const char*>(data));
					else if (type.name == "uchar")
						if (ConvertColor)
							return utils::color_cast<unsigned char, T>(*static_cast<const unsigned char*>(data));
						else
							return static_cast<T>(*static_cast<const unsigned char*>(data));
					else if (type.name == "short")
						if (ConvertColor)
							return utils::color_cast<short, T>(*static_cast<const short*>(data));
						else
							return static_cast<T>(*static_cast<const short*>(data));
					else if (type.name == "ushort")
						if (ConvertColor)
							return utils::color_cast<unsigned short, T>(*static_cast<const unsigned short*>(data));
						else
							return static_cast<T>(*static_cast<const unsigned short*>(data));
					else if (type.name == "int")
						if (ConvertColor)
							return utils::color_cast<int, T>(*static_cast<const int*>(data));
						else
							return static_cast<T>(*static_cast<const int*>(data));
					else if (type.name == "uint")
						if (ConvertColor)
							return utils::color_cast<unsigned int, T>(*static_cast<const unsigned int*>(data));
						else
							return static_cast<T>(*static_cast<const unsigned int*>(data));
					else if (type.name == "float")
						if (ConvertColor)
							return utils::color_cast<float, T>(*static_cast<const float*>(data));
						else
							return static_cast<T>(*static_cast<const float*>(data));
					else if (type.name == "double")
						if (ConvertColor)
							return utils::color_cast<double, T>(*static_cast<const double*>(data));
						else
							return static_cast<T>(*static_cast<const double*>(data));
					return T();
				};
				bool needReverse =
					(this->format == PlyFormat::binary_little_endian && std::endian::native == std::endian::big) ||
					(this->format == PlyFormat::binary_big_endian && std::endian::native == std::endian::little);
				std::vector<char> buf(8);
				for (const auto& ele : elements) {
					for (int cnt = 0; cnt < ele.size; cnt++) {
						for (const auto& pro : ele.properties) {
							if (pro.type.isList()) {
								size_t listSize;
								in.read(buf.data(), pro.type.sizeType->sizeOf);
								listSize = typeCast.operator() < size_t, false > (buf.data(), *pro.type.sizeType, needReverse);
								if (ele.name == "face" && (pro.name == "vertex_indices" || pro.name == "vertex_index")) {
									this->face[cnt].resize(listSize);
									for (size_t i = 0; i < listSize; i++) {
										in.read(buf.data(), pro.type.contentType->sizeOf);
										this->face[cnt][i] = typeCast.operator() < int, false > (buf.data(), *pro.type.contentType, needReverse);
									}
								}
								else {
									in.seekg(listSize * pro.type.contentType->sizeOf, std::ios::cur);
								}
							}
							else {
								in.read(buf.data(), pro.type.sizeOf);
								if (ele.name == "vertex" && pro.name == "x")
									this->vertex[cnt].x() = typeCast.operator() < VertexTy, false > (buf.data(), pro.type, needReverse);
								else if (ele.name == "vertex" && pro.name == "y")
									this->vertex[cnt].y() = typeCast.operator() < VertexTy, false > (buf.data(), pro.type, needReverse);
								else if (ele.name == "vertex" && pro.name == "z")
									this->vertex[cnt].z() = typeCast.operator() < VertexTy, false > (buf.data(), pro.type, needReverse);
								else if (ele.name == "vertex" && pro.name == "red")
									this->vertexColor[cnt][0] = typeCast.operator() < ColorTy, true > (buf.data(), pro.type, needReverse);
								else if (ele.name == "vertex" && pro.name == "green")
									this->vertexColor[cnt][1] = typeCast.operator() < ColorTy, true > (buf.data(), pro.type, needReverse);
								else if (ele.name == "vertex" && pro.name == "blue")
									this->vertexColor[cnt][2] = typeCast.operator() < ColorTy, true > (buf.data(), pro.type, needReverse);
								else if (ele.name == "vertex" && pro.name == "alpha" && HasAlpha)
									this->vertexColor[cnt][3] = typeCast.operator() < ColorTy, true > (buf.data(), pro.type, needReverse);
								else if (ele.name == "vertex" && pro.name == "nx")
									this->vertexNormal[cnt].x() = typeCast.operator() < VertexTy, false > (buf.data(), pro.type, needReverse);
								else if (ele.name == "vertex" && pro.name == "ny")
									this->vertexNormal[cnt].y() = typeCast.operator() < VertexTy, false > (buf.data(), pro.type, needReverse);
								else if (ele.name == "vertex" && pro.name == "nz")
									this->vertexNormal[cnt].z() = typeCast.operator() < VertexTy, false > (buf.data(), pro.type, needReverse);
								else if (ele.name == "edge" && pro.name == "vertex1")
									this->edge[cnt][0] = typeCast.operator() < int, false > (buf.data(), pro.type, needReverse);
								else if (ele.name == "edge" && pro.name == "vertex2")
									this->edge[cnt][1] = typeCast.operator() < int, false > (buf.data(), pro.type, needReverse);
								else if (ele.name == "edge" && pro.name == "red")
									this->edgeColor[cnt][0] = typeCast.operator() < ColorTy, true > (buf.data(), pro.type, needReverse);
								else if (ele.name == "edge" && pro.name == "green")
									this->edgeColor[cnt][1] = typeCast.operator() < ColorTy, true > (buf.data(), pro.type, needReverse);
								else if (ele.name == "edge" && pro.name == "blue")
									this->edgeColor[cnt][2] = typeCast.operator() < ColorTy, true > (buf.data(), pro.type, needReverse);
								else if (ele.name == "edge" && pro.name == "alpha" && HasAlpha)
									this->edgeColor[cnt][3] = typeCast.operator() < ColorTy, true > (buf.data(), pro.type, needReverse);
								else if (ele.name == "face" && pro.name == "red")
									this->faceColor[cnt][0] = typeCast.operator() < ColorTy, true > (buf.data(), pro.type, needReverse);
								else if (ele.name == "face" && pro.name == "green")
									this->faceColor[cnt][1] = typeCast.operator() < ColorTy, true > (buf.data(), pro.type, needReverse);
								else if (ele.name == "face" && pro.name == "blue")
									this->faceColor[cnt][2] = typeCast.operator() < ColorTy, true > (buf.data(), pro.type, needReverse);
								else if (ele.name == "face" && pro.name == "alpha" && HasAlpha)
									this->faceColor[cnt][3] = typeCast.operator() < ColorTy, true > (buf.data(), pro.type, needReverse);
								else if (ele.name == "face" && pro.name == "nx")
									this->faceNormal[cnt].x() = typeCast.operator() < VertexTy, false > (buf.data(), pro.type, needReverse);
								else if (ele.name == "face" && pro.name == "ny")
									this->faceNormal[cnt].y() = typeCast.operator() < VertexTy, false > (buf.data(), pro.type, needReverse);
								else if (ele.name == "face" && pro.name == "nz")
									this->faceNormal[cnt].z() = typeCast.operator() < VertexTy, false > (buf.data(), pro.type, needReverse);
							}
						}
					}
				}
			}
			return (bool)in;
		}
	}
}

inline std::ostream& operator<<(
	std::ostream& out,
	const jjyou::io::PlyFormat& format
	) {
	switch (format) {
	case jjyou::io::PlyFormat::ascii:
		out << "ascii";
		break;
	case jjyou::io::PlyFormat::binary_little_endian:
		out << "binary_little_endian";
		break;
	case jjyou::io::PlyFormat::binary_big_endian:
		out << "binary_big_endian";
		break;
	}
	return out;
}

inline std::istream& operator>>(
	std::istream& in,
	typename jjyou::io::PlyFormat& format
	) {
	std::string formatString;
	in >> formatString;
	if (formatString == "ascii")
		format = jjyou::io::PlyFormat::ascii;
	else if (formatString == "binary_little_endian")
		format = jjyou::io::PlyFormat::binary_little_endian;
	else if (formatString == "binary_big_endian")
		format = jjyou::io::PlyFormat::binary_big_endian;
	else
		throw std::runtime_error("Unknown format");
	return in;
}
/// @endcond

#endif /* jjyou_io_PlyFile_hpp */