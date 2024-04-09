// source 
//https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/1.getting_started/2.1.hello_triangle/hello_triangle.cpp

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Types.h" // Generic types used in the project
#include "ObjectFileLoader.h" // Can load and prepare .obj files to be rendered
#include "ShaderLoader.h" // Can load and prepare shader files  
#include "Level.h" // Handles loading level meshes
#include "Surface.h" // Surface function and generation
#include "Camera.h" // Handles camera controls and updates

// If anything happens to your frame, this is called
// resizing etc
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// settings
unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 800;

// camera
Camera camera(glm::vec3(0.0f, 2.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;	// time between current frame and last frame

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

int CurrentRenderMode = GL_TRIANGLES;

Entity* player = new Entity();

int main(int argc, char** argv)
{
    player->RadiusCollisionSize = 0.5f;
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
    Level level = Level(levelFile);
    camera.Position = glm::vec3(
        level.cameraPosition.x,
        level.cameraPosition.y,
        level.cameraPosition.z
    );

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
#pragma region Shader Setup

    std::string vertexShaderSourceStr = ShaderLoader::LoadShaderFromFile("svert.glsl");
    const char* vertexShaderSource = vertexShaderSourceStr.c_str();
    std::string fragmentShaderSourceStr = ShaderLoader::LoadShaderFromFile("sfrag.glsl");
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

    // set up shader variables
    unsigned int timePassedLocation = glGetUniformLocation(shaderProgram, "timePassed");
    unsigned int bUseTextureLoc = glGetUniformLocation(shaderProgram, "bUseTexture");
    unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
    unsigned int viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
    unsigned int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    unsigned int entityMatrixLoc = glGetUniformLocation(shaderProgram, "entityMatrix");

#pragma endregion
#pragma region Buffer Mesh Loading

    CurrentRenderMode = GL_TRIANGLES;

#pragma region +Surface Creation
    Entity* surface = new Entity();
    Surface::GenerateSurface(-20.0, 20.0, -20.0, 20.0, 40, surface->vertices, surface->indices);
    level.entities.push_back(surface);
    //Surface::GenerateTrees(level, -20.0, 20.0, -20.0, 20.0, 50);

#pragma endregion

    ReadObjectFile("player.obj", player->vertices, player->indices);
    level.entities.push_back(player);

    std::cout << "Entities: " << level.entities.size() << std::endl;
    for (int i = 0; i < level.entities.size(); i++)
    {
        unsigned int VBO, VAO, EBO;
        Entity* entity = level.entities[i];

        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, entity->vertices.size() * sizeof(Vertex), entity->vertices.data(), GL_STATIC_DRAW);

        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, entity->indices.size() * sizeof(int), entity->indices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);

        std::cout << i << ": " << VAO << std::endl;
        entity->VAO = VAO;
        entity->VBO = VBO;
        entity->EBO = EBO;
	}

    glEnable(GL_DEPTH_TEST);
    glDepthRange(0.0, 10000.0);

#pragma endregion

#pragma region Texture Loading
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nChannels;
    unsigned char* data = stbi_load(textureFileName.c_str(), &width, &height, &nChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture";
    }
    stbi_image_free(data);
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
		
        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glClear(GL_DEPTH_BUFFER_BIT);
        
		// Update shader variables
		glUniform1f(timePassedLocation, (float) currentFrameTime);

        glBindTexture(GL_TEXTURE_2D, texture);

        glUniform1i(bUseTextureLoc, 1);

        glm::mat4 view = camera.GetViewMatrix();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        glUniformMatrix4fv(viewPosLoc, 1, GL_FALSE, glm::value_ptr(camera.Position));

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        //camera.updateCameraVectors();

        // Do collision checks
		{
            // Top down 2D collisions (radius)
            glm::vec2 currentPosition = glm::vec2(player->transformation.x, player->transformation.z);
            for (int i = 0; i < level.entities.size(); i++)
            {
                Entity* entity = level.entities[i];
                if (!entity->bHasRadiusCollision) continue;

                glm::vec2 difference = currentPosition - glm::vec2(entity->transformation.x, entity->transformation.z);
                float distance = glm::length(difference);
                float contactDistance = (player->RadiusCollisionSize + entity->RadiusCollisionSize);
                if (distance < contactDistance)
                {
                    // We are colliding with something!
                    // Move the player away from the object
                    glm::vec2 newPosition = glm::vec2(entity->transformation.x, entity->transformation.z) + glm::normalize(difference) * contactDistance;
                    player->transformation.x = newPosition.x;
                    player->transformation.z = newPosition.y;
                }
            }
            player->previousTransformation = player->transformation;
		}

        for (int i = 0; i < level.entities.size(); i++)
        {
            Entity* entity = level.entities[i];
            // draw our first triangle
            glUseProgram(shaderProgram);

            glBindVertexArray(i + 1);
            // Calculate the entity matrix
            glm::mat4 entityMatrix = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
            glm::vec3 translation = glm::vec3(entity->transformation.x, entity->transformation.y, entity->transformation.z);

            // Account for surface displacement, but ignore the surface so that it won't be applied twice
            if (entity != surface)
				translation.y += Surface::GetGroundZAt2dCoord(translation.x, translation.z);

            entityMatrix = glm::translate(entityMatrix, translation);
            entityMatrix = glm::rotate(entityMatrix, entity->transformation.pitch, glm::vec3(1.0f, 0.0f, 0.0f));
            entityMatrix = glm::rotate(entityMatrix, entity->transformation.yaw, glm::vec3(0.0f, 1.0f, 0.0f));
            entityMatrix = glm::rotate(entityMatrix, entity->transformation.roll, glm::vec3(0.0f, 0.0f, 1.0f));

        	std::cout << "TRANSFORM " << entity->transformation.x << ", " << entity->transformation.y << ", " << entity->transformation.z << std::endl;

            glUniformMatrix4fv(entityMatrixLoc, 1, GL_FALSE, glm::value_ptr(entityMatrix));

            glDrawElements(CurrentRenderMode, entity->indices.size(), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0); 
        }

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    for (Entity* entity : level.entities)
    {
        glDeleteVertexArrays(1, &entity->VAO);
        glDeleteBuffers(1, &entity->VBO);
    }
    glDeleteProgram(shaderProgram);

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

bool isWireframeModeEnabled = false;
void processInput(GLFWwindow* window)
{
    if (hasKeyJustBeenPressed(window, GLFW_KEY_ESCAPE))
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_KP_8) == GLFW_PRESS)
        player->transformation.z -= 0.1f;
    if (glfwGetKey(window, GLFW_KEY_KP_5) == GLFW_PRESS)
        player->transformation.z += 0.1f;
    if (glfwGetKey(window, GLFW_KEY_KP_4) == GLFW_PRESS)
        player->transformation.x -= 0.1f;
    if (glfwGetKey(window, GLFW_KEY_KP_6) == GLFW_PRESS)
        player->transformation.x += 0.1f;

    std::cout << "PLAYER TRANSFORM " << player->transformation.x << ", " << player->transformation.y << ", " << player->transformation.z << std::endl;

    float cameraSpeed = deltaTime;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        cameraSpeed *= 2;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        cameraSpeed *= 2;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, cameraSpeed);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, cameraSpeed);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, cameraSpeed);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, cameraSpeed);

    // Wireframe mode toggle
    if (hasKeyJustBeenPressed(window, GLFW_KEY_T)) {
        if (isWireframeModeEnabled) {
            std::cout << "Wireframe mode disabled" << std::endl;
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        } else {
            std::cout << "Wireframe mode enabled" << std::endl;
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
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
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
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