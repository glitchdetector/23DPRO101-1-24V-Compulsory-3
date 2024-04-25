// source 
//https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/1.getting_started/2.1.hello_triangle/hello_triangle.cpp

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <map>
#include <stb_image.h>

#include "Types.h" // Generic types used in the project
#include "ObjectFileLoader.h" // Can load and prepare .obj files to be rendered
#include "ShaderLoader.h" // Can load and prepare shader files  
#include "Level.h" // Handles loading level meshes
#include "Surface.h" // Surface function and generation
#include "Camera.h" // Handles camera controls and updates
#include "Curve.h"
#include "Helper.h"
#include "glm/gtx/quaternion.hpp"

//#define _SHOW_VISUAL_CURVES

// If anything happens to your frame, this is called
// resizing etc
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
bool processInput(GLFWwindow* window, float deltaTime);

// settings
unsigned int SCR_WIDTH = 1080;
unsigned int SCR_HEIGHT = 720;

// camera
Camera camera(glm::vec3(0.0f, 2.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;	// time between current frame and last frame

struct Texture
{
    unsigned int index;
    std::string name;

    static Texture* LoadFromFile(std::string fileName)
    {
        std::cout << "Loading texture " << fileName << std::endl;
        Texture* tex = new Texture();
        tex->name = fileName;

        glGenTextures(1, &tex->index);
        glBindTexture(GL_TEXTURE_2D, tex->index); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        std::string path = "textures/" + fileName;

        int width, height, nChannels;
        unsigned char* data = stbi_load(path.c_str(), &width, &height, &nChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
            std::cout << "Loaded texture " << fileName << std::endl;
        }
        else
        {
            std::cout << "Failed to load texture " << fileName << std::endl;
        }
        stbi_image_free(data);

        return tex;
    }
};

struct Shader
{
    std::string name;
    unsigned int shaderProgram;

    std::map<std::string, unsigned int> locations;

    Shader(std::string name)
    {
        this->name = name;
        std::string shaderPath = "shaders/" + name + "/";
        std::string vertexShaderSourceStr = ShaderLoader::LoadShaderFromFile(shaderPath + "svert.glsl");
        const char* vertexShaderSource = vertexShaderSourceStr.c_str();
        std::string fragmentShaderSourceStr = ShaderLoader::LoadShaderFromFile(shaderPath + "sfrag.glsl");
        const char* fragmentShaderSource = fragmentShaderSourceStr.c_str();

        // build and compile our shader program
        // ------------------------------------
        // vertex shader
        unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);
        // check for shader compile errors
        int success;
        char infoLog[512];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
        // fragment shader
        unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);
        // check for shader compile errors
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
        // link shaders
        unsigned int shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);
        // check for linking errors
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        this->shaderProgram = shaderProgram;

        this->locations["timePassedLocation"] = glGetUniformLocation(shaderProgram, "timePassed");
        this->locations["bUseTextureLoc"] = glGetUniformLocation(shaderProgram, "bUseTexture");
        this->locations["viewLoc"] = glGetUniformLocation(shaderProgram, "view");
        this->locations["viewPosLoc"] = glGetUniformLocation(shaderProgram, "viewPos");
        this->locations["projectionLoc"] = glGetUniformLocation(shaderProgram, "projection");
        this->locations["entityMatrixLoc"] = glGetUniformLocation(shaderProgram, "entityMatrix");
        this->locations["playerPosLoc"] = glGetUniformLocation(shaderProgram, "playerPos");
        this->locations["evilmanPosLoc"] = glGetUniformLocation(shaderProgram, "evilmanPos");

        this->locations["texture0Loc"] = glGetUniformLocation(shaderProgram, "texture_0");
        this->locations["texture1Loc"] = glGetUniformLocation(shaderProgram, "texture_1");
        this->locations["texture2Loc"] = glGetUniformLocation(shaderProgram, "texture_2");

        for (std::pair<std::string, unsigned int> location : this->locations)
        {
            std::cout << location.first << " = " << location.second << std::endl;
        }
    }
};

bool wasAnyInputPressed = false;
bool IsKeyHeld(GLFWwindow* window, int key)
{
    bool result = (glfwGetKey(window, key) == GLFW_PRESS);
    if (result) wasAnyInputPressed = true;
    return result;
}

