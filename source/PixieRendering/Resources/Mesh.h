#pragma once
#include <vector>
#include <glm/glm.hpp>

struct Mesh {
	std::vector<glm::vec3> m_positions;
	std::vector<glm::vec3> m_normals;
	std::vector<glm::vec2> m_texCoords;
	std::vector<int32_t> m_indices;
	std::vector<int32_t> m_boneIDs;
	std::vector<float> m_boneWeights;
	int32_t m_bonesPerVertice = 0;

	Mesh() = default;
};
