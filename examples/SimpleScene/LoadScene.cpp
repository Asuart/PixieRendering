#include "LoadScene.h"

#include <iostream>

#define TINYOBJLOADER_DISABLE_FAST_FLOAT
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

using namespace PixieRenderer;

PixieRenderer::Mesh* LoadMesh(const std::string& path) {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string warn;
	std::string err;

	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str(), nullptr);

	if (!warn.empty()) {
		std::cout << "Warning: " << warn << std::endl;
	}

	if (!err.empty()) {
		std::cerr << "Error: " << err << std::endl;
	}

	if (!ret) {
		std::cerr << "Failed to load/parse .obj file" << std::endl;
		exit(1);
	}

	std::cout << "Loaded " << shapes.size() << " shapes." << std::endl;

	Mesh* mesh = new Mesh();

	for (size_t s = 0; s < shapes.size(); s++) {
		size_t index_offset = 0;
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
			size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
			for (size_t v = 0; v < fv; v++) {
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

				Vertex vertex{};

				vertex.position.x = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
				vertex.position.y = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
				vertex.position.z = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

				if (idx.normal_index >= 0) {
					vertex.normal.x = attrib.normals[3 * size_t(idx.normal_index) + 0];
					vertex.normal.y = attrib.normals[3 * size_t(idx.normal_index) + 1];
					vertex.normal.z = attrib.normals[3 * size_t(idx.normal_index) + 2];
				}

				if (idx.texcoord_index >= 0) {
					vertex.uv.x = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
					vertex.uv.y = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
				}

				mesh->vertexes.push_back(vertex);
				mesh->indexes.push_back(mesh->indexes.size());
			}
			index_offset += fv;
		}
	}

	return mesh;
}