struct MovementInput {
    float x, y, jump;
};

MovementInput processMovementInput(GLFWwindow* window)
{
    MovementInput movement = { 0, 0, 0 };

    wasAnyInputPressed = false;

    // Player Movement WSAD
    if (IsKeyHeld(window, GLFW_KEY_W))
        movement.y += 1;
    if (IsKeyHeld(window, GLFW_KEY_S))
        movement.y -= 1;
    if (IsKeyHeld(window, GLFW_KEY_A))
        movement.x -= 1;
    if (IsKeyHeld(window, GLFW_KEY_D))
        movement.x += 1;
    if (IsKeyHeld(window, GLFW_KEY_SPACE))
        movement.jump += 1;

    return movement;
}

float naive_lerp(float a, float b, float t)
{
    return a + t * (b - a);
}

float naive_lerp_loop(float a, float b, float t, float limit)
{
    if (a > limit) a -= limit;
    if (a < 0.0) a += limit;
    if (abs(a - b) > limit / 2.0f) b += limit;
    return naive_lerp(a, b, t);
}

inline float randomRange(float min, float max)
{
    float range = max - min;
    float modifier = RAND_MAX / range;
    float result = rand() / modifier;
    return min + result;
}


Level* currentLevel;
Level* nextLevel;
APlayer* player = new APlayer();

class ASceneTeleporter : public Entity
{
public:
    Level* scene;
    Transformation offset;
    void OnTrigger(Entity* other, Level& level) override
    {
        if (dynamic_cast<APlayer*>(other) != nullptr)
        {
            std::cout << "Teleported to new scene!" << std::endl;
            nextLevel = scene;
            player->transformation.x = 0.0f;
            player->transformation.y = 0.0f;
            player->transformation.z = 0.0f;
        }
    }
};

bool isWireframeModeEnabled = false;

int CurrentRenderMode = GL_TRIANGLES;

