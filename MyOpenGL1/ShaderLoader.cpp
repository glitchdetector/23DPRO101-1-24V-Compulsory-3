#include "ShaderLoader.h"

std::string ShaderLoader::LoadShaderFromFile(const std::string& filePath) {
	std::ifstream shaderFile(filePath);
	if (!shaderFile.is_open()) {
		throw std::exception("Failed to open shader file!");
	}
	std::stringstream shaderStream;
	shaderStream << shaderFile.rdbuf();
	shaderFile.close();

	std::cout << "Loaded shader file " << filePath << std::endl;

	return shaderStream.str();
}