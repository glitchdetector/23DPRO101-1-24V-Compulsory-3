#pragma once
#include <iostream>
#include <string>

#include "glm/geometric.hpp"
#include "glm/vec3.hpp"
#include "glm/vec2.hpp"

struct Texture;
struct Shader;

// Single vertex data
struct Vertex
{
    float x, y, z, r, g, b, u, v, nx, ny, nz;
};

// In-world transformations
struct Transformation
{
    float x = 0.0, y = 0.0, z = 0.0;
    float pitch = 0.0, yaw = 0.0, roll = 0.0;
    float scale_x = 1.0, scale_y = 1.0, scale_z = 1.0;
    Transformation() = default;
    glm::vec3 Position() { return glm::vec3(x, y, z); }
};

struct Material
{
    std::vector<Texture*> textures;
    void AddTexture(Texture* texture)
    {
        textures.push_back(texture);
    }
    Shader* shader;
};

struct BoxCollisionDef
{
    float x_relative, y_relative, z_relative;
    float x_size, y_size, z_size;
    BoxCollisionDef() = default;
};

struct WorldCollision
{
    float x1, x2, y1, y2, z1, z2;
};

// An instance of an object
struct Entity
{
    std::vector<Vertex> vertices;
    std::vector<int> indices;
    Transformation transformation;
    Transformation previousTransformation;

    Material* material;

    bool bIsEnabled = true;

    BoxCollisionDef collision;
    bool bHasBoxCollision = false;

    // Does this entity snap to terrain height?
    bool bIsAffectedByTerrain = true;

    bool bChecksCollision = false;

    // Size of the radius collider
    float RadiusCollisionSize = 0.0f;
    // Does this entity block the player using the radius collider?
    bool bHasRadiusCollision = false;
    // Does this entity trigger the OnTrigger function when the player collides with the radius collider?
    bool bHasRadiusTrigger = false;
    virtual void OnTrigger(Entity* other) {};

    virtual void OnTick(float deltaTime) {};

    unsigned int EBO, VBO, VAO;

    // Calculate vertex normals for all triangles in this entity's mesh
    // NOTE: Assumes separate triangles, 3 indices per triangle
    void GenerateNormals()
    {
	    for (int i = 0; i < indices.size(); i+=3)
	    {
            Vertex* P = &vertices[indices[i]];
            Vertex* Q = &vertices[indices[i + 1]];
            Vertex* R = &vertices[indices[i + 2]];
            glm::vec3 A = glm::vec3(P->x, P->y, P->z);
            glm::vec3 B = glm::vec3(Q->x, Q->y, Q->z);
            glm::vec3 C = glm::vec3(R->x, R->y, R->z);
            glm::vec3 normal = glm::normalize(glm::cross(B - A, C - A));
            P->nx = normal.x; P->ny = normal.y; P->nz = normal.z;
            Q->nx = normal.x; Q->ny = normal.y; Q->nz = normal.z;
            R->nx = normal.x; R->ny = normal.y; R->nz = normal.z;
	    }
    }

    // Get the absolute collision values for this entity
    WorldCollision GetWorldCollision() const
    {
        WorldCollision worldCollision;
        worldCollision.x1 = transformation.x + collision.x_relative * transformation.scale_x;
        worldCollision.x2 = worldCollision.x1 + collision.x_size * transformation.scale_x;
        worldCollision.y1 = transformation.y + collision.y_relative * transformation.scale_y;
        worldCollision.y2 = worldCollision.y1 + collision.y_size * transformation.scale_y;
        worldCollision.z1 = transformation.z + collision.z_relative * transformation.scale_z;
        worldCollision.z2 = worldCollision.z1 + collision.z_size * transformation.scale_z;
        return worldCollision;
    }

    Entity() = default;
};

class APlayer : public Entity
{
public:
    void OnTrigger(Entity* other) override
    {
    }

};

class ABox : public Entity
{
public:

	void OnTrigger(Entity* other) override
	{
        std::cout << "Box trigger" << std::endl;
        glm::vec2 currentPosition = glm::vec2(this->transformation.x, this->transformation.z);
        glm::vec2 difference = currentPosition - glm::vec2(other->transformation.x, other->transformation.z);
        float distance = glm::length(difference);
        float contactDistance = (other->RadiusCollisionSize + this->RadiusCollisionSize);
        glm::vec2 newPosition = glm::normalize(difference) * (distance + 0.1f);
        this->transformation.x = other->transformation.x + newPosition.x;
        this->transformation.z = other->transformation.z + newPosition.y;
	}

    ABox()
	{
        bHasRadiusCollision = true;
        bHasRadiusTrigger = true;
        RadiusCollisionSize = 0.35f;
	}
};