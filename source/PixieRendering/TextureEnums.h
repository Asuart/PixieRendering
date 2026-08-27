#pragma once
#include <cstdint>

namespace PixieRenderer {

enum class TextureType : int32_t { Texture2D, Texture3D, Cubemap };

enum class TextureWrap : int32_t {
	Reapeat,
	MirroredRepeat,
	ClampToEdge,
	ClampToBorder,
};

enum class TextureFiltering : int32_t {
	Nearest,
	Linear,
	NearestMipmapNearest,
	LinearMipmapNearest,
	NearestMipmapLinear,
	LinearMipmapLinear,
};

enum class TextureFormat : int32_t { Red8, RGB8, RGBA8, Red32f, RGB32f, RGBA32f };

static inline uint32_t FormatToByteSize(TextureFormat format) {
	switch (format) {
	case TextureFormat::Red8:
		return 1;
	case TextureFormat::RGB8:
		return 3;
	case TextureFormat::RGBA8:
		return 4;
	case TextureFormat::Red32f:
		return 4;
	case TextureFormat::RGB32f:
		return 12;
	case TextureFormat::RGBA32f:
		return 16;
	default:
		return 16;
	}
}

} // namespace PixieRenderer