int main(int argc, char** argv)
{
    player->RadiusCollisionSize = 0.5f;
    player->bChecksCollision = true;
    // glfw: initialize and configure
    // ------------------------------

#pragma region Program Input
    std::string levelFile = "level.data";
    std::string textureFileName = "texture.png";
    for (int i = 0; i < argc; i++)
    {
        std::cout << i << ": " << argv[i] << std::endl;
    }

    // Select object and texture files, drag the object file onto the executable
    if (argc > 1) textureFileName = argv[1];
    if (argc > 2) levelFile = argv[2];
#pragma endregion
#pragma region Level Loading

    Level* levelOutside = new Level("outside.data");
    {
        Level* level = levelOutside;

        level->bounds.x_relative = -50.f;
        level->bounds.y_relative = -50.f;
        level->bounds.x_size = 100.f;
        level->bounds.y_size = 100.f;

        level->planeFunction = new TerrainPlane();
    }

    Level* levelInside = new Level("inside.data");
    {
        Level* level = levelInside;

        level->bounds.x_relative = -5.f;
        level->bounds.y_relative = -5.f;
        level->bounds.x_size = 10.f;
        level->bounds.y_size = 10.f;
        level->bIsCameraControllable = false;
        level->planeFunction = new FlatPlane();
        level->cameraPosition.x = 8.5f;
        level->cameraPosition.y = 2.5f;
        level->cameraPosition.z = -8.5f;
        level->cameraPosition.pitch = -10.f;
        level->cameraPosition.yaw = 90.f + 45.f;
    }

    currentLevel = levelInside;
    nextLevel = currentLevel;
    {
        Level* level = currentLevel;
        camera.Position = glm::vec3(
            level->cameraPosition.x,
            level->cameraPosition.y,
            level->cameraPosition.z
        );
    }

#pragma endregion
#pragma region GLFW Initialization

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // to make this portable for other Devices/ operating system MacOS
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Triangle window", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

#pragma endregion

#pragma region Dear ImGui Setup

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

	ImGui_ImplGlfw_InitForOpenGL(window, false);
	ImGui_ImplOpenGL3_Init("#version 130");

#pragma endregion
#pragma region Shader Setup

    std::vector<Shader*> shaders;
    Shader* defaultShader = new Shader("default");
    shaders.push_back(defaultShader);
    Shader* skyboxShader = new Shader("skybox");
    shaders.push_back(skyboxShader);
    Shader* wireframeShader = new Shader("wireframe");
    shaders.push_back(wireframeShader);

#pragma endregion
#pragma region Material Setup
    Material* defaultMaterial = new Material();
    defaultMaterial->shader = defaultShader;

    Material* skyboxMaterial = new Material();
    skyboxMaterial->shader = skyboxShader;
#pragma endregion
#pragma region Buffer Mesh Loading

    CurrentRenderMode = GL_TRIANGLES;

    Entity* Skybox = new Entity();
    {
        ReadObjectFile("skybox.obj", Skybox->vertices, Skybox->indices);
        Skybox->bIsAffectedByTerrain = false;
        Skybox->transformation.scale_x = 5.f;
        Skybox->transformation.scale_y = 5.f;
        Skybox->transformation.scale_z = 5.f;
        levelOutside->entities.push_back(Skybox);
    }

    Entity* Box = new ABox();
    {
        ReadObjectFile("box.obj", Box->vertices, Box->indices);
        Box->bIsAffectedByTerrain = true;
        levelOutside->entities.push_back(Box);
        Box->transformation.x = -15.f;
        Box->transformation.z = -15.f;
        Box->bChecksCollision = true;
    }



    struct Bird
    {
        Curve* path;
        Entity* entity;
        float progress = 0.0f;
        bool reverse = false;
        float speed = 0.1f;
        Entity* visualCurve;
    };

    // Make birds
    std::vector<Bird*> birds;
    {
        int numBirds = 10;
        for (int i = 0; i < numBirds; i++)
        {
            Bird* bird = new Bird();
            bird->entity = new Entity();
            {
                ReadObjectFile("bird.obj", bird->entity->vertices, bird->entity->indices);
                levelOutside->entities.push_back(bird->entity);
            }
            bird->progress = (1.0 / numBirds) * i;
            bird->path = new Curve();
            {
                float min_x = -30.0f;
                float min_y = -30.0f;
                float max_x = 30.0f;
                float max_y = 30.0f;
                int numPoints = 4;
                for (int i = 0; i < numPoints; i++)
                {
                    float point_x = randomRange(min_x, max_x);
                    float point_y = randomRange(min_y, max_y);
                    bird->path->points.push_back(glm::vec2(point_x, point_y));
                }
            }
            //bird->entity->transformation.yaw = randomRange(0.0, 360.0);

#ifdef _SHOW_VISUAL_CURVES
            bird->visualCurve = new Entity();
            Surface::GenerateFromCurve(bird->path, 50, bird->visualCurve->vertices, bird->visualCurve->indices);
            for (Vertex v : bird->visualCurve->vertices)
            {
                v.r = 1.0f;
            }
            bird->visualCurve->bIsAffectedByTerrain = false;
            level.entities.push_back(bird->visualCurve);
#endif

            levelOutside->entities.push_back(bird->entity);
            birds.push_back(bird);
        }
    }

    Entity* lighthouse = new Entity();
    {
        ReadObjectFile("lighthouse.obj", lighthouse->vertices, lighthouse->indices);
        levelOutside->entities.push_back(lighthouse);
        lighthouse->RadiusCollisionSize = 2.0f;
        lighthouse->bHasRadiusCollision = true;
        lighthouse->transformation.x = -6.f;
    }

    ASceneTeleporter* lighthouseDoor = new ASceneTeleporter();
    {
        ReadObjectFile("lighthousedoor.obj", lighthouseDoor->vertices, lighthouseDoor->indices);
        levelOutside->entities.push_back(lighthouseDoor);
        lighthouseDoor->RadiusCollisionSize = 1.0f;
        lighthouseDoor->bHasRadiusTrigger = true;
        lighthouseDoor->transformation.x = -4.f;
        lighthouseDoor->scene = levelInside;
    }

    ASceneTeleporter* lighthouseDoorInside = new ASceneTeleporter();
    {
        ReadObjectFile("lighthousedoor.obj", lighthouseDoorInside->vertices, lighthouseDoorInside->indices);
        levelInside->entities.push_back(lighthouseDoorInside);
        lighthouseDoorInside->RadiusCollisionSize = 1.0f;
        lighthouseDoorInside->bHasRadiusTrigger = true;
        lighthouseDoorInside->transformation.x = -5.f;
        lighthouseDoorInside->scene = levelOutside;
    }

    Entity* lighthouseBeam = new Entity();
    {
        ReadObjectFile("lighthousebeam.obj", lighthouseBeam->vertices, lighthouseBeam->indices);
        levelOutside->entities.push_back(lighthouseBeam);
        lighthouseBeam->bHasAlpha = true;
        lighthouseBeam->transformation.x = -6.f;
    }

    std::vector<Entity*> pickups;
    {
	    for (int i = 0; i < 10; i++)
	    {
            APickup* coin = new APickup();
            {
                ReadObjectFile("coin.obj", coin->vertices, coin->indices);
                coin->transformation.x = randomRange(-50.f, 50.f);
                coin->transformation.z = randomRange(-50.f, 50.f);
            }
            levelOutside->entities.push_back(coin);
            pickups.push_back(coin);
	    }
    }

    Entity* evilman = new Entity();
    {
        Bird* bird = new Bird();
        bird->entity = evilman;
        {
            ReadObjectFile("evilman.obj", bird->entity->vertices, bird->entity->indices);
            levelOutside->entities.push_back(bird->entity);
            bird->progress = 0.0f;
            bird->speed = 0.02f;

            bird->path = new Curve();
            {
                float min_x = -20.0f;
                float min_y = -20.0f;
                float max_x = 20.0f;
                float max_y = 20.0f;
                int numPoints = 4;
                for (int i = 0; i < numPoints; i++)
                {
                    float point_x = randomRange(min_x, max_x);
                    float point_y = randomRange(min_y, max_y);
                    bird->path->points.push_back(glm::vec2(point_x, point_y));
                }
            }
        }
        levelOutside->entities.push_back(bird->entity);

#ifdef _SHOW_VISUAL_CURVES
        bird->visualCurve = new Entity();
        Surface::GenerateFromCurve(bird->path, 50, bird->visualCurve->vertices, bird->visualCurve->indices);
        bird->visualCurve->bIsAffectedByTerrain = false;
        level.entities.push_back(bird->visualCurve);
#endif

        birds.push_back(bird);
    }

    // Interior creation
    {
        Entity* indoorMesh = new Entity();
        ReadObjectFile("indoormesh.obj", indoorMesh->vertices, indoorMesh->indices);
        levelInside->entities.push_back(indoorMesh);
    }
    

#pragma region +Surface Creation

    Entity* surface = new Entity();
    {
        float min_x = -50.0f;
        float min_y = -50.0f;
        float max_x = 50.0f;
    	float max_y = 50.0f;
        int subdivision = 40;
        int numTrees = 50;

        Surface::GenerateSurface(levelOutside, min_x, max_x, min_y, max_y, subdivision, surface->vertices, surface->indices);
        levelOutside->entities.push_back(surface);

        surface->bIsAffectedByTerrain = false;

        std::vector<Vertex> treeVertices;
        std::vector<int> treeIndices;
        ReadObjectFile("tree.obj", treeVertices, treeIndices);

        for (int i = 0; i < numTrees; i++)
        {
            float tree_x = randomRange(min_x, max_x);
            float tree_y = randomRange(min_y, max_y);
            Entity* tree = new Entity();
            tree->vertices = treeVertices;
            tree->indices = treeIndices;
            tree->transformation.x = tree_x;
            tree->transformation.z = tree_y;
            tree->RadiusCollisionSize = 0.5f;
            tree->bHasRadiusCollision = true;
            levelOutside->entities.push_back(tree);
        }
    }

#pragma endregion

    ReadObjectFile("player.obj", player->vertices, player->indices);
    player->transformation.x = currentLevel->playerStart.x;
    levelInside->entities.push_back(player);
    levelOutside->entities.push_back(player);

    std::vector<Entity*> allEntites;
    for (Entity* ent : levelOutside->entities)
        allEntites.push_back(ent);
    for (Entity* ent : levelInside->entities)
        allEntites.push_back(ent);
    std::cout << "Entities: " << allEntites.size() << std::endl;

    for (int i = 0; i < allEntites.size(); i++)
    {
        unsigned int VBO, VAO, EBO;
        Entity* entity = allEntites[i];
        entity->GenerateNormals();

        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, entity->vertices.size() * sizeof(Vertex), entity->vertices.data(), GL_STATIC_DRAW);

        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, entity->indices.size() * sizeof(int), entity->indices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(8 * sizeof(float)));
        glEnableVertexAttribArray(3);

        glBindVertexArray(0);

        std::cout << i << ": " << VAO << std::endl;
        entity->VAO = VAO;
        entity->VBO = VBO;
        entity->EBO = EBO;
	}

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthRange(0.0, 100.0);

