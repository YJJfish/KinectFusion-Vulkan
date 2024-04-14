/***********************************************************************
 * @file	HalfedgeMesh_Impl.hpp
 * @author	jjyou
 * @date	2024-1-9
 * @brief	This file implements some methods of HalfedgeMesh class.
***********************************************************************/
#ifndef jjyou_geo_HalfedgeMesh_Impl_hpp
#define jjyou_geo_HalfedgeMesh_Impl_hpp

#include "HalfedgeMesh.hpp"
#include "IndexedMesh.hpp"
#include <vector>
#include <map>
#include <unordered_map>

namespace jjyou {

	namespace geo {

		template <class FP> void HalfedgeMesh<FP>::collectGarbage(void) {
			// create mapping
			std::vector<std::uint32_t> vertex_mapping(this->_vertices.size());
			std::vector<std::uint32_t> halfedge_mapping(this->_halfedges.size());
			std::vector<std::uint32_t> face_mapping(this->_faces.size());
			std::vector<std::uint32_t> edge_mapping(this->_edges.size());
			for (uint32_t offset = 0, cnt = 0; offset < this->_vertices.size(); ++offset) {
				vertex_mapping[offset] = cnt;
				if (!this->_vertices[offset]._removed)
					++cnt;
			}
			for (uint32_t offset = 0, cnt = 0; offset < this->_halfedges.size(); ++offset) {
				halfedge_mapping[offset] = cnt;
				if (!this->_halfedges[offset]._removed)
					++cnt;
			}
			for (uint32_t offset = 0, cnt = 0; offset < this->_faces.size(); ++offset) {
				face_mapping[offset] = cnt;
				if (!this->_faces[offset]._removed)
					++cnt;
			}
			for (uint32_t offset = 0, cnt = 0; offset < this->_edges.size(); ++offset) {
				edge_mapping[offset] = cnt;
				if (!this->_edges[offset]._removed)
					++cnt;
			}
			// update connectivity
			for (uint32_t offset = 0, cnt = 0; offset < this->_vertices.size(); ++offset, ++cnt) {
				while (cnt < this->_vertices.size() && this->_vertices[cnt]._removed) ++cnt;
				if (cnt == this->_vertices.size())
					break;
				Vertex& v = this->_vertices[offset];
				v = this->_vertices[cnt];
				v.halfedge.offset = halfedge_mapping[v.halfedge.offset];
			}
			this->_vertices.resize(this->numVertices());
			this->_removedVertices.clear();
			for (uint32_t offset = 0, cnt = 0; offset < this->_halfedges.size(); ++offset, ++cnt) {
				while (cnt < this->_halfedges.size() && this->_halfedges[cnt]._removed) ++cnt;
				if (cnt == this->_halfedges.size())
					break;
				Halfedge& h = this->_halfedges[offset];
				h = this->_halfedges[cnt];
				h.next.offset = halfedge_mapping[h.next.offset];
				h.prev.offset = halfedge_mapping[h.prev.offset];
				h.twin.offset = halfedge_mapping[h.twin.offset];
				h.source.offset = vertex_mapping[h.source.offset];
				h.edge.offset = edge_mapping[h.edge.offset];
				h.face.offset = face_mapping[h.face.offset];
			}
			this->_halfedges.resize(this->numHalfedges());
			this->_removedHalfedges.clear();
			for (uint32_t offset = 0, cnt = 0; offset < this->_faces.size(); ++offset, ++cnt) {
				while (cnt < this->_faces.size() && this->_faces[cnt]._removed) ++cnt;
				if (cnt == this->_faces.size())
					break;
				Face& f = this->_faces[offset];
				f = this->_faces[cnt];
				f.halfedge.offset = halfedge_mapping[f.halfedge.offset];
			}
			this->_faces.resize(this->numFaces());
			this->_removedFaces.clear();
			for (uint32_t offset = 0, cnt = 0; offset < this->_edges.size(); ++offset, ++cnt) {
				while (cnt < this->_edges.size() && this->_edges[cnt]._removed) ++cnt;
				if (cnt == this->_edges.size())
					break;
				Edge& e = this->_edges[offset];
				e = this->_edges[cnt];
				e.halfedge.offset = halfedge_mapping[e.halfedge.offset];
			}
			this->_edges.resize(this->numEdges());
			this->_removedEdges.clear();
		}

