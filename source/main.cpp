#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <iostream>
#include <random>
#include <sstream>
#include <fstream>
#include <string>

#include "shader.h"

#define TINYPLY_IMPLEMENTATION
#include "tinyply.h"

#include "stl.h"
#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int width, height;

static void error_callback(int /*error*/, const char* description)
{
	std::cerr << "Error: " << description << std::endl;
}

static void key_callback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

/* PARTICULES */
struct Particle {
	glm::vec3 position;
	glm::vec3 color;
	glm::vec3 speed;
	float size;
};

std::default_random_engine generator;
std::uniform_real_distribution<float> distribution01(0, 1);
std::uniform_real_distribution<float> distributionWorld(-1, 1);

std::vector<Particle> MakeParticles(const int n) {

	std::vector<Particle> p;
	p.reserve(n);

	for(int i = 0; i < n; i++)
	{
		p.push_back(Particle{
				{
				distributionWorld(generator),
				distributionWorld(generator),
				distributionWorld(generator)
				},
				{
				distribution01(generator),
				distribution01(generator),
				distribution01(generator)
				},
				{0.f, 0.f, 0.f},
				20.0f * distribution01(generator)
				});
	}

	return p;
}


void APIENTRY opengl_error_callback(GLenum source,
		GLenum type,
		GLuint id,
		GLenum severity,
		GLsizei length,
		const GLchar *message,
		const void *userParam)
{
	std::cout << message << std::endl;
}

static const float g = 3.711; //m.s-2
static const glm::vec3 gravityDirection(0.0f, 1.0f, 0.0f);

bool IsOutsideScreen(glm::vec3 pos) {
	return pos.y < -1.0f || pos.x < -1.0f ||pos.x > 1.0f;
}

glm::vec3 ScreenCoordinatesToWorldCoordinates(double &xpos, double &ypos) {
	return glm::vec3(xpos / width * 2.0f - 1.0f, (ypos / height * 2.0f - 1.0f) * -1.0f, 1.0f);
}

void ApplyGravity(std::vector<Particle> &particules, double &xpos, double &ypos) {

	for (auto &particle : particules)
	{

		if (IsOutsideScreen(particle.position)) {
			particle.position = ScreenCoordinatesToWorldCoordinates(xpos, ypos);
			particle.speed = glm::vec3(distribution01(generator) * -1.0f, distribution01(generator) * -1.0f, 0.0f) * 5.0f;
		}

		//Compute acceleration
		glm::vec3 acceleration = (-particle.size * g * gravityDirection - particle.speed) / particle.size;

		//Compute next speed & position
		glm::vec3 nextSpeed = particle.speed + acceleration * deltaTime;
		glm::vec3 nextPosition = particle.position + particle.speed * deltaTime;

		particle.speed = nextSpeed;
		particle.position = nextPosition;
	}

}

Image LoadImage(const char *filename) {
	int width, height, nbComponents;
	unsigned char *picture = stbi_load(filename, &width, &height, &nbComponents, 0);

	if (!picture)
	{
		stbi_image_free(picture);
		throw std::runtime_error("File not found: " + std::string(filename));
		exit(EXIT_FAILURE);
	}

	return { picture, width, height };
}

int main(void)
{
	GLFWwindow* window;
	glfwSetErrorCallback(error_callback);

	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

	window = glfwCreateWindow(1200, 1200, "Simple example", NULL, NULL);

	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwSetKeyCallback(window, key_callback);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	// NOTE: OpenGL error checks have been omitted for brevity

	if(!gladLoadGL()) {
		std::cerr << "Something went wrong!" << std::endl;
		exit(EXIT_FAILURE);
	}

	// Callbacks
	glDebugMessageCallback(opengl_error_callback, nullptr);


	//const size_t nParticles = 1000;
	//const auto particules = MakeParticles(nParticles);

	const auto stlMesh = ReadStl("resources/models/logo.stl");

	// Shader
	const auto vertex = MakeShader(GL_VERTEX_SHADER, "resources/shaders/shaderStl.vert");
	const auto fragment = MakeShader(GL_FRAGMENT_SHADER, "resources/shaders/shaderStl.frag");


	const auto program = AttachAndLink({vertex, fragment});

	glUseProgram(program);


	// Buffers
	GLuint vbo, vao;
	glGenBuffers(1, &vbo);
	glGenVertexArrays(1, &vao);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glBufferData(GL_ARRAY_BUFFER, stlMesh.size() * sizeof(Triangle), stlMesh.data(), GL_DYNAMIC_DRAW);

	
	// Bindings
	//Position
	const auto index = glGetAttribLocation(program, "position");
	glVertexAttribPointer(index, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(index);


	//Texture

	const char *filename = "resources/images/tronc.jpg";
	Image im = LoadImage(filename);
	GLuint tex;
	glCreateTextures(GL_TEXTURE_2D, 1, &tex);
	glTextureStorage2D(tex, 1, GL_RGB8, im.width, im.height);



	glEnable(GL_DEPTH_TEST);

	glPointSize(10.f);
	glEnable(GL_PROGRAM_POINT_SIZE);

	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDrawArrays(GL_TRIANGLES, 0, stlMesh.size() * 3);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	//system("pause");

	exit(EXIT_SUCCESS);
}
