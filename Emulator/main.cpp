#include <iostream>
#include <sstream>
#include <Windows.h>
#include <map>
#include <sys/stat.h>


#include "Chip8.h"
#include "SuperChip.h"

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

#define SHADER_DEBUGGING
#define SUPERCHIP

#define CHECK_KEY(key) if (glfwGetKey(window, key) == GLFW_PRESS) emulator.m_Key |= 1 << gKeyMap[key]

#ifdef SUPERCHIP
typedef SuperChip Emulator;
#else
typedef Chip8 Emulator;
#endif

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


std::map<int, U16> gKeyMap;

//Create emulator;
Emulator emulator;


int main(int argc, char* argv[]) {
	//Set keys
	gKeyMap.insert(std::make_pair(GLFW_KEY_1, 0x1));
	gKeyMap.insert(std::make_pair(GLFW_KEY_2, 0x2));
	gKeyMap.insert(std::make_pair(GLFW_KEY_3, 0x3));
	gKeyMap.insert(std::make_pair(GLFW_KEY_4, 0xC));
	gKeyMap.insert(std::make_pair(GLFW_KEY_Q, 0x4));
	gKeyMap.insert(std::make_pair(GLFW_KEY_W, 0x5));
	gKeyMap.insert(std::make_pair(GLFW_KEY_E, 0x6));
	gKeyMap.insert(std::make_pair(GLFW_KEY_R, 0xD));
	gKeyMap.insert(std::make_pair(GLFW_KEY_A, 0x7));
	gKeyMap.insert(std::make_pair(GLFW_KEY_S, 0x8));
	gKeyMap.insert(std::make_pair(GLFW_KEY_D, 0x9));
	gKeyMap.insert(std::make_pair(GLFW_KEY_F, 0xE));
	gKeyMap.insert(std::make_pair(GLFW_KEY_Z, 0xA));
	gKeyMap.insert(std::make_pair(GLFW_KEY_X, 0x0));
	gKeyMap.insert(std::make_pair(GLFW_KEY_C, 0xB));
	gKeyMap.insert(std::make_pair(GLFW_KEY_V, 0xF));


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

	glfwSwapInterval(1);

#ifdef SUPERCHIP
	emulator.SetExitCallback([&]() {glfwSetWindowShouldClose(window, GL_TRUE); });
#endif

	if (argc > 1) {
		emulator.LoadRom(argv[1]);
	}
	srand(GetTickCount());

	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetDropCallback(window, drop_callback);

	//Set texture parameters
	GLuint texture_id;
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);  //Always set the base and max mipmap levels of a texture.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);


#ifdef SHADER_DEBUGGING
	struct stat buf;
	stat("Resources/fragmentShader.glsl", &buf);
	int lastShaderModification = (int)buf.st_mtime;
#endif

	// Game loop
	while (!glfwWindowShouldClose(window)) {

		// Check if any events have been activated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();

		HandleInput(window);
		emulator.DecreaseTimers();
		for (int i = 0; i < 5; i++) {
			//loop emulator
			emulator.Loop();
		}
		emulator.m_Key = 0;

#ifdef SHADER_DEBUGGING
		stat("Resources/fragmentShader.glsl", &buf);
		if (lastShaderModification < (int)buf.st_mtime) {
			lastShaderModification = (int)buf.st_mtime;
			ReloadShaderFromFile("Resources/fragmentShader.glsl", fragmentShader);
			glLinkProgram(shaderProgram);
		}
#endif

#ifdef SUPERCHIP
		if (emulator.m_Extended)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 128, 64, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, (GLvoid*)emulator.m_Gfx);
		else
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 64, 32, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, (GLvoid*)emulator.m_Gfx);
#else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 64, 32, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, (GLvoid*)emulator.m_Texture);
#endif

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

#pragma region Input

void HandleInput(GLFWwindow* window) {
	CHECK_KEY(GLFW_KEY_1);
	CHECK_KEY(GLFW_KEY_2);
	CHECK_KEY(GLFW_KEY_3);
	CHECK_KEY(GLFW_KEY_4);
	CHECK_KEY(GLFW_KEY_Q);
	CHECK_KEY(GLFW_KEY_W);
	CHECK_KEY(GLFW_KEY_E);
	CHECK_KEY(GLFW_KEY_R);
	CHECK_KEY(GLFW_KEY_A);
	CHECK_KEY(GLFW_KEY_S);
	CHECK_KEY(GLFW_KEY_D);
	CHECK_KEY(GLFW_KEY_F);
	CHECK_KEY(GLFW_KEY_Z);
	CHECK_KEY(GLFW_KEY_X);
	CHECK_KEY(GLFW_KEY_C);
	CHECK_KEY(GLFW_KEY_V);
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	UNREFERENCED_PARAMETER(mode);
	UNREFERENCED_PARAMETER(scancode);
	UNREFERENCED_PARAMETER(window);

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

void drop_callback(GLFWwindow* window, int count, const char** paths) {
	UNREFERENCED_PARAMETER(window);
	for (int i = 0; i < count; i++) {
		std::cout << paths[i] << std::endl;
	}
	emulator.LoadRom(paths[0]);
}

#pragma endregion

#pragma region Shader Interpreter and Compiler

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

#pragma endregion