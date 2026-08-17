#pragma once
#include <cstdint>

namespace PixieRenderer {

struct TextureHandle {
	uint64_t id = UINT64_MAX;

	TextureHandle() = default;
	explicit TextureHandle(uint64_t id) : id(id) {
	}
};

struct MeshHandle {
	uint64_t id = UINT64_MAX;

	MeshHandle() = default;
	explicit MeshHandle(uint64_t id) : id(id) {
	}
};

struct MaterialHandle {
	uint64_t id = UINT64_MAX;

	MaterialHandle() = default;
	explicit MaterialHandle(uint64_t id) : id(id) {
	}
};

struct ComputeProgramHandle {
	uint64_t id = UINT64_MAX;

	ComputeProgramHandle() = default;
	explicit ComputeProgramHandle(uint64_t id) : id(id) {
	}
};

struct FrameBufferHandle {
	uint64_t id = UINT64_MAX;

	FrameBufferHandle() = default;
	explicit FrameBufferHandle(uint64_t id) : id(id) {
	}
};

struct ShaderStorageBufferHandle {
	uint64_t id = UINT64_MAX;

	ShaderStorageBufferHandle() = default;
	explicit ShaderStorageBufferHandle(uint64_t id) : id(id) {
	}
};

struct UniformBufferHandle {
	uint64_t id = UINT64_MAX;

	UniformBufferHandle() = default;
	explicit UniformBufferHandle(uint64_t id) : id(id) {
	}
};

} // namespace PixieRenderer
