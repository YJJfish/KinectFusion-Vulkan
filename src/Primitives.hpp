#pragma once
#include <vulkan/vulkan_raii.hpp>
#include <jjyou/vk/Vulkan.hpp>
#include <jjyou/glsl/glsl.hpp>

/***********************************************************************
 * @enum	MaterialType
 * @brief	Enum used to determine the material type.
 *
 *			The enum is defined as C enum instead of C++ enum class
 *			so that you can directly use it to index into an array.
 ***********************************************************************/
enum MaterialType : std::size_t {
	Simple,				/**< Primitives that have position and color attributes. */
	Lambertian,			/**< Primitives that have position, normal, and color attributes. */
	NumMaterialTypes,	/**< Used to indicate the number of material types. */
};

/***********************************************************************
 * @enum	PrimitiveType
 * @brief	Enum used to determine the primitive type.
 *
 *			The enum is defined as C enum instead of C++ enum class
 *			so that you can directly use it to index into an array.
 ***********************************************************************/
enum PrimitiveType : std::size_t {
	Point,
	Line,
	Triangle,
	NumPrimitiveTypes,	/**< Used to indicate the number of primitive types. */
};

/***********************************************************************
 * @class	Vertex
 * @brief	Vertex class that provides vertex binding information of
 *			vertex shaders
 ***********************************************************************/
template <MaterialType _materialType>
struct Vertex;

/***********************************************************************
 * @class	Vertex
 * @brief	Specialization of vertex class for simple material.
 ***********************************************************************/
template <>
struct Vertex<MaterialType::Simple> {

	using vec3f = jjyou::glsl::vec<float, 3>;
	using vec4uc = jjyou::glsl::vec<unsigned char, 4>;

	vec3f position{};
	vec4uc color{};

	static constexpr vk::VertexInputBindingDescription getInputBindingDescription(void) {
		return vk::VertexInputBindingDescription(
			0,
			sizeof(Vertex<MaterialType::Simple>),
			vk::VertexInputRate::eVertex
		);
	}

	static constexpr std::vector<vk::VertexInputAttributeDescription> getInputAttributeDescriptions(void) {
		std::vector<vk::VertexInputAttributeDescription> ret{
			// layout(location = 0) in vec3 inPosition
			vk::VertexInputAttributeDescription(
				0,
				0,
				vk::Format::eR32G32B32Sfloat,
				offsetof(Vertex, position)
			),
			// layout(location = 1) in vec4 inColor
			vk::VertexInputAttributeDescription(
				1,
				0,
				vk::Format::eR8G8B8A8Unorm,
				offsetof(Vertex, color)
			),
		};
		return ret;
	}

};

/***********************************************************************
 * @class	Vertex
 * @brief	Specialization of vertex class for lambertian material.
 ***********************************************************************/
template <>
struct Vertex<MaterialType::Lambertian> {

	using vec3f = jjyou::glsl::vec<float, 3>;
	using vec4uc = jjyou::glsl::vec<unsigned char, 4>;

	vec3f position{};
	vec3f normal{};
	vec4uc color{};

	static constexpr vk::VertexInputBindingDescription getInputBindingDescription(void) {
		return vk::VertexInputBindingDescription(
			0,
			sizeof(Vertex<MaterialType::Lambertian>),
			vk::VertexInputRate::eVertex
		);
	}

	static constexpr std::vector<vk::VertexInputAttributeDescription> getInputAttributeDescriptions(void) {
		std::vector<vk::VertexInputAttributeDescription> ret{
			// layout(location = 0) in vec3 inPosition;
			vk::VertexInputAttributeDescription(
				0,
				0,
				vk::Format::eR32G32B32Sfloat,
				offsetof(Vertex, position)
			),
			// layout(location = 1) in vec3 inNormal;
			vk::VertexInputAttributeDescription(
				1,
				0,
				vk::Format::eR32G32B32Sfloat,
				offsetof(Vertex, normal)
			),
			// layout(location = 2) in vec4 inColor;
			vk::VertexInputAttributeDescription(
				2,
				0,
				vk::Format::eR8G8B8A8Unorm,
				offsetof(Vertex, color)
			),
		};
		return ret;
	}

};

class Engine;

/***********************************************************************
 * @class	Primitives
 * @brief	Primitives class that manages relevant Vulkan resources for rendering
 *			a collection of primitives of the same primitive and material type
 *			in the scene.
 *			Primitives are owned by the graphics queue family. If you wish to access
 *			it in other queue families like compute, you need to do the following steps:
 *			1. Release the ownership in a graphics command buffer.
 *			2. Acquire the ownership in a compute command buffer.
 *			3. Do your processing.
 *			4. Release the ownership in a compute command buffer.
 *			5. Acquire the ownership in a graphics command buffer.
 ***********************************************************************/
template <MaterialType _materialType, PrimitiveType _primitiveType>
class Primitives {

public:

	static inline constexpr MaterialType materialType = _materialType;
	static inline constexpr PrimitiveType primitiveType = _primitiveType;

	/** @brief	Construct an empty collection of primitives (in invalid state).
	  */
	Primitives(std::nullptr_t) {}

	/** @brief	Copy constructor is disabled.
	  */
	Primitives(const Primitives&) = delete;

	/** @brief	Move constructor.
	  */
	Primitives(Primitives&& other_) = default;

	/** @brief	Destructor.
	  */
	~Primitives(void) = default;

	/** @brief	Copy assignment is disabled.
	  */
	Primitives& operator=(const Primitives&) = delete;

	/** @brief	Move assignment.
	  */
	Primitives& operator=(Primitives&& other_) noexcept {
		if (this != &other_) {
			this->_pEngine = other_._pEngine;
			this->_vertexBuffer = std::move(other_._vertexBuffer);
			this->_vertexBufferMemory = std::move(other_._vertexBufferMemory);
			this->_numVertices = other_._numVertices;
		}
		return *this;
	}

	/** @brief	Construct an empty collection of primitives.
	  */
	Primitives(const Engine& engine_) : _pEngine(&engine_) {}

	/** @brief	Set the vertex buffer from CPU data.
	  */
	template <size_t _length>
	Primitives& setVertexData(const std::array<Vertex<_materialType>, _length>& data_, bool waitIdle_) {
		return this->setVertexData(data_.data(), static_cast<std::uint32_t>(data_.size()), waitIdle_);
	}

	/** @brief	Set the vertex buffer from CPU data.
	  */
	Primitives& setVertexData(const std::vector<Vertex<_materialType>>& data_, bool waitIdle_) {
		return this->setVertexData(data_.data(), static_cast<std::uint32_t>(data_.size()), waitIdle_);
	}

	/** @brief	Set the vertex buffer from CPU data.
	  */
	Primitives& setVertexData(const Vertex<_materialType>* data_, std::uint32_t numVertices_, bool waitIdle_);

	/** @brief	Get the number of vertices.
	  */
	std::uint32_t numVertices(void) const {
		return this->_numVertices;
	}

	/** @brief	Bind vertex buffer and draw the primitives.
	  */
	void draw(const vk::raii::CommandBuffer& commandBuffer_) const {
		commandBuffer_.bindVertexBuffers(0, *this->_vertexBuffer, vk::DeviceSize(0));
		commandBuffer_.draw(this->_numVertices, 1, 0, 0);
	}

protected:

	const Engine* _pEngine = nullptr;
	vk::raii::Buffer _vertexBuffer{ nullptr };
	jjyou::vk::VmaAllocation _vertexBufferMemory{ nullptr };
	std::uint32_t _numVertices = 0U;

};