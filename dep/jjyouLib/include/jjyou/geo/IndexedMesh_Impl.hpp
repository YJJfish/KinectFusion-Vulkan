/***********************************************************************
 * @file	IndexedMesh_Impl.hpp
 * @author	jjyou
 * @date	2024-1-9
 * @brief	This file implements some methods of IndexedMesh class.
***********************************************************************/
#ifndef jjyou_geo_IndexedMesh_Impl_hpp
#define jjyou_geo_IndexedMesh_Impl_hpp

#include "HalfedgeMesh.hpp"
#include "IndexedMesh.hpp"

namespace jjyou {

	namespace geo {

		template <class FP> void IndexedMesh<FP>::fromHalfedgeMesh(const HalfedgeMesh<FP>& halfedgeMesh) {
			this->clear();
			// Reserve memory
			this->_vertices.reserve(halfedgeMesh.numVertices());
			this->_faces.reserve(halfedgeMesh.numFaces());
			// Create vertices
			std::unordered_map<typename HalfedgeMesh<FP>::VertexCIter, std::uint32_t> verticeMap;
			for (typename HalfedgeMesh<FP>::VertexCIter v = halfedgeMesh.vertices().begin(); v != halfedgeMesh.vertices().end(); ++v) {
				verticeMap[v] = this->_vertices.size();
				this->_vertices.emplace_back(v->position);
			}
			// Create faces
			for (typename HalfedgeMesh<FP>::FaceCIter f = halfedgeMesh.faces().begin(); f != halfedgeMesh.faces().end(); ++f) {
				std::vector<Corner> corners; corners.reserve(f->degree());
				typename HalfedgeMesh<FP>::HalfedgeCIter h = f->halfedge;
				do {
					corners.emplace_back(verticeMap[h->source], h->uv, h->normal, h->tangent);
					h = h->next;
				} while (h != f->halfedge);
				this->_faces.emplace_back(std::move(corners));
			}
			return;
		}

		template <class FP> void IndexedMesh<FP>::computeFaceNormals(void) {
			for (const Face& f : this->_faces) {
				Vec3 normal = (this->_vertices[f.corners[1].vIdx].position - this->_vertices[f.corners.front().vIdx].position).cross(
					this->_vertices[f.corners.back().vIdx].position - this->_vertices[f.corners.front().vIdx].position
				).normalized();
				for (Corner& corner : f.corners) {
					corner.normal = normal;
				}
			}
		}

		template <class FP> void IndexedMesh<FP>::computeVertexNormals(void) {
			this->computeFaceNormals();
			std::vector<Vec3> vertexNormals(this->_vertices.size(), Vec3::Zero());
			for (const Face& f : this->_faces) {
				for (const Corner& corner : f.corners) {
					vertexNormals[corner.vIdx] += corner.normal;
				}
			}
			for (Vec3& normal : vertexNormals) {
				normal.normalize();
			}
			for (const Face& f : this->_faces) {
				for (Corner& corner : f.corners) {
					corner.normal = vertexNormals[corner.vIdx];
				}
			}
		}

		template <class FP> void IndexedMesh<FP>::computeTangents(void) {
			for (const Face& f : this->_faces) {
				Eigen::Matrix<FP, 3, 2> E;
				E.col(0) = this->_vertices[f.corners[1].vIdx].position - this->_vertices[f.corners.front().vIdx].position;
				E.col(1) = this->_vertices[f.corners.back().vIdx].position - this->_vertices[f.corners.front().vIdx].position;
				Eigen::Matrix<FP, 2, 2> UV;
				UV.col(0) = f.corners.[1].uv - f.corners.front().uv;
				UV.col(1) = f.corners.back().uv - f.corners.front().uv;
				Vec3 tangent = (E * UV.inverse()).col(0).normalized();
				for (Corner& corner : f.corners) {
					corner.tangent = tangent;
				}
			}
		}

	}

}

#endif /* jjyou_geo_IndexedMesh_Impl_hpp */