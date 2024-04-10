#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Helper
{
	static glm::vec3 Forward(float Pitch, float Yaw, float Roll)
	{
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        return glm::normalize(front);
	}

    static glm::vec3 Right(glm::vec3 Front)
    {
        return glm::normalize(glm::cross(Front, glm::vec3(0.0, 1.0, 0.0)));
    }
    static glm::vec3 Up(glm::vec3 Right, glm::vec3 Front)
    {
        return glm::normalize(glm::cross(Right, Front));
    }
};