#pragma endregion

#pragma region Texture Loading

    defaultMaterial->AddTexture(Texture::LoadFromFile("texture.png"));

    skyboxMaterial->AddTexture(Texture::LoadFromFile("texture_skybox.png"));
    skyboxMaterial->AddTexture(Texture::LoadFromFile("noise_skybox.png"));

    Skybox->material = skyboxMaterial;
    Box->material = defaultMaterial;
    player->material = defaultMaterial;

    for (Entity* pickup : pickups)
    {
        pickup->material = defaultMaterial;
    }

#pragma endregion

    glLineWidth(0.1);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0);

    // uncomment this call to draw in wireframe polygons.
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
    double previousFrameTime = glfwGetTime();
    while (!glfwWindowShouldClose(window))
    {
		double currentFrameTime = glfwGetTime();
        deltaTime = currentFrameTime - previousFrameTime;
		previousFrameTime = currentFrameTime;
        currentLevel = nextLevel;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
		
        // input
        // -----
        bool bIsCameraControlsInUse = processInput(window, deltaTime);

        // Move camera to player
        if ((!bIsCameraControlsInUse) && currentLevel->bIsCameraControllable) {
            glm::vec3 newCameraOffset = camera.Front * -5.0f;
            glm::vec3 playerPos = player->transformation.Position();
            newCameraOffset.y += 1.0f;

            camera.Position = playerPos + newCameraOffset;
            camera.Position.y += currentLevel->GetGroundZAt2dCoord(playerPos.x, playerPos.z);
            camera.Position.y = std::max(currentLevel->GetGroundZAt2dCoord(camera.Position.x, camera.Position.z) + 0.1f, camera.Position.y);
        } else if (!currentLevel->bIsCameraControllable)
        {
            camera.Position = currentLevel->cameraPosition.Position();
            camera.Pitch = currentLevel->cameraPosition.pitch;
            camera.Yaw = currentLevel->cameraPosition.yaw;
            camera.updateCameraVectors();
            //camera.Roll = currentLevel->cameraPosition.roll;
        }

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glClear(GL_DEPTH_BUFFER_BIT);
       
        //camera.updateCameraVectors();

        // Do player collision checks
		{
            // Top down 2D collisions (radius)
            glm::vec2 currentPosition = glm::vec2(player->transformation.x, player->transformation.z);
            for (int i = 0; i < currentLevel->entities.size(); i++)
            {
                Entity* entity = currentLevel->entities[i];
                if (!entity->bHasRadiusCollision && !entity->bHasRadiusTrigger) continue;

                glm::vec2 difference = currentPosition - glm::vec2(entity->transformation.x, entity->transformation.z);
                float distance = glm::length(difference);
                float contactDistance = (player->RadiusCollisionSize + entity->RadiusCollisionSize);
                if (distance < contactDistance)
                {
                    if (entity->bHasRadiusCollision)
                    {
	                    // We are colliding with something!
	                    // Move the player away from the object
	                    glm::vec2 newPosition = glm::vec2(entity->transformation.x, entity->transformation.z) + glm::normalize(difference) * contactDistance;
	                    player->transformation.x = newPosition.x;
	                    player->transformation.z = newPosition.y;
	                    currentPosition = glm::vec2(player->transformation.x, player->transformation.z);
                    }
                    if (entity->bHasRadiusTrigger)
                    {
                        entity->OnTrigger(player);
                        entity->OnTrigger(player, *currentLevel);
                    }
                }
            }
            if (currentPosition.x < currentLevel->bounds.x_relative)
                player->transformation.x = currentLevel->bounds.x_relative;
            if (currentPosition.x > currentLevel->bounds.x_size / 2)
                player->transformation.x = currentLevel->bounds.x_size / 2;
            if (currentPosition.y < currentLevel->bounds.y_relative)
                player->transformation.z = currentLevel->bounds.y_relative;
            if (currentPosition.y > currentLevel->bounds.y_size / 2)
                player->transformation.z = currentLevel->bounds.y_size / 2;
		}

        // Do entity collision triggers
		{
            for (Entity* entity : currentLevel->entities)
            {
                if (!entity->bChecksCollision) continue;
                if (!entity->bHasRadiusCollision && !entity->bHasRadiusTrigger) continue;
                if (!entity->bIsEnabled) continue;

                glm::vec2 currentPosition = glm::vec2(entity->transformation.x, entity->transformation.z);
                for (Entity* other : currentLevel->entities)
                {
                    if (other == entity) continue;
                    if (!entity->bHasRadiusCollision && !entity->bHasRadiusTrigger) continue;
                    if (!other->bIsEnabled) continue;

                    glm::vec2 difference = currentPosition - glm::vec2(other->transformation.x, other->transformation.z);
                    float distance = glm::length(difference);
                    float contactDistance = (entity->RadiusCollisionSize + other->RadiusCollisionSize);
                    if (distance < contactDistance)
                    {
                        if (other->bHasRadiusTrigger)
                        {
                            other->OnTrigger(entity);
                            other->OnTrigger(entity, *currentLevel);
                        }
                        if (entity->bHasRadiusTrigger)
                        {
                            entity->OnTrigger(other);
                            entity->OnTrigger(other, *currentLevel);
                        }
                    }
                } 
            }
		}

        // Move birds
		{
            for (int i = 0; i < birds.size(); i++)
            {
                Bird* bird = birds[i];

                if (bird->reverse)
					bird->progress -= deltaTime * bird->speed;
                if (!bird->reverse)
                    bird->progress += deltaTime * bird->speed;

                // Ping pong path behavior
                if (bird->progress > 1.0f)
                    bird->reverse = true;
                if (bird->progress < 0.0f)
                    bird->reverse = false;

                //bird->progress = std::min(std::max(bird->progress, 0.0f), 1.0f);

                // Move bird towards new point
                glm::vec2 newPoint = bird->path->getBezierPoint(bird->progress);
                bird->entity->transformation.x = naive_lerp(bird->entity->transformation.x, newPoint.x, deltaTime);
                bird->entity->transformation.z = naive_lerp(bird->entity->transformation.z, newPoint.y, deltaTime);

                // Rotate bird to face direction
                glm::vec2 difference = glm::normalize(newPoint - glm::vec2(bird->entity->transformation.x, bird->entity->transformation.z));
                float angle = atan2(difference.y, difference.x);
                bird->entity->transformation.yaw = naive_lerp(bird->entity->transformation.yaw, glm::radians(glm::degrees(-angle) - 90.0f), deltaTime * 5.0);

                
            }

		}

        // Update player visually
		{
            // Rotate player to match movement direction
            glm::vec2 difference = glm::normalize(glm::vec2(player->previousTransformation.x, player->previousTransformation.z) - glm::vec2(player->transformation.x, player->transformation.z));
            if (glm::length(difference) > 0)
            {
				float angle = atan2(difference.y, difference.x);
	            player->transformation.yaw = naive_lerp_loop(player->transformation.yaw, glm::radians(glm::degrees(-angle) + 90.0f), deltaTime * 5.0, glm::radians(360.0));
            }
		}

        // Rotate lighthouse beam
		{
            lighthouseBeam->transformation.yaw += glm::radians(deltaTime * 12.0f);
		}

        // Rotate coins
		{
			for (Entity* pickup : pickups)
			{
                if (!pickup->bIsEnabled) continue;
                pickup->transformation.yaw += glm::radians(deltaTime * 120.0f);
			}
		}

        // Update skybox
		{
            Skybox->transformation.pitch += glm::radians(deltaTime * 1.0f);
            // Place it on the player so it appears to be infinitely away
            Skybox->transformation.x = player->transformation.x;
            Skybox->transformation.z = player->transformation.z;
            Skybox->transformation.y = camera.Position.y - 10.f;
		}

        // Update shader variables
        glm::vec3 playerPosition = glm::vec3(player->transformation.x, player->transformation.y, player->transformation.z);
        glm::vec3 evilmanPosition = glm::vec3(evilman->transformation.x, evilman->transformation.y, evilman->transformation.z);

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 500.f);

    	for (Shader* shader : shaders)
        {
            glUseProgram(shader->shaderProgram);
            glUniform1f(shader->locations["timePassedLocation"], (float)currentFrameTime);
            glUniform1i(shader->locations["bUseTextureLoc"], 1);
            glUniformMatrix4fv(shader->locations["viewLoc"], 1, GL_FALSE, glm::value_ptr(view));
            glUniform3fv(shader->locations["viewPosLoc"], 1, &camera.Position[0]);
            glUniformMatrix4fv(shader->locations["projectionLoc"], 1, GL_FALSE, glm::value_ptr(projection));
			glUniform3fv(shader->locations["playerPosLoc"], 1, &playerPosition[0]);
			glUniform3fv(shader->locations["evilmanPosLoc"], 1, &evilmanPosition[0]);

            glUniform1i(shader->locations["texture0Loc"], 0);
            glUniform1i(shader->locations["texture1Loc"], 1);
            glUniform1i(shader->locations["texture2Loc"], 2);
        }

        player->previousTransformation = player->transformation;

        if (isWireframeModeEnabled) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            Shader* shader = wireframeShader;
            glUseProgram(shader->shaderProgram);
            for (int i = 0; i < currentLevel->entities.size(); i++)
            {
                Entity* entity = currentLevel->entities[i];
                if (!entity->bIsEnabled) continue;
                glBindVertexArray( entity->VAO);
                glm::mat4 entityMatrix = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
                glm::vec3 translation = glm::vec3(entity->transformation.x, entity->transformation.y, entity->transformation.z);

                // Account for surface displacement if the entity is configured to do so
                if (entity->bIsAffectedByTerrain)
                    translation.y += currentLevel->GetGroundZAt2dCoord(translation.x, translation.z);

                entityMatrix = glm::translate(entityMatrix, translation);
                entityMatrix = glm::rotate(entityMatrix, entity->transformation.pitch, glm::vec3(1.0f, 0.0f, 0.0f));
                entityMatrix = glm::rotate(entityMatrix, entity->transformation.yaw, glm::vec3(0.0f, 1.0f, 0.0f));
                entityMatrix = glm::rotate(entityMatrix, entity->transformation.roll, glm::vec3(0.0f, 0.0f, 1.0f));

                entityMatrix = glm::scale(entityMatrix, glm::vec3(entity->transformation.scale_x, entity->transformation.scale_y, entity->transformation.scale_z));

                //std::cout << "TRANSFORM " << entity->transformation.x << ", " << entity->transformation.y << ", " << entity->transformation.z << std::endl;

                glUniformMatrix4fv(shader->locations["entityMatrixLoc"], 1, GL_FALSE, glm::value_ptr(entityMatrix));

                glDrawElements(CurrentRenderMode, entity->indices.size(), GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);
            }
        }

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        //std::cout << "Level ents " << currentLevel->entities.size() << std::endl;
        for (int i = 0; i < currentLevel->entities.size(); i++)
        {
            Entity* entity = currentLevel->entities[i];
            if (!entity->bIsEnabled) continue;

            Material* material;
            if (entity->material)
            {
                material = entity->material;
            }
            else
            {
                material = defaultMaterial;
            }

            if (entity->bHasAlpha)
            {
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	            
            }
            Shader* shader = material->shader;
            //std::cout << shader->name << std::endl;
            // draw our first triangle
            glUseProgram(shader->shaderProgram);
            {
                int textureOffset = 0;
                for (Texture* texture : material->textures)
                {
                    glActiveTexture(GL_TEXTURE0 + textureOffset); // Texture unit 0
                    glBindTexture(GL_TEXTURE_2D, texture->index);
                    textureOffset++;
                }
            }

            glBindVertexArray(entity->VAO);
            // Calculate the entity matrix
            glm::mat4 entityMatrix = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
            glm::vec3 translation = glm::vec3(entity->transformation.x, entity->transformation.y, entity->transformation.z);

            // Account for surface displacement if the entity is configured to do so
            if (entity->bIsAffectedByTerrain)
				translation.y += currentLevel->GetGroundZAt2dCoord(translation.x, translation.z);

            entityMatrix = glm::translate(entityMatrix, translation);
            entityMatrix = glm::rotate(entityMatrix, entity->transformation.pitch, glm::vec3(1.0f, 0.0f, 0.0f));
            entityMatrix = glm::rotate(entityMatrix, entity->transformation.yaw, glm::vec3(0.0f, 1.0f, 0.0f));
            entityMatrix = glm::rotate(entityMatrix, entity->transformation.roll, glm::vec3(0.0f, 0.0f, 1.0f));

            entityMatrix = glm::scale(entityMatrix, glm::vec3(entity->transformation.scale_x, entity->transformation.scale_y, entity->transformation.scale_z));

        	//std::cout << "TRANSFORM " << entity->transformation.x << ", " << entity->transformation.y << ", " << entity->transformation.z << std::endl;

            glUniformMatrix4fv(shader->locations["entityMatrixLoc"], 1, GL_FALSE, glm::value_ptr(entityMatrix));

            glDrawElements(CurrentRenderMode, entity->indices.size(), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
            glDisable(GL_BLEND);
        }

        /*ImGui::SeparatorText("Use [W A S D] to Move");
        ImGui::SeparatorText("Hold [Left Shift] to sprint");
        ImGui::SeparatorText("Hold [Space] to control camera");
        ImGui::SeparatorText("Use Mouse to move camera view");*/

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    for (Entity* entity : currentLevel->entities)
    {
        glDeleteVertexArrays(1, &entity->VAO);
        glDeleteBuffers(1, &entity->VBO);
    }
    for (Shader* shader : shaders)
    {
		glDeleteProgram(shader->shaderProgram);
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// Convert input to directional vector


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------

bool pressedKeyLookup[GLFW_KEY_LAST];
bool hasKeyJustBeenPressed(GLFWwindow* window, int key) {
    bool pressed = glfwGetKey(window, key) == GLFW_PRESS;
    bool wasPressed = pressedKeyLookup[key];
    pressedKeyLookup[key] = pressed;

    if (pressed && !wasPressed) return true;
    return false;
}
// Returns if the camera is in use
bool processInput(GLFWwindow* window, float deltaTime)
{
    if (hasKeyJustBeenPressed(window, GLFW_KEY_ESCAPE))
        glfwSetWindowShouldClose(window, true);

    bool bUseCameraControls = (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);

    if (!bUseCameraControls)
    {
        float playerSpeed = 5.0f;
        glm::vec2 playerInput = glm::vec2(0, 0);
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            playerSpeed *= 2;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            playerInput.y += 1.0f;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            playerInput.y -= 1.0f;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            playerInput.x -= 1.0f;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            playerInput.x += 1.0f;

        glm::vec3 playerImpulse =
            ((camera.Front * playerInput.y) +
                (camera.Right * playerInput.x))
            * playerSpeed * deltaTime;

        player->transformation.x += playerImpulse.x;
        player->transformation.z += playerImpulse.z;
    }


    //std::cout << "PLAYER TRANSFORM " << player->transformation.x << ", " << player->transformation.y << ", " << player->transformation.z << std::endl;

    if (bUseCameraControls)
    {
        float cameraSpeed = deltaTime * 2.0f;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            cameraSpeed *= 2;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.ProcessKeyboard(FORWARD, cameraSpeed);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.ProcessKeyboard(BACKWARD, cameraSpeed);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.ProcessKeyboard(LEFT, cameraSpeed);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.ProcessKeyboard(RIGHT, cameraSpeed);
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            camera.ProcessKeyboard(UP, cameraSpeed);
        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
		    camera.ProcessKeyboard(DOWN, cameraSpeed);
    }

    // Wireframe mode toggle
    if (hasKeyJustBeenPressed(window, GLFW_KEY_T)) {
        isWireframeModeEnabled = !isWireframeModeEnabled;
    }

    // Render mode toggle
    if (hasKeyJustBeenPressed(window, GLFW_KEY_R)) {
	    if (CurrentRenderMode == GL_LINE_STRIP) {
            std::cout << "Switching to GL_TRIANGLES" << std::endl;
	    	CurrentRenderMode = GL_TRIANGLES;
		} else {
            std::cout << "Switching to GL_LINE_STRIP" << std::endl;
			CurrentRenderMode = GL_LINE_STRIP;
		}
	}

    return bUseCameraControls;
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    if (ImGui::GetIO().WantCaptureMouse) return;
    if (!currentLevel->bIsCameraControllable) return;

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
    if (ImGui::GetIO().WantCaptureMouse) return;

    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
    std::cout << " windows resized with " << width << " Height " << height << std::endl;
}