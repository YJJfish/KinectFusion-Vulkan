/***********************************************************************
 * @file	HalfedgeMesh.hpp
 * @author	jjyou
 * @date	2024-1-7
 * @brief	This file implements HalfedgeMesh class.
***********************************************************************/
#ifndef jjyou_geo_HalfedgeMesh_hpp
#define jjyou_geo_HalfedgeMesh_hpp

#include <iostream>
#include <vector>
#include <type_traits>
#include <functional>
#include <string>
#include <unordered_set>
#include <Eigen/Eigen>

namespace jjyou {
	namespace geo {

		//Forward declaration
		template <class FP>
		class IndexedMesh;

		/***********************************************************************
		 * @class HalfedgeMesh
		 * @brief Halfedge data structure of a mesh.
		 *
		 * This class stores meshes in halfedge data structure.
		 * 
		 * @sa			https://en.wikipedia.org/wiki/Doubly_connected_edge_list
		 * @sa			jjyou::geo::IndexedMesh
		 ***********************************************************************/
		template <class FP>
		class HalfedgeMesh {

			/*============================================================
			 *                    Forward declarations
			 *============================================================*/
		public:
			using Vec3 = Eigen::Vector<FP, 3>;
			using Vec2 = Eigen::Vector<FP, 2>;
			class Vertex;
			class Halfedge;
			class Face;
			class Edge;
		private:
			template <class T> class BaseIterator;
			template <class T> class BaseRange;
		public:
			using VertexIter = BaseIterator<std::vector<Vertex>>;
			using VertexCIter = BaseIterator<const std::vector<Vertex>>;
			using HalfedgeIter = BaseIterator<std::vector<Halfedge>>;
			using HalfedgeCIter = BaseIterator<const std::vector<Halfedge>>;
			using FaceIter = BaseIterator<std::vector<Face>>;
			using FaceCIter = BaseIterator<const std::vector<Face>>;
			using EdgeIter = BaseIterator<std::vector<Edge>>;
			using EdgeCIter = BaseIterator<const std::vector<Edge>>;
			using VertexRange = BaseRange<std::vector<Vertex>>;
			using VertexCRange = BaseRange<const std::vector<Vertex>>;
			using HalfedgeRange = BaseRange<std::vector<Halfedge>>;
			using HalfedgeCRange = BaseRange<const std::vector<Halfedge>>;
			using FaceRange = BaseRange<std::vector<Face>>;
			using FaceCRange = BaseRange<const std::vector<Face>>;
			using EdgeRange = BaseRange<std::vector<Edge>>;
			using EdgeCRange = BaseRange<const std::vector<Edge>>;
			/*============================================================
			 *                 End of forward declarations
			 *============================================================*/

		private:

			template <class T> class BaseIterator {

			public:

				/** @brief	Default constructor.
				  */
				BaseIterator(void) : data(nullptr), offset(0) {}

				/** @brief	Check whether the iterator is valid or not.
				  * @return `true` if the iterator is valid.
				  */
				bool valid(void) const {
					return (this->data) && (this->offset < this->data->size()) && !(*this->data)[this->offset].removed();
				}

				/** @brief	Construct from non-const iterator.
				  *
				  * The current iterator can be constructed from a non-const iterator if the current one
				  * is a const one.
				  */
				BaseIterator(const BaseIterator<std::remove_const_t<T>>& other) : data(other.data), offset(other.offset) { }

				/** @brief	Compare two iterators.
				  *
				  * Two iterators can be compared iff they have the same class type, or one of them
				  * is the const version of the other.
				  * @return	`true` if lhs == rhs.
				  */
				template <class U, std::enable_if_t<std::is_same_v<std::remove_const_t<T>, std::remove_const_t<U>>, bool> = true>
				bool operator==(const BaseIterator<U>& other) const {
					return (this->data == other.data) && (this->offset == other.offset);
				}

				/** @brief	Compare two iterators.
				  *
				  * Two iterators can be compared iff they have the same class type, or one of them
				  * is the const version of the other.
				  * @return	`true` if lhs != rhs.
				  */
				template <class U, std::enable_if_t<std::is_same_v<std::remove_const_t<T>, std::remove_const_t<U>>, bool> = true>
				bool operator!=(const BaseIterator<U>& other) const {
					return !(*this == other);
				}

				/** @brief	Compare two iterators.
				  *
				  * Two iterators can be compared iff they have the same class type, or one of them
				  * is the const version of the other.
				  * @return	`true` if lhs >= rhs.
				  */
				template <class U, std::enable_if_t<std::is_same_v<std::remove_const_t<T>, std::remove_const_t<U>>, bool> = true>
				bool operator>=(const BaseIterator<U>& other) const {
					return this->data == other.data && this->offset >= other.offset;
				}

				/** @brief	Compare two iterators.
				  *
				  * Two iterators can be compared iff they have the same class type, or one of them
				  * is the const version of the other.
				  * @return	`true` if lhs > rhs.
				  */
				template <class U, std::enable_if_t<std::is_same_v<std::remove_const_t<T>, std::remove_const_t<U>>, bool> = true>
				bool operator>(const BaseIterator<U>& other) const {
					return this->data == other.data && this->offset > other.offset;
				}

				/** @brief	Compare two iterators.
				  *
				  * Two iterators can be compared iff they have the same class type, or one of them
				  * is the const version of the other.
				  * @return	`true` if lhs <= rhs.
				  */
				template <class U, std::enable_if_t<std::is_same_v<std::remove_const_t<T>, std::remove_const_t<U>>, bool> = true>
				bool operator<=(const BaseIterator<U>& other) const {
					return this->data == other.data && this->offset <= other.offset;
				}

				/** @brief	Compare two iterators.
				  *
				  * Two iterators can be compared iff they have the same class type, or one of them
				  * is the const version of the other.
				  * @return	`true` if lhs < rhs.
				  */
				template <class U, std::enable_if_t<std::is_same_v<std::remove_const_t<T>, std::remove_const_t<U>>, bool> = true>
				bool operator<(const BaseIterator<U>& other) const {
					return this->data == other.data && this->offset < other.offset;
				}

				/** @brief	Increment the iterator.
				  * @return	Reference of current iterator after incrementing.
				  */
				BaseIterator& operator++(void) {
					do {
						++this->offset;
					} while ((this->data) && (this->offset < this->data->size()) && (*this->data)[this->offset].removed());
					return *this;
				}

				/** @brief	Increment the iterator.
				  * @return	Copy of current iterator before incrementing.
				  */
				BaseIterator operator++(int) {
					BaseIterator ret = *this;
					this->operator++();
					return ret;
				}

				/** @brief	Decrement the iterator.
				  * @return	Reference of current iterator after decrementing.
				  */
				BaseIterator& operator--(void) {
					do {
						--this->offset;
					} while ((this->data) && (this->offset < this->data->size()) && (*this->data)[this->offset].removed());
					return *this;
				}

				/** @brief	Decrement the iterator.
				  * @return	Copy of current iterator before decrementing.
				  */
				BaseIterator operator--(int) {
					BaseIterator ret = *this;
					this->operator--();
					return ret;
				}

				/** @brief	Add an integer to the iterator.
				  * @return	Reference of current iterator.
				  */
				BaseIterator& operator+=(std::ptrdiff_t n) {
					for (int i = 0; i < n; ++i)
						this->operator++();
					return *this;
				}

				/** @brief	Perform addition with an iterator and an integer.
				  * @return	A new iterator after addition.
				  */
				BaseIterator operator+(std::ptrdiff_t n) const {
					BaseIterator ret = *this;
					ret.operator+=(n);
					return ret;
				}

				/** @brief	Subtract an integer from the iterator.
				  * @return	Reference of current iterator.
				  */
				BaseIterator& operator-=(std::ptrdiff_t n) {
					for (int i = 0; i < n; ++i)
						this->operator--();
					return *this;
				}

				/** @brief	Perform subtraction with an iterator and an integer.
				  * @return	A new iterator after subtraction.
				  */
				BaseIterator operator-(std::ptrdiff_t n) const {
					BaseIterator ret = *this;
					ret.operator-=(n);
					return ret;
				}

				/** @brief	Fetch the current element.
				  * @return	The current element.
				  */
				auto& operator*() const {
					return (*this->data)[this->offset];
				}

				/** @brief	Fetch the current element.
				  * @return	The current element.
				  */
				auto* operator->() const {
					return &(*this->data)[this->offset];
				}

			private:

				T* data;

				std::uint32_t offset;

				/** @brief	Construct from data pointer and index.
				  */
				BaseIterator(T* data, std::uint32_t offset) : data(data), offset(offset) {}

				friend class HalfedgeMesh;
				template <class U> friend class BaseIterator;
				template <class U> friend class BaseRange;

			};

