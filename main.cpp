#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>

#include <vector>
#include <iostream>
#include <random>
#include <sstream>
#include <fstream>
#include <string>

/*

UNCOMMENT when you'll start using tinyply.

#define TINYPLY_IMPLEMENTATION
#include <tinyply.h>
*/

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

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
struct Particule {
	glm::vec3 position;
	glm::vec3 color;
	glm::vec3 speed;
	float size;
};

std::vector<Particule> MakeParticules(const int n)
{
	std::default_random_engine generator;
	std::uniform_real_distribution<float> distribution01(0, 1);
	std::uniform_real_distribution<float> distributionWorld(-1, 1);

	std::vector<Particule> p;
	p.reserve(n);

	for(int i = 0; i < n; i++)
	{
		p.push_back(Particule{
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

GLuint MakeShader(GLuint t, std::string path)
{
	std::cout << path << std::endl;
	std::ifstream file(path.c_str(), std::ios::in);
	std::ostringstream contents;
	contents << file.rdbuf();
	file.close();

	const auto content = contents.str();
	std::cout << content << std::endl;

	const auto s = glCreateShader(t);

	GLint sizes[] = {(GLint) content.size()};
	const auto data = content.data();

	glShaderSource(s, 1, &data, sizes);
	glCompileShader(s);

	GLint success;
	glGetShaderiv(s, GL_COMPILE_STATUS, &success);
	if(!success)
	{
		GLchar infoLog[512];
		GLsizei l;
		glGetShaderInfoLog(s, 512, &l, infoLog);

		std::cout << infoLog << std::endl;
	}

	return s;
}

GLuint AttachAndLink(std::vector<GLuint> shaders)
{
	const auto prg = glCreateProgram();
	for(const auto s : shaders)
	{
		glAttachShader(prg, s);
	}

	glLinkProgram(prg);

	GLint success;
	glGetProgramiv(prg, GL_LINK_STATUS, &success);
	if(!success)
	{
		GLchar infoLog[512];
		GLsizei l;
		glGetProgramInfoLog(prg, 512, &l, infoLog);

		std::cout << infoLog << std::endl;
	}

	return prg;
}

void APIENTRY opengl_error_callback(GLenum /* source */,
									GLenum /* type */,
									GLuint /* id */,
									GLenum /* severity */,
									GLsizei /* length */,
									const GLchar *message,
									const void * /*userParam */)
{
	std::cout << message << std::endl;
}

static const float g = 3.711; //m.s-2
static const glm::vec3 gravityDirection(0.0f, 1.0f, 0.0f);

void ApplyGravity(std::vector<Particule> &particules) {

	for (auto &particle : particules)
	{

		//Compute acceleration
		glm::vec3 acceleration = (-particle.size * g * gravityDirection - particle.speed) / particle.size;

		//Compute next speed & position
		glm::vec3 nextSpeed = particle.speed + acceleration * deltaTime;
		glm::vec3 nextPosition = particle.position + particle.speed * deltaTime;

		particle.speed = nextSpeed;
		particle.position = nextPosition;
	}

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

	window = glfwCreateWindow(1000, 1000, "Simple example", NULL, NULL);

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
		exit(-1);
	}

	// Callbacks
	glDebugMessageCallback(opengl_error_callback, nullptr);

	const size_t nParticules = 10;
	auto particules = MakeParticules(nParticules);

	// Shader
	//const auto vertex = MakeShader(GL_VERTEX_SHADER, "shader.vert");
	//const auto fragment = MakeShader(GL_FRAGMENT_SHADER, "shader.frag");

	const auto vertex = MakeShader(GL_VERTEX_SHADER, "shaderGravity.vert");
	const auto fragment = MakeShader(GL_FRAGMENT_SHADER, "shaderGravity.frag");

	const auto program = AttachAndLink({vertex, fragment});

	glUseProgram(program);


	// Buffers
	GLuint vbo, vao;
	glGenBuffers(1, &vbo);
	glGenVertexArrays(1, &vao);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, nParticules * sizeof(Particule), particules.data(), GL_DYNAMIC_DRAW);

	// Bindings
	//Position
	const auto index = glGetAttribLocation(program, "position");
	glVertexAttribPointer(index, 3, GL_FLOAT, GL_FALSE, sizeof(Particule), reinterpret_cast<GLvoid *>(offsetof(Particule, position)));
	glEnableVertexAttribArray(index);

	//Color
	const auto index_color = glGetAttribLocation(program, "color");
	glVertexAttribPointer(index_color, 3, GL_FLOAT, GL_FALSE, sizeof(Particule), reinterpret_cast<GLvoid *>(offsetof(Particule, color)));
	glEnableVertexAttribArray(index_color);

	//Pointsize
	const auto index_pointSize = glGetAttribLocation(program, "pointSize");
	glVertexAttribPointer(index_pointSize, 1, GL_FLOAT, GL_FALSE, sizeof(Particule), reinterpret_cast<GLvoid *>(offsetof(Particule, size)));
	glEnableVertexAttribArray(index_pointSize);


	glPointSize(10.f);
	glEnable(GL_PROGRAM_POINT_SIZE);

	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);



		ApplyGravity(particules);
		glBufferSubData(GL_ARRAY_BUFFER, 0, nParticules * sizeof(Particule), particules.data());


		//double xpos, ypos;
		//glfwGetCursorPos(window, &xpos, &ypos);

		


		///* For shader.XXXX */
		////Time
		//glUniform1f(glGetUniformLocation(program, "time"), glfwGetTime());

		////Mouse position
		//double xpos, ypos;
		//glfwGetCursorPos(window, &xpos, &ypos);
		//glUniform2f(glGetUniformLocation(program, "mouse_position"), xpos/width, ypos/height);

		////Colors
		//glUniform3f(glGetUniformLocation(program, "colorA"), 0.5f, 1.0f, 0.0f);
		//glUniform3f(glGetUniformLocation(program, "colorB"), 0.17f, 0.45f, 1.0f);


		/* For shaderGravity.XXXX */
		//Time
		glUniform1f(glGetUniformLocation(program, "time"), glfwGetTime());




		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glDrawArrays(GL_POINTS, 0, nParticules);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	//system("pause");

	exit(EXIT_SUCCESS);
}
