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

int width = 1200;
int height = 1200;

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

	// Shader
	const auto vertex = MakeShader(GL_VERTEX_SHADER, "resources/shaders/shaderStl.vert");
	const auto fragment = MakeShader(GL_FRAGMENT_SHADER, "resources/shaders/shaderStl.frag");

	// Program
	const auto program = AttachAndLink({ vertex, fragment });
	glUseProgram(program);


	// Quads
	//Vertices
	float triangles[] = {
		// positions          // colors           // texture coords
		 0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
		 0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
		-0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
		-0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left 
	};
	//Indices
	unsigned int indices[] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};

	// Buffers
	GLuint vao, vbo, ebo;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangles), triangles, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	
	// Bindings
	//Position
	const auto indexPosition = glGetAttribLocation(program, "position");
	glVertexAttribPointer(indexPosition, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(indexPosition);

	//Color
	const auto indexColor = glGetAttribLocation(program, "color");
	glVertexAttribPointer(indexColor, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(indexColor);

	//UV
	const auto indexTexUV = glGetAttribLocation(program, "texuv");
	glVertexAttribPointer(indexTexUV, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
	glEnableVertexAttribArray(indexTexUV);


	//Texture
	const char *filename = "resources/images/wall.jpg";
	Image im = LoadImage(filename);

	GLuint tex;
	glCreateTextures(GL_TEXTURE_2D, 1, &tex);
	glTextureStorage2D(tex, 1, GL_RGB8, im.width, im.height);

	GLuint texUnit = {0};
	glTextureSubImage2D(tex, 0, 0, 0, im.width, im.height, GL_RGB, GL_UNSIGNED_BYTE, im.data);
	glBindTextureUnit(texUnit, tex);


	//FrameBuffer
	GLuint fbo;
	glCreateFramebuffers(1, &fbo);

	GLuint ct;
	glCreateTextures(GL_TEXTURE_2D, 1, &ct);
	glTextureStorage2D(ct, 1, GL_RGB8, width, height);

	GLuint dt;
	glCreateTextures(GL_TEXTURE_2D, 1, &dt);
	glTextureStorage2D(dt, 1, GL_DEPTH_COMPONENT32F, width, height);

	glNamedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, ct, 0);
	glNamedFramebufferTexture(fbo, GL_DEPTH_ATTACHMENT, dt, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	//Options
	glPointSize(10.f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_PROGRAM_POINT_SIZE);

	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);



		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	stbi_image_free(im.data);

	glfwDestroyWindow(window);
	glfwTerminate();

	//system("pause");

	exit(EXIT_SUCCESS);
}