			template <class T> class BaseRange {

			public:

				/** @brief	Default constructor.
				  */
				BaseRange(void) : data(nullptr), removed(nullptr) {}

				/** @brief	Check whether the range is valid or not.
				  * @return `true` if the range is valid.
				  */
				bool valid(void) const {
					return this->data;
				}

				std::size_t size(void) const {
					this->data->size() - this->removed->size();
				}

				/** @brief	Construct from non-const range.
				  *
				  * The current range can be constructed from a non-const range if the current one
				  * is a const one.
				  */
				BaseRange(const BaseRange<std::remove_const_t<T>>& other) : data(other.data), removed(other.removed) { }

				BaseIterator<T> begin(void) const {
					BaseIterator<T> ret(this->data, -1);
					return ++ret;
				}

				BaseIterator<const T> cbegin(void) const {
					BaseIterator<const T> ret(this->data, -1);
					return ++ret;
				}

				BaseIterator<T> end(void) const {
					BaseIterator<T> ret(this->data, this->data->size());
					return ret;
				}

				BaseIterator<const T> cend(void) const {
					BaseIterator<const T> ret(this->data, this->data->size());
					return ret;
				}

			private:

				T* data;
				const std::vector<std::uint32_t>* removed;

				/** @brief	Construct from data pointer.
				  */
				BaseRange(T* data, const std::vector<std::uint32_t>* removed) : data(data), removed(removed) {}

				friend class HalfedgeMesh;
				template <class U> friend class BaseIterator;
				template <class U> friend class BaseRange;

			};

			 /** @defgroup	Element Classes
			   * @brief		Element classes for vertex, halfedge, face, and edge.
			   *
			   * @{
			   */
		public:

			class Vertex {

			public:

				/** @brief Outgoing halfedge.
				  */
				HalfedgeIter halfedge;

				/** @brief Position of the vertex.
				  */
				Vec3 position;

				/** @brief Default constructor.
				  */
				Vertex(void) : _id(-1), halfedge(), position(Vec3::Zero()), _removed(false) {}

				/** @brief Unique identifier.
				  */
				std::uint32_t id(void) const {
					return this->_id;
				}

				/** @brief Whether this element is removed.
				  */
				bool removed(void) const {
					return this->_removed;
				}

				/** @brief Check whether the vertex is on a boundary face.
				  */
				bool onBoundary(void) const {
					bool boundary = false;
					HalfedgeCIter h = this->halfedge;
					do {
						boundary = boundary || h->face->boundary;
						h = h->twin->next;
					} while (!boundary && h != this->halfedge);
					return boundary;
				}

				/** @brief Compute the degree of the vertex.
				  */
				std::uint32_t degree(void) const {
					uint32_t deg = 0;
					HalfedgeCIter h = this->halfedge;
					do {
						++deg;
						h = h->twin->next;
					} while (h != this->halfedge);
					return deg;
				}

				/** @brief Get element name.
				  */
				std::string name(void) const {
					return "v" + std::to_string(this->_id);
				}

				/** @brief Helper printing function.
				  */
				friend std::ostream& operator<<(std::ostream& out, const Vertex& v) {
					out << "v" << v._id;
					return out;
				}

			private:

				/** @brief Unique identifier.
				  */
				std::uint32_t _id;

				/** @brief Whether this element is removed.
				  */
				bool _removed;

				friend class HalfedgeMesh;

			};

			class Halfedge {

			public:

				/** @brief Next halfedge.
				  */
				HalfedgeIter next;

				/** @brief Previous halfedge.
				  */
				HalfedgeIter prev;

				/** @brief Twin halfedge.
				  */
				HalfedgeIter twin;

				/** @brief The vertex this halfedge is leaving.
				  */
				VertexIter source;

				/** @brief The edge this halfedge belongs to.
				  */
				EdgeIter edge;