		template <class FP> bool HalfedgeMesh<FP>::fromIndexedMesh(const IndexedMesh<FP>& indexedMesh) {
			this->clear();
			// Reserve memory
			this->_vertices.reserve(indexedMesh._vertices.size());
			this->_edges.reserve(indexedMesh._faces.size() + indexedMesh._vertices.size() - 2); // Euler's formula: F+V=E+2
			this->_halfedges.reserve(2 * (indexedMesh._faces.size() + indexedMesh._vertices.size() - 2));
			this->_faces.reserve(indexedMesh._faces.size());
			// Create vertices
			for (const auto& v : indexedMesh._vertices) {
				VertexIter newv = this->emplace<Vertex>();
				newv->position = v.position;
			}
			// Define hash function for `std::pair<std::uint32_t, std::uint32_t>`
			struct PairHash {
				using argument_type = std::pair<std::uint32_t, std::uint32_t>;
				using result_type = std::size_t;
				result_type operator()(argument_type const& key) const {
					static const std::hash<std::uint32_t> h;
					result_type h1 = h(key.first);
					result_type h2 = h(key.second);
					return h1 ^ (h2 << (sizeof(result_type) * 4)) ^ (h2 >> (sizeof(result_type) * 4));
				}
			};
			// halfedgeMap[{v1, v2}] stores the halfedge v1->v2
			std::unordered_map<std::pair<std::uint32_t, std::uint32_t>, std::uint32_t, PairHash> halfedgeMap;
			// For each face
			for (const auto& f : indexedMesh._faces) {
				std::vector<HalfedgeIter> faceHalfedges(f.degree()); // faceHalfedges stores the halfedges around the current face
				FaceIter newf = this->emplace<Face>();
				for (int hi = 0; hi < f.degree(); hi++) {
					const auto& c1 = f.corners[hi];
					if (c1.vIdx >= this->_vertices.size()) {
						this->clear(); return false;
					}
					VertexIter v1 = this->vertex(c1.vIdx);
					const auto& c2 = f.corners[(hi + 1) % f.degree()];
					if (c2.vIdx >= this->_vertices.size()) {
						this->clear(); return false;
					}
					VertexIter v2 = this->vertex(c2.vIdx);
					if (c1.vIdx == c2.vIdx) {
						this->clear(); return false;
					}
					auto ret1 = halfedgeMap.emplace(std::pair<std::uint32_t, std::uint32_t>(c1.vIdx, c2.vIdx), 0U);
					if (ret1.second) { // If insertion succeeds, insert its twin halfedge
						auto ret2 = halfedgeMap.emplace(std::pair<std::uint32_t, std::uint32_t>(c2.vIdx, c1.vIdx), 0U);
						EdgeIter newe = this->emplace<Edge>();
						HalfedgeIter newh1 = this->emplace<Halfedge>();
						HalfedgeIter newh2 = this->emplace<Halfedge>();
						ret1.first->second = newh1.offset;
						ret2.first->second = newh2.offset;
						newh1->twin = newh2;
						newh1->source = v1;
						newh1->edge = newe;
						newh2->twin = newh1;
						newh2->source = v2;
						newh2->edge = newe;
						newe->halfedge = newh1;
						faceHalfedges[hi] = newh1;
					} else // If insertion fails, get the halfedge
						faceHalfedges[hi] = this->halfedge(ret1.first->second);
					if (faceHalfedges[hi]->face.valid()) { // If this halfedge already belongs to a face, fail
						this->clear(); return false;
					}
					faceHalfedges[hi]->face = newf;
					faceHalfedges[hi]->uv = c1.uv;
					faceHalfedges[hi]->normal = c1.normal;
					faceHalfedges[hi]->tangent = c1.tangent;
					if (!v1->halfedge.valid())
						v1->halfedge = faceHalfedges[hi];
					if (hi == 0)
						newf->halfedge = faceHalfedges[hi];
				}
				for (int hi = 0; hi < f.degree(); hi++) {
					faceHalfedges[hi]->next = faceHalfedges[(hi + 1) % f.degree()];
					faceHalfedges[hi]->prev = faceHalfedges[(hi - 1 + f.degree()) % f.degree()];
				}
			}
			// Find all boundary halfedges
			std::map<std::uint32_t, std::uint32_t> boundaryHalfedges;
			for (const auto& [vIdxPair, hIdx] : halfedgeMap) {
				if (!this->halfedge(hIdx)->face.valid()) {
					auto ret = boundaryHalfedges.insert(vIdxPair);
					if (!ret.second) { // A vertex can belong to at most one boundary face
						this->clear(); return false;
					}
				}
			}
			// Create boundary faces
			while (!boundaryHalfedges.empty()) {
				std::vector<HalfedgeIter> faceHalfedges; // faceHalfedges stores the halfedges around the current face
				FaceIter newf = this->emplace<Face>();
				newf->boundary = true;
				std::pair<std::uint32_t, std::uint32_t> v12Idx = *boundaryHalfedges.begin();
				std::uint32_t firstVIdx = v12Idx.first;
				faceHalfedges.push_back(this->halfedge(halfedgeMap[v12Idx]));
				boundaryHalfedges.erase(v12Idx.first);
				while (true) {
					auto ret = boundaryHalfedges.find(v12Idx.second);
					if (ret == boundaryHalfedges.end()) {
						this->clear(); return false;
					}
					v12Idx = *ret;
					boundaryHalfedges.erase(v12Idx.first);
					HalfedgeIter h = this->halfedge(halfedgeMap[v12Idx]);
					faceHalfedges.push_back(h);
					if (v12Idx.second == firstVIdx)
						break;
				}
				newf->halfedge = faceHalfedges.front();
				for (int hi = 0; hi < faceHalfedges.size(); hi++) {
					faceHalfedges[hi]->next = faceHalfedges[(hi + 1) % faceHalfedges.size()];
					faceHalfedges[hi]->prev = faceHalfedges[(hi - 1 + faceHalfedges.size()) % faceHalfedges.size()];
					faceHalfedges[hi]->face = newf;
				}
			}
			return true;
		}

