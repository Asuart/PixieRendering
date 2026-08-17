#pragma once 
#include <cstdint>
#include <string>
#include <vector>

namespace PixieRenderer {

enum class ShaderResourceType : uint8_t {
	Undefined,
	Attribute,    
	Uniform,      
	UniformBuffer, 
	Sampler2D,      
	StorageBuffer,
};

struct ShaderBinding {
	std::string name = "";
	ShaderResourceType type = ShaderResourceType::Undefined;
	uint32_t binding = UINT32_MAX;
	uint32_t set = UINT32_MAX;
	uint32_t size = UINT32_MAX;
	uint32_t count = UINT32_MAX;
	uint32_t stageFlags = UINT32_MAX;

	ShaderBinding() = default;

	ShaderBinding(
	    const std::string& _name,
		ShaderResourceType _type,
	    uint32_t _binding,
	    uint32_t _set,
	    uint32_t _size,
	    uint32_t _count,
	    uint32_t _stageFlags
	);
};

struct SpirVBinary {
	uint32_t* words;
	int32_t size;
};

struct BindingsInfo {
	std::vector<ShaderBinding> bindings;
	std::vector<uint32_t> uniformBufferBindings;
};

class ShaderCompiler {
  public:

};

} // namespace PixieRenderer