				/** @brief The face this halfedge belongs to.
				  */
				FaceIter face;

				/** @brief The texture coordinate of the source vertex.
				  */
				Vec2 uv;

				/** @brief The normal of the source vertex.
				  */
				Vec3 normal;

				/** @brief The tangent of the source vertex.
				  */
				Vec3 tangent;

				/** @brief Default constructor.
				  */
				Halfedge(void) : _id(-1), next(), prev(), twin(), source(), edge(), face(), uv(Vec2::Zero()), normal(Vec3::Zero()), tangent(Vec3::Zero()), _removed(false) {}
				
				/** @brief Unique identifier.
				  */
				std::uint32_t id(void) const {
					return this->_id;
				}

				/** @brief Whether this element is removed.
				  */
				bool removed(void) const {
					return this->_removed;
				}
				
				/** @brief Check whether the halfedge is on a boundary face.
				  */
				bool onBoundary(void) const {
					return this->face->boundary;
				}

				/** @brief Get the vector from the source vertex to the destination vertex.
				  */
				Vec3 vector(void) const {
					return this->twin->source->position - this->source->position;
				}

				/** @brief Get element name.
				  */
				std::string name(void) const {
					return "h" + std::to_string(this->_id);
				}
				
				/** @brief Helper printing function.
				  */
				friend std::ostream& operator<<(std::ostream& out, const Halfedge& h) {
					out << "h" << h._id;
					return out;
				}

			private:

				/** @brief Unique identifier.
				  */
				std::uint32_t _id;

				/** @brief Whether this element is removed.
				  */
				bool _removed;

				friend class HalfedgeMesh;

			};

			class Face {

			public:

				/** @brief A halfedge in this face.
				  */
				HalfedgeIter halfedge;

				/** @brief Whether this element is a boundary face.
				  */
				bool boundary;

				/** @brief Default constructor.
				  */
				Face(void) : _id(-1), halfedge(), _removed(false), boundary(false) {}

				/** @brief Unique identifier.
				  */
				std::uint32_t id(void) const {
					return this->_id;
				}

				/** @brief Whether this element is removed.
				  */
				bool removed(void) const {
					return this->_removed;
				}
				
				/** @brief Compute the degree of the face.
				  */
				std::uint32_t degree(void) const {
					uint32_t deg = 0;
					HalfedgeCIter h = this->halfedge;
					do {
						++deg;
						h = h->next;
					} while (h != this->halfedge);
					return deg;
				}

				/** @brief Get element name.
				  */
				std::string name(void) const {
					return "f" + std::to_string(this->_id);
				}

				/** @brief Helper printing function.
				  */
				friend std::ostream& operator<<(std::ostream& out, const Face& f) {
					out << "f" << f._id;
					return out;
				}

			private:

				/** @brief Unique identifier.
				  */
				std::uint32_t _id;

				/** @brief Whether this element is removed.
				  */
				bool _removed;

				friend class HalfedgeMesh;

			};

			class Edge {

			public:

				/** @brief A halfedge in this edge.
				  */
				HalfedgeIter halfedge;

				/** @brief Default constructor.
				  */
				Edge(void) : _id(-1), halfedge(), _removed(false) {}

				/** @brief Unique identifier.
				  */
				std::uint32_t id(void) const {
					return this->_id;
				}

				/** @brief Whether this element is removed.
				  */
				bool removed(void) const {
					return this->_removed;
				}
				
				/** @brief Check whether the edge is on a boundary face.
				  */
				bool onBoundary(void) const {
					return this->halfedge->face->boundary || this->halfedge->twin->face->boundary;
				}

				/** @brief Get element name.
				  */
				std::string name(void) const {
					return "e" + std::to_string(this->_id);
				}

				/** @brief Helper printing function.
				  */
				friend std::ostream& operator<<(std::ostream& out, const Edge& e) {
					out << "e" << e._id;
					return out;
				}

			private:

				/** @brief Unique identifier.
				  */
				std::uint32_t _id;

				/** @brief Whether this element is removed.
				  */
				bool _removed;

				friend class HalfedgeMesh;

			};

			/** @}
			  */

		public:

			template <class T>
			BaseIterator<std::vector<T>> emplace_back(void);

