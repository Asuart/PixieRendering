#pragma once
#include <cstdint>

enum class TextureType : int32_t {
	Texture2D,
	Texture3D,
	Cubemap
};

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

enum class TextureFormat : int32_t {
	Red8,
	RGB8,
	RGBA8,
	Red32f,
	RGB32f,
	RGBA32f
};
