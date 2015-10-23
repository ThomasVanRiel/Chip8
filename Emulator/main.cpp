#include <iostream>
#include <sstream>
#include <Windows.h>

#include "Chip8.h"

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

#define SHADER_DEBUGGING

class LogBuf : public std::stringbuf {
protected:
	int sync() {
		OutputDebugString(str().c_str());
		//std::cout << str();
		str("");
		return 0;
	}
};


// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void drop_callback(GLFWwindow* window, int count, const char** paths);

void HandleInput(GLFWwindow *window);
void printShaderLog(GLuint shader);
GLuint LoadShaderFromFile(const std::string & filePath, GLenum shaderType);
void ReloadShaderFromFile(const std::string & filePath, GLuint shaderID);
bool gIsShaderError = false;
// Window dimensions
const GLuint WIDTH = 1024, HEIGHT = 512;


Chip8 emulator;


int main() {
	//Set debugging log
	LogBuf logBuf;
	std::clog.rdbuf(&logBuf);

	std::cout << "Starting GLFW context, OpenGL 3.3" << std::endl;
	// Init GLFW
	glfwInit();
	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	// Create a GLFWwindow object that we can use for GLFW's functions
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Chip8 - Emulator", NULL, NULL);
	glfwMakeContextCurrent(window);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetDropCallback(window, drop_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize OpenGL context" << std::endl;
		return -1;
	}

	// Define the viewport dimensions
	glViewport(0, 0, WIDTH, HEIGHT);

	// Create Vertex Array Object
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Create a Vertex Buffer Object and copy the vertex data to it
	GLuint vbo;
	glGenBuffers(1, &vbo);

	GLfloat vertices[] = {
		-1, 1, 0.0f, 0.0f, // Top-left
		1, 1, 1.0f, 0.0f, // Top-right
		1, -1, 1.0f, 1.0f, // Bottom-right
		-1, -1, 0.0f, 1.0f  // Bottom-left
	};

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Create an element array
	GLuint ebo;
	glGenBuffers(1, &ebo);

	GLuint elements[] = {
		0, 1, 2,
		2, 3, 0
	};

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

	// Create and compile the shaders
	GLuint vertexShader = LoadShaderFromFile("Resources/vertexShader.glsl", GL_VERTEX_SHADER);
	GLuint fragmentShader = LoadShaderFromFile("Resources/fragmentShader.glsl", GL_FRAGMENT_SHADER);

	// Link the vertex and fragment shader into a shader program
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glBindFragDataLocation(shaderProgram, 0, "outColor");
	glLinkProgram(shaderProgram);
	glUseProgram(shaderProgram);

	// Specify the layout of the vertex data
	GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

	GLint texCoordAttrib = glGetAttribLocation(shaderProgram, "texCoord");
	glEnableVertexAttribArray(texCoordAttrib);
	glVertexAttribPointer(texCoordAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));


	emulator.LoadRom("c8games/MAZE");

	srand(GetTickCount());

	GLuint texture_id;
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);  //Always set the base and max mipmap levels of a texture.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);


	__int64 countsPerSecond, prevTime, currTime;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSecond);
	float secondsPerCount = 1.0f / countsPerSecond;
	QueryPerformanceCounter((LARGE_INTEGER*)&prevTime);

	glfwSwapInterval(1);

	// Game loop
	while (!glfwWindowShouldClose(window)) {

		// Check if any events have been activated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();


		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
		float elapsedTime = (float)(currTime - prevTime) * secondsPerCount;
		float epsilon = 1.0f / 120.0f;
		if (elapsedTime > epsilon) {


		}
		emulator.DecreaseTimers();
		for (int i = 0; i < 5; i++) {

		prevTime = currTime;
		//emulator.m_Key = 0;
		HandleInput(window);

		//loop emulator
		emulator.Loop();
		}
		emulator.m_Key = 0;

#ifdef SHADER_DEBUGGING
		if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS) {
			ReloadShaderFromFile("Resources/fragmentShader.glsl", fragmentShader);
			glLinkProgram(shaderProgram);
		}
#endif


		if (emulator.m_DoRedraw) {
		}
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 64, 32, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, (GLvoid*)emulator.m_Texture);

		// Render
		// Clear the screen to white
		glClearColor(1, 1, 1, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// Draw a rectangle from the 2 triangles using 6 indices
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		// Swap the screen buffers
		glfwSwapBuffers(window);
	}

	// Terminates GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();
	return 0;
}

