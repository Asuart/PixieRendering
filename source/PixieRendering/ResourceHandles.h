#pragma once
#include <cstdint>

namespace PixieRenderer {

struct TextureHandle {
	uint64_t id = 0;

	TextureHandle() = default;
	explicit TextureHandle(uint64_t id) : id(id) {
	}
};

struct MeshHandle {
	uint64_t id = 0;

	MeshHandle() = default;
	explicit MeshHandle(uint64_t id) : id(id) {
	}
};

struct MaterialHandle {
	uint64_t id = 0;

	MaterialHandle() = default;
	explicit MaterialHandle(uint64_t id) : id(id) {
	}
};

struct ComputeProgramHandle {
	uint64_t id = 0;

	ComputeProgramHandle() = default;
	explicit ComputeProgramHandle(uint64_t id) : id(id) {
	}
};

struct FrameBufferHandle {
	uint64_t id = 0;

	FrameBufferHandle() = default;
	explicit FrameBufferHandle(uint64_t id) : id(id) {
	}
};

struct ShaderStorageBufferHandle {
	uint64_t id = 0;

	ShaderStorageBufferHandle() = default;
	explicit ShaderStorageBufferHandle(uint64_t id) : id(id) {
	}
};

struct UniformBufferHandle {
	uint64_t id = 0;

	UniformBufferHandle() = default;
	explicit UniformBufferHandle(uint64_t id) : id(id) {
	}
};

} // namespace PixieRenderer