		template <class FP> void HalfedgeMesh<FP>::computeFaceNormals(void) {
			for (FaceCIter f = this->faces().cbegin(); f != this->faces().cend(); ++f) {
				if (f->boundary) continue;
				Vec3 normal = f->halfedge->vector().cross(f->halfedge->prev->twin->vector()).normalized();
				HalfedgeIter h = f->halfedge;
				do {
					h->normal = normal;
					h = h->next;
				} while (h != f->halfedge);
			}
		}

		template <class FP> void HalfedgeMesh<FP>::computeVertexNormals(void) {
			this->computeFaceNormals();
			for (VertexCIter v = this->vertices().cbegin(); v != this->vertices().cend(); ++v) {
				Vec3 normal = Vec3::Zero();
				HalfedgeIter h;
				h = v->halfedge;
				do {
					if (!h->onBoundary()) {
						normal += h->normal;
					}
					h = h->twin->next;
				} while (h != v->halfedge);
				normal.normalize();
				h = v->halfedge;
				do {
					if (!h->onBoundary()) {
						h->normal = normal;
					}
					h = h->twin->next;
				} while (h != v->halfedge);
			}
		}

		template <class FP> void HalfedgeMesh<FP>::computeTangents(void) {
			for (FaceCIter f = this->faces().cbegin(); f != this->faces().cend(); ++f) {
				if (f->boundary) continue;
				Eigen::Matrix<FP, 3, 2> E;
				E.col(0) = f->halfedge->vector();
				E.col(1) = f->halfedge->prev->twin->vector();
				Eigen::Matrix<FP, 2, 2> UV;
				UV.col(0) = f->halfedge->next->uv - f->halfedge->uv;
				UV.col(1) = f->halfedge->prev->uv - f->halfedge->uv;
				Vec3 tangent = (E * UV.inverse()).col(0).normalized();
				HalfedgeIter h = f->halfedge;
				do {
					h->tangent = tangent;
					h = h->next;
				} while (h != f->halfedge);
			}
		}