			template <class T>
			BaseIterator<std::vector<T>> emplace(void);

			std::size_t numVertices(void) const {
				return this->_vertices.size() - this->_removedVertices.size();
			}

			VertexRange vertices(void) {
				return VertexRange(&this->_vertices, &this->_removedVertices);
			}

			VertexCRange vertices(void) const {
				return VertexCRange(&this->_vertices, &this->_removedVertices);
			}

			VertexIter vertex(std::uint32_t offset) {
				return VertexIter(&this->_vertices, offset);
			}

			VertexCIter vertex(std::uint32_t offset) const {
				return VertexCIter(&this->_vertices, offset);
			}

			/** @brief	Emplace a vertex after the last position.
			  * @return	Iterator pointing to the created vertex.
			  */
			template <>
			VertexIter emplace_back<Vertex>(void) {
				std::uint32_t offset = this->_vertices.size();
				this->_vertices.emplace_back();
				VertexIter ret(&this->_vertices, offset);
				ret->_id = this->idCnt++;
				return ret;
			}

			/** @brief	Emplace a vertex (may reuse the memory of a deleted vertex).
			  * @return	Iterator pointing to the created vertex.
			  */
			template <>
			VertexIter emplace<Vertex>(void) {
				std::uint32_t offset;
				if (this->_removedVertices.empty()) {
					offset = this->_vertices.size();
					this->_vertices.emplace_back();
				}
				else {
					offset = this->_removedVertices.back();
					this->_removedVertices.pop_back();
				}
				VertexIter ret(&this->_vertices, offset);
				*ret = Vertex();
				ret->_id = this->idCnt++;
				return ret;
			}

			/** @brief	Remove the given vertex.
			  * @return	`true` if successfully removed; `false` if already removed or invalid iterator.
			  */
			bool remove(VertexIter v) {
				if (v.data == &this->_vertices && v.offset < this->_vertices.size() && !v->_removed) {
					v->_removed = true;
					this->_removedVertices.push_back(v.offset);
					return true;
				}
				else return false;
			}

			std::size_t numHalfedges(void) const {
				return this->_halfedges.size() - this->_removedHalfedges.size();
			}

			HalfedgeRange halfedges(void) {
				return HalfedgeRange(&this->_halfedges, &this->_removedHalfedges);
			}

			HalfedgeCRange halfedges(void) const {
				return HalfedgeCRange(&this->_halfedges, &this->_removedHalfedges);
			}

			HalfedgeIter halfedge(std::uint32_t offset) {
				return HalfedgeIter(&this->_halfedges, offset);
			}

			HalfedgeCIter halfedge(std::uint32_t offset) const {
				return HalfedgeCIter(&this->_halfedges, offset);
			}

			/** @brief	Emplace a halfedge after the last position.
			  * @return	Iterator pointing to the created halfedge.
			  */
			template <>
			HalfedgeIter emplace_back<Halfedge>(void) {
				std::uint32_t offset = this->_halfedges.size();
				this->_halfedges.emplace_back();
				HalfedgeIter ret(&this->_halfedges, offset);
				ret->_id = this->idCnt++;
				return ret;
			}

			/** @brief	Emplace a halfedge (may reuse the memory of a deleted halfedge).
			  * @return	Iterator pointing to the created halfedge.
			  */
			template <>
			HalfedgeIter emplace<Halfedge>(void) {
				std::uint32_t offset;
				if (this->_removedHalfedges.empty()) {
					offset = this->_halfedges.size();
					this->_halfedges.emplace_back();
				}
				else {
					offset = this->_removedHalfedges.back();
					this->_removedHalfedges.pop_back();
				}
				HalfedgeIter ret(&this->_halfedges, offset);
				*ret = Halfedge();
				ret->_id = this->idCnt++;
				return ret;
			}

			/** @brief	Remove the given halfedge.
			  * @return	`true` if successfully removed; `false` if already removed or invalid iterator.
			  */
			bool remove(HalfedgeIter h) {
				if (h.data == &this->_halfedges && h.offset < this->_halfedges.size() && !h->_removed) {
					h->_removed = true;
					this->_removedHalfedges.push_back(h.offset);
					return true;
				}
				else return false;
			}

