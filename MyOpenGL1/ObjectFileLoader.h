#pragma once
#include <sstream>
#include <string>

struct ObjectFileReturnInfo
{
    bool bHasTextureData = false;
    bool bHasNormalData = false;
    bool bSuccess = false;
    int nVertices = 0;
    int nFaces = 0;
    int nObjects = 0;
    void print()
    {
        std::cout << "Object Properties" << std::endl;
        std::cout << "Loaded: " << bSuccess << std::endl;
        std::cout << "Has Texture Data: " << bHasTextureData << std::endl;
        std::cout << "Has Normal Data: " << bHasNormalData << std::endl;
        std::cout << "Vertices: " << nVertices << std::endl;
        std::cout << "Faces: " << nFaces << std::endl;
        std::cout << "Meshes: " << nObjects << std::endl;
    }
};

inline ObjectFileReturnInfo ReadObjectFile(std::string fileName, std::vector<Vertex>& vertices, std::vector<int>& indices)
{
    std::ifstream in;
    in.open("models/" + fileName);

    ObjectFileReturnInfo output;

    //vertices.push_back(Vertex{ 0,0,0,0,0,0,0,0 });

    if (!in.is_open())
    {
        std::cout << "Could not open file " << fileName << std::endl;
        return output;
    }
    std::cout << "Reading file " << fileName << std::endl;

    struct TempVertex
    {
        float x, y, z;
        float r, g, b;
    };
    std::vector<TempVertex> vertex_vector;

    struct TempUV
    {
        float u, v;
    };
    std::vector<TempUV> uv_vector;

    struct TempNormal
    {
        float x, y, z;
    };
    std::vector<TempNormal> normal_vector;

    bool bHasUVData = false;

    std::string line;

    std::string prefix;
    in >> prefix;
    while (!in.eof())
    {
        if (prefix == "#") {
            std::getline(in, line);
            std::cout << line << std::endl;
        }
        else if (prefix == "vn") {
            // Normal data
            TempNormal normal;
            in
                >> normal.x
                >> normal.y
                >> normal.z;
            normal_vector.push_back(normal);
            output.bHasNormalData = true;
        }
        else if (prefix == "vt") {
            // Vertex Texture (UV)
            TempUV uv;
            in
                >> uv.u
                >> uv.v;
            uv_vector.push_back(uv);
            output.bHasTextureData = true;
        }
        else if (prefix == "v") {
            // Vertex
            TempVertex v;
            in
                >> v.x
                >> v.y
                >> v.z
                >> v.r
                >> v.g
                >> v.b;
            vertex_vector.push_back(v);
        }
        else if (prefix == "f") {
            // Go over each point in the face
            for (int i = 0; i < 3; i++)
            {
                std::string vertex;
                in >> vertex;

                std::string string;
                std::istringstream iss(vertex);

                int vertexIndex;
                iss >> vertexIndex;

                TempVertex t = vertex_vector[vertexIndex - 1];
                Vertex v{ t.x, t.y, t.z, t.r, t.g, t.b, 0.f, 0.f };

                if (output.bHasTextureData)
                {
                    std::getline(iss, string, '/');
                    int uvIndex;
                    iss >> uvIndex;

                    TempUV uv = uv_vector[uvIndex - 1];
                    v.u = uv.u;
                    v.v = 1.f - uv.v;
                }
                if (output.bHasNormalData)
                {
                    // Unused by our program
                }
                vertices.push_back(v);
                indices.push_back(vertices.size() - 1);
            }
            output.nFaces++;
        }
        else if (prefix == "o") {
            std::getline(in, line);
            std::cout << "Object " << line << std::endl;
            output.nObjects++;
        }
        else if (prefix == "s") {
            int shadeSmooth;
            in >> shadeSmooth;
            if (shadeSmooth == 1) {
                std::cout << "Set smooth shading" << std::endl;
            }
            else {
                std::cout << "Set flat shading" << std::endl;
            }
        }
        else if (prefix == "usemtl") {
            std::getline(in, line);
            std::cout << "Using material " << line << std::endl;
        }
        else {
            std::getline(in, line);
            std::cout << "Unknown line " << line << std::endl;
        }
        in >> prefix;
    }
    output.nVertices = vertex_vector.size();
    output.bSuccess = true;
    std::cout << "Loaded " << fileName << " with " << vertices.size() << " verts and " << (indices.size() / 3) << " tris." << std::endl;
    return output;
}