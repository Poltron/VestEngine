
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
#include <fstream>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Shader.h"

glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float verticalAxis = 0.0f;
float horizontalAxis = 0.0f;

int lastXPos = -1.0f;
int xPos = 0.0f;
int lastYPos = -1.0f;
int yPos = 0.0f;

float cameraTranslationSpeed = 25.0f;
float cameraRotationSpeed = 25.0f;

bool floatEquals(float a, float b)
{
	float r = a - b;
	return std::fabs(r) < FLT_EPSILON;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		verticalAxis = 1.0f;
	}
	else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		verticalAxis = -1.0f;
	}
	else
	{
		verticalAxis = 0.0f;
	}

	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		horizontalAxis = 1.0f;
	}
	else if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		horizontalAxis = -1.0f;
	}
	else
	{
		horizontalAxis = 0.0f;
	}

	double xPosD, yPosD;
	glfwGetCursorPos(window, &xPosD, &yPosD);
	xPos = xPosD;
	yPos = yPosD;

	if (lastYPos == -1)
	{
		lastYPos = yPos;
	}
	if (lastXPos == -1)
	{
		lastXPos = xPos;
	}
	std::cout << "Cursor: " << xPos << ":" << yPos << std::endl;
}

void computeCameraPosition(glm::vec3& cameraPosition, double deltaTime)
{
	float speed = cameraTranslationSpeed * deltaTime;

	if (!floatEquals(horizontalAxis, 0))
	{
		cameraPosition += glm::normalize(glm::cross(cameraFront, cameraUp)) * horizontalAxis * speed;
	}
	
	if (!floatEquals(verticalAxis, 0))
	{
		cameraPosition += cameraFront * verticalAxis * speed;
	}
}

void computeCameraRotation(glm::mat4& model, double deltaTime)
{
	float speed = cameraRotationSpeed * deltaTime;

	int deltaY = yPos - lastYPos;
	int deltaX = xPos - lastXPos;

	if (deltaX != 0)
	{
		const glm::vec4 upAxis = glm::vec4(0, deltaX, 0, 0);

		glm::vec3 cameraUpAxis = upAxis;
		model = glm::rotate(model, glm::radians(speed), cameraUpAxis);
	}

	if (deltaY != 0)
	{
		const glm::vec4 rightAxis = glm::vec4(deltaY, 0, 0, 0);

		glm::vec3 cameraRightAxis = rightAxis;
		model = glm::rotate(model, glm::radians(speed), cameraRightAxis);
	}

	lastYPos = yPos;
	lastXPos = xPos;
}


GLFWwindow* CreateWindow(int width, int height)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(width, height, "VestEngine", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return nullptr;
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return nullptr;
	}

	glViewport(0, 0, 800, 600);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	
	return window;
}

unsigned int loadTexture(const char* texturePath, GLenum textureFormat)
{
	stbi_set_flip_vertically_on_load(true);

	int width, height, nrChannels;
	unsigned char* data = stbi_load(texturePath, &width, &height, &nrChannels, 0);
	if (!data)
	{
		std::cout << "Failed to load texture: " << texturePath << std::endl;
	}

	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, textureFormat, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(data);

	return textureID;
}

int main()
{
	int width = 800;
	int height = 600;
	GLFWwindow* window = CreateWindow(width, height);
	if (!window)
	{
		return -1;
	}

	float vertices[] = {
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};

	unsigned int texture0 = loadTexture("../Resources/container.jpg", GL_RGB);
	unsigned int texture1 = loadTexture("../Resources/awesomeface.png", GL_RGBA);

	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	unsigned int VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//unsigned int EBO;
	//glGenBuffers(1, &EBO);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	unsigned int vertexAttribPointer = 0; // vertex loayout location in vertex shader
	glVertexAttribPointer(vertexAttribPointer, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(vertexAttribPointer);

	vertexAttribPointer = 1; // texcoord layout location
	glVertexAttribPointer(vertexAttribPointer, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(vertexAttribPointer);
	 
	//vertexAttribPointer = 1; // color layout location
	//glVertexAttribPointer(vertexAttribPointer, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	//glEnableVertexAttribArray(vertexAttribPointer);

	//vertexAttribPointer = 2; // texcoord layout location
	//glVertexAttribPointer(vertexAttribPointer, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	//glEnableVertexAttribArray(vertexAttribPointer);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture1);

	glBindVertexArray(0);
	
	glEnable(GL_DEPTH_TEST);

	Shader shader;
	shader.load("vertex.glsl", "fragment.glsl");
	
	//////
	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);

	glm::mat4 view;
	//////

	const float fov = glm::radians(45.0f);
	const float aspectRatio = (float)width / (float)height;
	const float near = 0.1f;
	const float far = 100.0f;

	glm::mat4 projection = glm::mat4(1.0f);
	projection = glm::perspective(fov, aspectRatio, near, far);

	glm::vec3 cubePositions[] = {
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};

	double Time = glfwGetTime();

	while (!glfwWindowShouldClose(window))
	{
		double ActualTime = glfwGetTime();
		double deltaTime = ActualTime - Time;
		Time = ActualTime;

		// input
		processInput(window);
		computeCameraPosition(cameraPos, deltaTime);
		//computeCameraRotation(view, deltaTime);

		// render
		glClearColor(0.3f, 0.3f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.use();

		float timeValue = glfwGetTime();
		float alpha = (sin(timeValue) / 2.0f) + 0.5f;
		shader.setFloat("alpha", alpha);
		shader.setInt("texture0", 0);
		shader.setInt("texture1", 1);
		
		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		shader.setMat4("view", glm::value_ptr(view));
		shader.setMat4("projection", glm::value_ptr(projection));

		glBindVertexArray(VAO);

		for (unsigned int i = 0; i < 10; i++)
		{
			glm::mat4 cubeModel = glm::translate(glm::mat4(1.0f), cubePositions[i]);
			cubeModel = glm::rotate(cubeModel, glm::radians(20.0f * i), glm::vec3(1, 0.3f, 0.5f));
			shader.setMat4("model", glm::value_ptr(cubeModel));
			
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		glBindVertexArray(0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}