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

template <ResourceType t> struct ResourceHandle {
	uint64_t m_id = UINT64_MAX;
	IResourceManager* m_manager = nullptr;

  public:
	static constexpr ResourceType type = t;

	ResourceHandle() = default;

	ResourceHandle(IResourceManager* mgr, uint64_t id) : m_manager(mgr), m_id(id) {
		if (m_manager && m_id != UINT64_MAX) {
			manager->AddRef(id);
		}
	}

	ResourceHandle(const ResourceHandle& other) : m_manager(other.m_manager), m_id(other.m_id) {
		if (m_manager && m_id != UINT64_MAX) {
			m_manager->AddRef(m_id);
		}
	}

	ResourceHandle(ResourceHandle&& other) noexcept : m_manager(other.m_manager), m_id(other.m_id) {
		other.m_manager = nullptr;
		other.m_id = UINT64_MAX;
	}

	~ResourceHandle() {
		if (m_manager && m_id != UINT64_MAX) {
			m_manager->Release(m_id);
		}
	}

	ResourceHandle& operator=(const ResourceHandle& other) {
		if (this != &other) {
			if (m_manager && m_id != UINT64_MAX) {
				m_manager->Release(m_id);
			}
			m_manager = other.m_manager;
			m_id = other.m_id;
			if (m_manager && m_id != UINT64_MAX) {
				m_manager->AddRef(m_id);
			}
		}
		return *this;
	}

	explicit operator bool() const {
		return m_id != UINT64_MAX && m_manager;
	}

	void Reset() {
		if (m_manager && m_id != UINT64_MAX) {
			m_manager->Release(m_id);
		}
		m_manager = nullptr;
		m_id = UINT64_MAX;
	}

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
