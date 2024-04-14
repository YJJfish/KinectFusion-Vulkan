/***********************************************************************
 * @file	IndexedMesh.hpp
 * @author	jjyou
 * @date	2024-1-7
 * @brief	This file implements IndexedMesh class.
***********************************************************************/
#ifndef jjyou_geo_IndexedMesh_hpp
#define jjyou_geo_IndexedMesh_hpp

namespace jjyou {
	namespace geo {

		//Forward declaration
		template <class FP>
		class HalfedgeMesh;

		/***********************************************************************
		 * @class IndexedMesh
		 * @brief Indexed data structure of a mesh.
		 *
		 * This class stores meshes in indexed data structure. Different from
		 * jjyou::geo::HalfedgeMesh, this data structure is relatively simpler.
		 * It does not provide mesh modification functions. To modify the mesh,
		 * you need to directly modify its member variables.
		 *
		 * @sa			jjyou::geo::HalfedgeMesh
		 ***********************************************************************/
		template <class FP>
		class IndexedMesh {

			/*============================================================
			 *                    Forward declarations
			 *============================================================*/
		public:
			using Vec3 = Eigen::Vector<FP, 3>;
			using Vec2 = Eigen::Vector<FP, 2>;
			class Vertex;
			class Corner;
			class Face;
			/*============================================================
			 *                 End of forward declarations
			 *============================================================*/

			 /** @defgroup	Element Classes
				* @brief		Element classes for vertex, halfedge, face, and edge.
				*
				* @{
				*/

		public:

			class Vertex {

			public:

				/** @brief Position of the vertex.
				  */
				Vec3 position;

				/** @brief Default constructor.
				  */
				Vertex(void) : position(Vec3::Zero()) {}

				/** @brief Construct from position.
				  */
				Vertex(const Vec3& position) : position(position) {}

				/** @brief Construct from position.
				  */
				Vertex(FP x, FP y, FP z) : position(x, y, z) {}

				/** @brief Helper printing function.
				  */
				friend std::ostream& operator<<(std::ostream& out, const Vertex& v) {
					out << "v(" << v.position.x << ", " << v.position.y << ", " << v.position.z << ")";
					return out;
				}

			private:

				friend class IndexedMesh;

			};

			class Corner {

			public:

				/** @brief Index of the vertex.
				  */
				std::uint32_t vIdx;

				/** @brief The texture coordinate of the corner.
				  */
				Vec2 uv;

				/** @brief The normal of the corner.
				  */
				Vec3 normal;

				/** @brief The tangent of the source vertex.
				  */
				Vec3 tangent;

				Corner(void) : vIdx(0U), uv(Vec2::Zero()), normal(Vec3::Zero()), tangent(Vec3::Zero()) {}

				Corner(std::uint32_t vIdx) : vIdx(vIdx), uv(Vec2::Zero()), normal(Vec3::Zero()), tangent(Vec3::Zero()) {}

				Corner(std::uint32_t vIdx, const Vec2& uv, const Vec3& normal, const Vec3& tangent) : vIdx(vIdx), uv(uv), normal(normal), tangent(tangent) {}
			
			private:

				friend class IndexedMesh;
			
			};

			class Face {

			public:

				/** @brief List of vertices, uv coordinates, and normals.
				  */
				std::vector<Corner> corners;

				/** @brief Default constructor.
				  */
				Face(void) : corners() {}

				/** @brief Construct from corners.
				  */
				Face(const std::vector<Corner>& corners) : corners(corners) {}

				/** @brief Construct from corners.
				  */
				Face(std::vector<Corner>&& corners) : corners(std::move(corners)) {}

				/** @brief Compute the degree of the face.
				  */
				std::uint32_t degree(void) const {
					return this->corners.size();
				}

			private:

				friend class IndexedMesh;

			};

			/** @}
			  */

		public:

			/** @brief Default constructor.
			  */
			IndexedMesh(void) : _vertices(), _faces() {}

			/** @brief Construct from vertices and faces.
			  */
			IndexedMesh(const std::vector<Vertex>& vertices, const std::vector<Face>& faces) : _vertices(vertices), _faces(faces) {}
			
			/** @brief Construct from vertices and faces.
			  */
			IndexedMesh(std::vector<Vertex>&& vertices, std::vector<Face>&& faces) : _vertices(std::move(vertices)), _faces(std::move(faces)) {}

			/** @brief	Remove all elements in the mesh.
			  */
			void clear(void) {
				this->_vertices.clear();
				this->_faces.clear();
			}

			std::vector<Vertex>& vertices(void) { return this->_vertices; }

			const std::vector<Vertex>& vertices(void) const { return this->_vertices; }

			std::vector<Face>& faces(void) { return this->_faces; }

			const std::vector<Face>& faces(void) const { return this->_faces; }

			/** @brief Convert HalfedgeMesh to IndexedMesh.
			  */
			void fromHalfedgeMesh(const HalfedgeMesh<FP>& halfedgeMesh);

			/** @brief	Compute face normals.
			  *			Compute non-boundary face normals and store them in Corner::normal.
			  *			All corners around a face will have the same normal.
			  *			It is better to triangulate the mesh before calling this method.
			  * @sa		jjyou::geo::IndexedMesh::computeVertexNormals
			  */
			void computeFaceNormals(void);

			/** @brief	Compute vertex normals.
			  *			Compute vertex normals and store them in Corner::normal. The vertex normals
			  *			are computed as the average of incident faces' normals.
			  *			It is better to triangulate the mesh before calling this method.
			  * @sa		jjyou::geo::IndexedMesh::computeFaceNormals
			  */
			void computeVertexNormals(void);

			/** @brief	Compute tangents.
			  *			Compute tangents and store them in Corner::tangent. The mesh
			  *			must have uv coordinates.
			  * @sa		https://learnopengl.com/Advanced-Lighting/Normal-Mapping
			  */
			void computeTangents(void);

		private:

			std::vector<Vertex> _vertices;
			std::vector<Face> _faces;

			template <class _FP> friend class HalfedgeMesh;

		};

		IndexedMesh<float>;
		IndexedMesh<double>;

	}
}

#endif /* jjyou_geo_IndexedMesh_hpp */