			std::size_t numFaces(void) const {
				return this->_faces.size() - this->_removedFaces.size();
			}

			FaceRange faces(void) {
				return FaceRange(&this->_faces, &this->_removedFaces);
			}

			FaceCRange faces(void) const {
				return FaceCRange(&this->_faces, &this->_removedFaces);
			}

			FaceIter face(std::uint32_t offset) {
				return FaceIter(&this->_faces, offset);
			}

			FaceCIter face(std::uint32_t offset) const {
				return FaceCIter(&this->_faces, offset);
			}

			/** @brief	Emplace a face after the last position.
			  * @return	Iterator pointing to the created face.
			  */
			template <>
			FaceIter emplace_back<Face>(void) {
				std::uint32_t offset = this->_faces.size();
				this->_faces.emplace_back();
				FaceIter ret(&this->_faces, offset);
				ret->_id = this->idCnt++;
				return ret;
			}

			/** @brief	Emplace a face (may reuse the memory of a deleted face).
			  * @return	Iterator pointing to the created face.
			  */
			template <>
			FaceIter emplace<Face>(void) {
				std::uint32_t offset;
				if (this->_removedFaces.empty()) {
					offset = this->_faces.size();
					this->_faces.emplace_back();
				}
				else {
					offset = this->_removedFaces.back();
					this->_removedFaces.pop_back();
				}
				FaceIter ret(&this->_faces, offset);
				*ret = Face();
				ret->_id = this->idCnt++;
				return ret;
			}

			/** @brief	Remove the given face.
			  * @return	`true` if successfully removed; `false` if already removed or invalid iterator.
			  */
			bool remove(FaceIter f) {
				if (f.data == &this->_faces && f.offset < this->_faces.size() && !f->_removed) {
					f->_removed = true;
					this->_removedFaces.push_back(f.offset);
					return true;
				}
				else return false;
			}

			std::size_t numEdges(void) const {
				return this->_edges.size() - this->_removedEdges.size();
			}

			EdgeRange edges(void) {
				return EdgeRange(&this->_edges, &this->_removedEdges);
			}

			EdgeCRange edges(void) const {
				return EdgeCRange(&this->_edges, &this->_removedEdges);
			}

			EdgeIter edge(std::uint32_t offset) {
				return EdgeIter(&this->_edges, offset);
			}

			EdgeCIter edge(std::uint32_t offset) const {
				return EdgeCIter(&this->_edges, offset);
			}

			/** @brief	Emplace an edge after the last position.
			  * @return	Iterator pointing to the created edge.
			  */
			template <>
			EdgeIter emplace_back<Edge>(void) {
				std::uint32_t offset = this->_edges.size();
				this->_edges.emplace_back();
				EdgeIter ret(&this->_edges, offset);
				ret->_id = this->idCnt++;
				return ret;
			}

			/** @brief	Emplace an edge (may reuse the memory of a deleted edge).
			  * @return	Iterator pointing to the created edge.
			  */
			template <>
			EdgeIter emplace<Edge>(void) {
				std::uint32_t offset;
				if (this->_removedEdges.empty()) {
					offset = this->_edges.size();
					this->_edges.emplace_back();
				}
				else {
					offset = this->_removedEdges.back();
					this->_removedEdges.pop_back();
				}
				EdgeIter ret(&this->_edges, offset);
				*ret = Edge();
				ret->_id = this->idCnt++;
				return ret;
			}

			/** @brief	Remove the given edge.
			  * @return	`true` if successfully removed; `false` if already removed or invalid iterator.
			  */
			bool remove(EdgeIter e) {
				if (e.data == &this->_edges && e.offset < this->_edges.size() && !e->_removed) {
					e->_removed = true;
					this->_removedEdges.push_back(e.offset);
					return true;
				}
				else return false;
			}

			HalfedgeMesh(void) : idCnt(0) {}

			/** @brief	Remove all elements in the mesh.
			  */
			void clear(void) {
				this->_vertices.clear(); this->_removedVertices.clear();
				this->_halfedges.clear(); this->_removedHalfedges.clear();
				this->_faces.clear(); this->_removedFaces.clear();
				this->_edges.clear(); this->_removedEdges.clear();
				this->idCnt = 0;
			}

