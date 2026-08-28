#include "ResourceHandles.h"

namespace PixieRenderer {

ResourceHandle::ResourceHandle(IResourceManager* mgr, uint64_t id) : m_manager(mgr), m_id(id) {
	if (m_manager && m_id != UINT64_MAX) {
		manager->AddRef(id);
	}
}

ResourceHandle::ResourceHandle(const ResourceHandle& other)
    : m_manager(other.m_manager), m_id(other.m_id) {
	if (m_manager && m_id != UINT64_MAX) {
		m_manager->AddRef(m_id);
	}
}

ResourceHandle::ResourceHandle(ResourceHandle&& other) noexcept
    : m_manager(other.m_manager), m_id(other.m_id) {
	other.m_manager = nullptr;
	other.m_id = UINT64_MAX;
}

ResourceHandle::~ResourceHandle() {
	if (m_manager && m_id != UINT64_MAX) {
		m_manager->Release(m_id);
	}
}

ResourceHandle::ResourceHandle& operator=(const ResourceHandle& other) {
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

 operator ResourceHandle::bool() const {
	return m_id != UINT64_MAX && m_manager;
}

void ResourceHandle::Reset() {
	if (m_manager && m_id != UINT64_MAX) {
		m_manager->Release(m_id);
	}
	m_manager = nullptr;
	m_id = UINT64_MAX;
}

inline uint64_t ResourceHandle::GetId() const {
	return m_id;
}

inline IResourceManager* ResourceHandle::GetManager() const {
	return m_manager;
}

} // namespace PixieRenderer
