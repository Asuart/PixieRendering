#pragma once
#include <cstdint>

namespace PixieRenderer {

struct TextureHandle {
	uint64_t id = UINT64_MAX;

	TextureHandle() = default;
	explicit TextureHandle(uint64_t id) : id(id) {
	}

	explicit operator bool() const {
		return id != UINT64_MAX;
	}
};

struct MeshHandle {
	uint64_t id = UINT64_MAX;

	MeshHandle() = default;
	explicit MeshHandle(uint64_t id) : id(id) {
	}

	explicit operator bool() const {
		return id != UINT64_MAX;
	}
};

struct MaterialHandle {
	uint64_t id = UINT64_MAX;

	MaterialHandle() = default;
	explicit MaterialHandle(uint64_t id) : id(id) {
	}

	explicit operator bool() const {
		return id != UINT64_MAX;
	}
};

struct ComputeProgramHandle {
	uint64_t id = UINT64_MAX;

	ComputeProgramHandle() = default;
	explicit ComputeProgramHandle(uint64_t id) : id(id) {
	}

	explicit operator bool() const {
		return id != UINT64_MAX;
	}
};

struct FrameBufferHandle {
	uint64_t id = UINT64_MAX;

	FrameBufferHandle() = default;
	explicit FrameBufferHandle(uint64_t id) : id(id) {
	}

	explicit operator bool() const {
		return id != UINT64_MAX;
	}
};

struct ShaderStorageBufferHandle {
	uint64_t id = UINT64_MAX;

	ShaderStorageBufferHandle() = default;
	explicit ShaderStorageBufferHandle(uint64_t id) : id(id) {
	}

	explicit operator bool() const {
		return id != UINT64_MAX;
	}
};

struct UniformBufferHandle {
	uint64_t id = UINT64_MAX;

	UniformBufferHandle() = default;
	explicit UniformBufferHandle(uint64_t id) : id(id) {
	}

	explicit operator bool() const {
		return id != UINT64_MAX;
	}
};

} // namespace PixieRenderer