		template <class FP> std::string HalfedgeMesh<FP>::validate(void) const {
			// 1. Check reference validity
			VertexCRange vRange = this->vertices();
			HalfedgeCRange hRange = this->halfedges();
			FaceCRange fRange = this->faces();
			EdgeCRange eRange = this->edges();
			for (VertexCIter v = vRange.begin(); v != vRange.end(); ++v) {
				if (!v->halfedge.valid())
					return v->name() + ": Invalid halfedge.";
			}
			for (HalfedgeCIter h = hRange.begin(); h != hRange.end(); ++h) {
				if (!h->next.valid())
					return h->name() + ": Invalid next.";
				if (!h->prev.valid())
					return h->name() + ": Invalid prev.";
				if (!h->twin.valid())
					return h->name() + ": Invalid twin.";
				if (!h->source.valid())
					return h->name() + ": Invalid source.";
				if (!h->edge.valid())
					return h->name() + ": Invalid edge.";
				if (!h->face.valid())
					return h->name() + ": Invalid face.";
			}
			for (FaceCIter f = fRange.begin(); f != fRange.end(); ++f) {
				if (!f->halfedge.valid())
					return f->name() + ": Invalid halfedge.";
			}
			for (EdgeCIter e = eRange.begin(); e != eRange.end(); ++e) {
				if (!e->halfedge.valid())
					return e->name() + ": Invalid halfedge.";
			}
			// 2. v->halfedge(->twin->next)^n is a cycle and == all halfedges whose source is v.
			for (VertexCIter v = vRange.begin(); v != vRange.end(); ++v) {
				std::unordered_set<HalfedgeCIter> vertexHalfedges;
				HalfedgeCIter h = v->halfedge;
				int cnt = 0;
				while (!vertexHalfedges.count(h)) {
					vertexHalfedges.emplace(h);
					if (h->source != v)
						return v->name() + ": " + h->name() + " = " + v->name() + "->halfedge(->twin->next)^" + std::to_string(cnt)  + " but its source is not " + v->name() + ".";
					h = h->twin->next;
					++cnt;
				}
				if (h != v->halfedge)
					return v->name() + ": " + v->name() + "->halfedge(->twin->next)^n does not form a cycle.";
				if (vertexHalfedges.size() == 1)
					return v->name() + ": " + v->name() + "->halfedge(->twin->next)^n only contains one halfedge " + h->name() + ".";
				for (HalfedgeCIter h = hRange.begin(); h != hRange.end(); ++h) {
					if (!vertexHalfedges.count(h) && h->source == v)
						return h->name() + ": " + h->name() + " does not belong to " + v->name() + "->halfedge(->twin->next)^n but its source is " + v->name() + ".";
				}
			}
			// 3. f->halfedge(->next)^n is a cycle and == all halfedges whose face is f.
			for (FaceCIter f = fRange.begin(); f != fRange.end(); ++f) {
				std::unordered_set<HalfedgeCIter> faceHalfedges;
				HalfedgeCIter h = f->halfedge;
				int cnt = 0;
				while (!faceHalfedges.count(h)) {
					faceHalfedges.emplace(h);
					if (h->face != f)
						return f->name() + ": " + h->name() + " = " + f->name() + "->halfedge(->next)^" + std::to_string(cnt) + " but its face is not " + f->name() + ".";
					h = h->next;
					++cnt;
				}
				if (h != f->halfedge)
					return f->name() + ": " + f->name() + "->halfedge(->next)^n does not form a cycle.";
				if (faceHalfedges.size() <= 2)
					return f->name() + ": " + f->name() + "->halfedge(->next)^n contains less than 3 halfedges.";
				for (HalfedgeCIter h = hRange.begin(); h != hRange.end(); ++h) {
					if (!faceHalfedges.count(h) && h->face == f)
						return h->name() + ": " + h->name() + " does not belong to " + f->name() + "->halfedge(->next)^n but its face is " + f->name() + ".";
				}
			}
			// 4. e->halfedge(->twin)^n is a cycle and == two halfedges whose edge is e.
			for (EdgeCIter e = eRange.begin(); e != eRange.end(); ++e) {
				std::unordered_set<HalfedgeCIter> edgeHalfedges;
				HalfedgeCIter h = e->halfedge;
				int cnt = 0;
				while (!edgeHalfedges.count(h)) {
					edgeHalfedges.emplace(h);
					if (h->edge != e)
						return e->name() + ": " + h->name() + " = " + e->name() + "->halfedge(->twin)^" + std::to_string(cnt) + " but its edge is not " + e->name() + ".";
					h = h->twin;
					++cnt;
				}
				if (h != e->halfedge)
					return e->name() + ": " + e->name() + "->halfedge(->twin)^n does not form a cycle.";
				if (edgeHalfedges.size() != 2)
					return e->name() + ": " + e->name() + "->halfedge(->twin)^n does not contain exactly 2 halfedges.";
				for (HalfedgeCIter h = hRange.begin(); h != hRange.end(); ++h) {
					if (!edgeHalfedges.count(h) && h->edge == e)
						return h->name() + ": " + h->name() + " does not belong to " + e->name() + "->halfedge(->twin)^n but its edge is " + e->name() + ".";
				}
			}
			// 5. The prev of the next of a halfedge is itself.
			for (HalfedgeCIter h = hRange.begin(); h != hRange.end(); ++h) {
				if (h->next->prev != h)
					return h->name() + ": " + h->name() + "->next->prev is not itself.";
			}
			// 6. A vertex can belong to at most one boundary face.
			for (VertexCIter v = vRange.begin(); v != vRange.end(); ++v) {
				HalfedgeCIter h = v->halfedge;
				FaceCIter boundaryFace;
				do {
					if (h->face->boundary) {
						if (!boundaryFace.valid())
							boundaryFace = h->face;
						else
							return v->name() + ": " + v->name() + " belongs to at least two boundary faces " + boundaryFace->name() + " " + h->face->name() + ".";
					}
					h = h->twin->next;
				} while (h != v->halfedge);
			}
			return "";
		}
	}
	
}

#endif /* jjyou_geo_HalfedgeMesh_Impl_hpp */