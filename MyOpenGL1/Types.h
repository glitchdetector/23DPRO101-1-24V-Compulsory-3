#pragma once

// Single vertex data
struct Vertex
{
    float x, y, z, r, g, b, u, v;
};

// In-world transformations
struct Transformation
{
    float x = 0.0, y = 0.0, z = 0.0;
    float pitch = 0.0, yaw = 0.0, roll = 0.0;
    float scale_x = 1.0, scale_y = 1.0, scale_z = 1.0;
    Transformation() = default;
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

    BoxCollisionDef collision;
    bool bHasBoxCollision = false;

    float RadiusCollisionSize = 0.0f;
    bool bHasRadiusCollision = false;

    unsigned int EBO, VBO, VAO;

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