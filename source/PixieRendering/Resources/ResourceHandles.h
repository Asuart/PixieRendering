#pragma once
#include <cstdint>

namespace PixieRenderer {

enum class ResourceType : uint32_t {
	Texture = 0,
	Mesh,
	Material,
	ComputeProgram,
	FrameBuffer,
	ShaderStorageBuffer,
	UniformBuffer
};

class IResourceManager;

template <ResourceType t> class ResourceHandle {
	uint64_t m_id = UINT64_MAX;
	IResourceManager* m_manager = nullptr;

  public:
	static constexpr ResourceType type = t;

	ResourceHandle() = default;

	ResourceHandle(IResourceManager* mgr, uint64_t id);
	ResourceHandle(const ResourceHandle& other);
	ResourceHandle(ResourceHandle&& other) noexcept;
	~ResourceHandle();

	ResourceHandle& operator=(const ResourceHandle& other);
	ResourceHandle& operator=(ResourceHandle&& other) noexcept;

	explicit operator bool() const;
	void Reset();

	inline uint64_t GetId() const {
		return m_id;
	}

	inline IResourceManager* GetManager() const {
		return m_manager;
	}
};

using TextureHandle = ResourceHandle<ResourceType::Texture>;
using MeshHandle = ResourceHandle<ResourceType::Mesh>;
using MaterialHandle = ResourceHandle<ResourceType::Material>;
using ComputeProgramHandle = ResourceHandle<ResourceType::ComputeProgram>;
using FrameBufferHandle = ResourceHandle<ResourceType::FrameBuffer>;
using ShaderStorageBufferHandle = ResourceHandle<ResourceType::ShaderStorageBuffer>;
using UniformBufferHandle = ResourceHandle<ResourceType::UniformBuffer>;

} // namespace PixieRenderer