			/** @brief Garbage collection.
			  * 
			  * HalfedgeMesh class uses a lazy way to remove elements. When vertices/halfedges/faces/edges
			  * are removed, they are simply "labeled" to be removed, but are not actually deleted from the memory.
			  * In order to clean the memory, you need to explicitly call this method.
			  * Once finished, all existing iterators will be invalidated.
			  */
			void collectGarbage(void);

			/** @brief	Convert IndexedMesh to HalfedgeMesh.
			  * @return	`true` if successfully. The conversion will fail if the mesh is not a manifold.
			  */
			bool fromIndexedMesh(const IndexedMesh<FP>& indexedMesh);

			/** @brief	Compute face normals.
			  *			Compute non-boundary face normals and store them in Halfedge::normal.
			  *			All halfedges around a face will have the same normal.
			  *			It is better to triangulate the mesh before calling this method.
			  * @sa		jjyou::geo::HalfedgeMesh::computeVertexNormals
			  */
			void computeFaceNormals(void);

			/** @brief	Compute vertex normals.
			  *			Compute vertex normals and store them in Halfedge::normal. The vertex normals
			  *			are computed as the average of incident non-boundary faces' normals.
			  *			It is better to triangulate the mesh before calling this method.
			  * @sa		jjyou::geo::HalfedgeMesh::computeFaceNormals
			  */
			void computeVertexNormals(void);

			/** @brief	Compute tangents.
			  *			Compute tangents and store them in Halfedge::tangent. The mesh
			  *			must have uv coordinates.
			  * @sa		https://learnopengl.com/Advanced-Lighting/Normal-Mapping
			  */
			void computeTangents(void);

			/** @brief	Validate. Only for debugging.
			  * @return	Empty if valid, otherwise the reason for invalidity.
			  */
			std::string validate(void) const;

		private:

			std::vector<Vertex> _vertices; std::vector<std::uint32_t> _removedVertices;
			std::vector<Halfedge> _halfedges; std::vector<std::uint32_t> _removedHalfedges;
			std::vector<Face> _faces; std::vector<std::uint32_t> _removedFaces;
			std::vector<Edge> _edges; std::vector<std::uint32_t> _removedEdges;
			std::uint32_t idCnt;

			template <class _FP> friend class IndexedMesh;

		};

		HalfedgeMesh<float>;
		HalfedgeMesh<double>;

	}
}

#define JJYOU_GEO_HALFEDGEMESH_HASH_ITER(T) \
template <> struct ::std::hash<typename T> { \
	using argument_type = T; \
	using result_type = size_t; \
	result_type operator()(argument_type const& key) const { \
		static const ::std::hash<decltype(&*key)> h; \
		return h(&*key); \
	} \
}

#define JJYOU_GEO_HALFEDGEMESH_HASH_ITERS(FP) \
JJYOU_GEO_HALFEDGEMESH_HASH_ITER(::jjyou::geo::HalfedgeMesh<FP>::VertexIter); \
JJYOU_GEO_HALFEDGEMESH_HASH_ITER(::jjyou::geo::HalfedgeMesh<FP>::VertexCIter); \
JJYOU_GEO_HALFEDGEMESH_HASH_ITER(::jjyou::geo::HalfedgeMesh<FP>::HalfedgeIter); \
JJYOU_GEO_HALFEDGEMESH_HASH_ITER(::jjyou::geo::HalfedgeMesh<FP>::HalfedgeCIter); \
JJYOU_GEO_HALFEDGEMESH_HASH_ITER(::jjyou::geo::HalfedgeMesh<FP>::FaceIter); \
JJYOU_GEO_HALFEDGEMESH_HASH_ITER(::jjyou::geo::HalfedgeMesh<FP>::FaceCIter); \
JJYOU_GEO_HALFEDGEMESH_HASH_ITER(::jjyou::geo::HalfedgeMesh<FP>::EdgeIter); \
JJYOU_GEO_HALFEDGEMESH_HASH_ITER(::jjyou::geo::HalfedgeMesh<FP>::EdgeCIter)

JJYOU_GEO_HALFEDGEMESH_HASH_ITERS(float);
JJYOU_GEO_HALFEDGEMESH_HASH_ITERS(double);


#endif /* jjyou_geo_HalfedgeMesh_hpp */