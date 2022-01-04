#version 330 core

layout(location = 0) in vec3 vertPosition;
layout(location = 1) in vec2 vertTexcoords;
layout(location = 2) in vec3 vertNormal;
layout(location = 3) in vec3 vertTangent;
layout(location = 4) in mat4 instMatrix;

uniform mat4 uniModelMatrix;
uniform mat4 uniVPMatrix;

out struct MeshInfo {
  vec3 vertPosition;
  vec2 vertTexcoords;
  mat3 vertTBNMatrix;
} fragMeshInfo;

void main() {
  mat4 modelMat = instMatrix * uniModelMatrix;

  fragMeshInfo.vertPosition  = (modelMat * vec4(vertPosition, 1.0)).xyz;
  fragMeshInfo.vertTexcoords = vertTexcoords;

  mat3 modelMat3 = mat3(modelMat);

  vec3 tangent   = normalize(modelMat3 * vertTangent);
  vec3 normal    = normalize(modelMat3 * vertNormal);
  vec3 bitangent = cross(normal, tangent);
  fragMeshInfo.vertTBNMatrix = mat3(tangent, bitangent, normal);

  gl_Position = uniVPMatrix * modelMat * vec4(vertPosition, 1.0);
}
