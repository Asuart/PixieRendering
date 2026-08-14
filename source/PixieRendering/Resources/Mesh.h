#pragma once
#include <vector>
#include <glm/glm.hpp>

namespace PixieRenderer {

struct Mesh {
	static constexpr int32_t cBonesPerVertex = 4;

	std::vector<glm::vec3> m_positions;
	std::vector<glm::vec3> m_normals;
	std::vector<glm::vec2> m_texCoords;
	std::vector<int32_t> m_boneIDs;
	std::vector<float> m_boneWeights;
	std::vector<int32_t> m_indices;

	Mesh() = default;
};

} // namespace PixieRenderer
