#pragma once
#include <cstdint>

struct TextureHandle {
    uint64_t id = 0;

    TextureHandle() = default;
	explicit TextureHandle(uint64_t id) : id(id) {}
};

struct MeshHandle {
	uint64_t id = 0;

	MeshHandle() = default;
	explicit MeshHandle(uint64_t id) : id(id) {}
};

struct ShaderHandle {
	uint64_t id = 0;

	ShaderHandle() = default;
	explicit ShaderHandle(uint64_t id) : id(id) {}
};

struct ComputeShaderHandle {
	uint64_t id = 0;

	ComputeShaderHandle() = default;
	explicit ComputeShaderHandle(uint64_t id) : id(id) {}
};

struct FrameBufferHandle {
	uint64_t id = 0;

	FrameBufferHandle() = default;
	explicit FrameBufferHandle(uint64_t id) : id(id) {}
};

struct ShaderStorageBufferHandle {
    uint64_t id = 0;

	ShaderStorageBufferHandle() = default;
	explicit ShaderStorageBufferHandle(uint64_t id) : id(id) {}
};

struct UniformBufferHandle {
	uint64_t id = 0;

	UniformBufferHandle() = default;
	explicit UniformBufferHandle(uint64_t id) : id(id) {}
};
