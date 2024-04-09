#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#ifdef bitch

class Waveform
{
	struct Vertex
	{
		float x, y, z, w;
		float r, g, b;
	};

	struct VertexTexture
	{
		float u, v, w;
	};

	struct VertexNormals
	{
		float x, y, z;
	};

	struct VertexParameter
	{
		float u, v, w;
	};

	struct Face
	{
		std::vector<Vertex> m_vertices;
		std::vector<VertexTexture> m_vertexTextures;
	};

	struct Object
	{
		std::string name;
		std::vector<Vertex> m_vertices;
		std::vector<VertexTexture> m_vertexTextures;
		std::vector<Face> m_faces;
		bool smoothShading;
	};

	std::vector<Object> LoadFile(std::string fileName)
	{
		std::vector<Object> output;

        std::ifstream in;
        in.open(fileName);

        //vertices.push_back(Vertex{ 0,0,0,0,0,0,0,0 });

        if (!in.is_open())
        {
            std::cout << "Could not open file " << fileName << std::endl;
            return output;
        }
        std::cout << "Reading file " << fileName << std::endl;

        std::string line;
        Object object;
        object.name = "Object";

        bool bFirstObject;

        char prefix;
        in >> prefix;
        while (!in.eof())
        {
            switch (prefix)
            {
            case '#':
                std::getline(in, line);
                std::cout << line << std::endl;
                break;
            case 'v':
                // Vertex
                Vertex v;
                in
                    >> v.x
                    >> v.y
                    >> v.z
                    >> v.r
                    >> v.g
                    >> v.b;
                vertices.push_back(v);
                break;
            case 'f':
                int v1, v2, v3;
                in
                    >> v1
                    >> v2
                    >> v3;
                indices.push_back(v1 - 1);
                indices.push_back(v2 - 1);
                indices.push_back(v3 - 1);
                break;
            case 'o':
                std::getline(in, line);
                std::cout << "Object " << line << std::endl;
                object.name = line;
                break;
            case 's':
                int shadeSmooth;
                in >> shadeSmooth;
                if (shadeSmooth == 1) {
                    std::cout << "Set smooth shading" << std::endl;
                }
                else {
                    std::cout << "Set flat shading" << std::endl;
                }
                break;
            default:
                std::getline(in, line);
                std::cout << "Unknown line " << line << std::endl;
                break;
            }
            in >> prefix;
        }
		return output;
	}
};
#endif