void HandleInput(GLFWwindow *window) {
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
		emulator.m_Key |= 1;
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
		emulator.m_Key |= 2;
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
		emulator.m_Key |= 4;
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
		emulator.m_Key |= 8;
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		emulator.m_Key |= 16;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		emulator.m_Key |= 32;
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		emulator.m_Key |= 64;
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
		emulator.m_Key |= 128;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		emulator.m_Key |= 256;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		emulator.m_Key |= 512;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		emulator.m_Key |= 1024;
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
		emulator.m_Key |= 2048;
	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
		emulator.m_Key |= 4096;
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
		emulator.m_Key |= 8196;
	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
		emulator.m_Key |= 16384;
	if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
		emulator.m_Key |= 32768;

}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	//std::cout << key << std::endl;
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_1)
		emulator.m_Key |= 1;
	if (key == GLFW_KEY_2)
		emulator.m_Key |= 2;
	if (key == GLFW_KEY_3)
		emulator.m_Key |= 4;
	if (key == GLFW_KEY_4)
		emulator.m_Key |= 8;
	if (key == GLFW_KEY_Q)
		emulator.m_Key |= 16;
	if (key == GLFW_KEY_W)
		emulator.m_Key |= 32;
	if (key == GLFW_KEY_E)
		emulator.m_Key |= 64;
	if (key == GLFW_KEY_R)
		emulator.m_Key |= 128;
	if (key == GLFW_KEY_A)
		emulator.m_Key |= 256;
	if (key == GLFW_KEY_S)
		emulator.m_Key |= 512;
	if (key == GLFW_KEY_D)
		emulator.m_Key |= 1024;
	if (key == GLFW_KEY_F)
		emulator.m_Key |= 2048;
	if (key == GLFW_KEY_Z)
		emulator.m_Key |= 4096;
	if (key == GLFW_KEY_X)
		emulator.m_Key |= 8196;
	if (key == GLFW_KEY_C)
		emulator.m_Key |= 16384;
	if (key == GLFW_KEY_V)
		emulator.m_Key |= 32768;

}

void drop_callback(GLFWwindow* window, int count, const char** paths) {
	for (int i = 0; i < count; i++) {
		std::cout << paths[i] << std::endl;
	}
	//emulator.LoadRom(paths[0]);
}

void printShaderLog(GLuint shader) {
	//Make sure name is shader 
	if (glIsShader(shader)) {
		//Shader log length 
		int infoLogLength = 0;
		int maxLength = infoLogLength;
		//Get info string length 
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
		//Allocate string 
		char* infoLog = new char[maxLength];
		//Get info log 
		glGetShaderInfoLog(shader, maxLength, &infoLogLength, infoLog);
		if (infoLogLength > 0) {
			//Print Log
			std::clog << infoLog << std::endl;
			//printf("%s\n", infoLog);
		}
		//Deallocate string 
		delete[] infoLog;
	} else {
		printf("Name %d is not a shader\n", shader);
	}
}

GLuint LoadShaderFromFile(const std::string & filePath, GLenum shaderType) {
	GLuint shaderID = 0;
	std::string shaderString;
	std::ifstream sourceFile(filePath.c_str());

	if (sourceFile) {
		//Get shader source
		shaderString.assign((std::istreambuf_iterator< char >(sourceFile)), std::istreambuf_iterator< char >());

		//Create shader ID
		shaderID = glCreateShader(shaderType);

		//Set shader source
		const GLchar* shaderSource = shaderString.c_str();
		glShaderSource(shaderID, 1, (const GLchar**)&shaderSource, NULL);

		//Compile shader source
		glCompileShader(shaderID);

		//Check shader for errors
		GLint shaderCompiled = GL_FALSE;
		glGetShaderiv(shaderID, GL_COMPILE_STATUS, &shaderCompiled);
		if (shaderCompiled != GL_TRUE) {
			std::clog << "Unable to compile shader " << shaderCompiled << "!\n\nSource:\n" << shaderSource << std::endl;
			printShaderLog(shaderID);
			glDeleteShader(shaderID);
			shaderID = 0;
		}
	} else {
		printf("Unable to open file %s\n", filePath.c_str());
	}

	return shaderID;
}

void ReloadShaderFromFile(const std::string & filePath, GLuint shaderID) {
	std::string shaderString;
	std::ifstream sourceFile(filePath.c_str());

	if (sourceFile) {
		glDeleteShader(shaderID);

		//Get shader source
		shaderString.assign((std::istreambuf_iterator< char >(sourceFile)), std::istreambuf_iterator< char >());

		//Set shader source
		const GLchar* shaderSource = shaderString.c_str();
		glShaderSource(shaderID, 1, (const GLchar**)&shaderSource, NULL);

		//Compile shader source
		glCompileShader(shaderID);

		//Check shader for errors
		GLint shaderCompiled = GL_FALSE;
		glGetShaderiv(shaderID, GL_COMPILE_STATUS, &shaderCompiled);
		if (shaderCompiled != GL_TRUE) {
			if (!gIsShaderError) {
				std::clog << "Unable to compile shader " << shaderCompiled << "!\n\nSource:\n" << shaderSource << std::endl;
				printShaderLog(shaderID);
			}
			glDeleteShader(shaderID);
			shaderID = 0;
			gIsShaderError = true;
		} else {
			gIsShaderError = false;
		}
	} else {
		printf("Unable to open file %s\n", filePath.c_str());
	}
}
