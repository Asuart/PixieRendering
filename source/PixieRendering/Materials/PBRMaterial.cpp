#include "PBRMaterial.h"

static const char* vertexShaderSource = R"(
#version 450

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in ivec4 boneIDs; 
layout(location = 4) in vec4 boneWeights; 

layout(location = 0) out vec2 TexCoord;

layout(set = 0, binding = 0, std140) uniform CameraUBO {
    mat4 view;
    mat4 projection;
} camera;

layout(set = 0, binding = 1, std140) uniform ModelUBO {
    mat4 model;
} modelData;

void main()
{
    gl_Position = camera.projection * camera.view * modelData.model * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
}
)";

static const char* fragmentShaderSource = R"(
#version 450

layout(location = 0) out vec4 FragColor;

layout(location = 0) in vec2 TexCoords;
layout(location = 1) in vec3 WorldPos;
layout(location = 2) in vec3 Normal;

layout(set = 0, binding = 2, std140) uniform CameraPosition {
    vec3 po;
} cameraPosition;

layout(set = 0, binding = 3, std140) uniform MaterialUBO {
    vec3 albedo;
    float metallic;
    float roughness;
    float ao;
} materialData;

layout(set = 0, binding = 4) uniform sampler2D texSampler;

float DistributionGGX(vec3 N, vec3 H, float a)
{
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float nom    = a2;
    float denom  = (NdotH2 * (a2 - 1.0) + 1.0);
    denom        = PI * denom * denom;
	
    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float k)
{
    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return nom / denom;
}
  
float GeometrySmith(vec3 N, vec3 V, vec3 L, float k)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, k);
    float ggx2 = GeometrySchlickGGX(NdotL, k);
	
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

void main()
{
    vec3 N = normalize(Normal); 
    vec3 V = normalize(camPos - WorldPos);

    FragColor = texture(texSampler, TexCoord);
}
)";

