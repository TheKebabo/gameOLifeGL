#include <iostream>
#include <cmath>
#include <vector>

#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "vf_shader_program.h"
#include "compute_shader_program.h"

using namespace glm;


// SETTINGS
// --------
int SCR_WIDTH = 800, SCR_HEIGHT = 800; 
const uint NUMCELLS_X = 10, NUMCELLS_Y = 10;


// FUNCTIONS
// ---------
// Utilities
GLFWwindow* configGLFW();
void outputGLLimits();

// Callbacks
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouseMoveCallback(GLFWwindow* window, double mouseX, double mouseY);
void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);


// GLOBALS
// -------
GLuint prevCellsBuf, newCellsBuf;
GLuint VAO; // Vertex attr. obj
VFShaderProgram* mainShader;
ComputeShaderProgram* computeShader;

int main()
{
    // GLFW: INIT & CONFIG
    // -------------------
    GLFWwindow* window = configGLFW();
    if (window == NULL) return -1;

    // INIT GLAD, WHICH MANAGES FUNCTION POINTERS FOR OPENGL
    // -----------------------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    
    mainShader = new VFShaderProgram("src//shaders//vertexShader.vert", "src//shaders//fragmentShader.frag");
    computeShader = new ComputeShaderProgram("src//shaders//computeShader.comp");

    computeShader->setInt_w_Name("numCellsX", NUMCELLS_X);
    computeShader->setInt_w_Name("numCellsY", NUMCELLS_Y);

    // RENDER LOOP
    // -----------
    while(!glfwWindowShouldClose(window))
    {
        // INPUT
        // -----
        processInput(window);

        // RENDER
        // ------        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.85f, 0.85f, 0.85f, 1.0f);

        renderCells();

        // GLFW: POLL & CALL IOEVENTS + SWAP BUFFERS
        // -----------------------------------------
        glfwSwapBuffers(window);    // For reader - search 'double buffer'
        glfwPollEvents();
    }

    // GLFW: TERMINATE GLFW, CLEARING ALL PREVIOUSLY ALLOCATED GLFW RESOURCES
    glfwTerminate();
    return 0;
}

void renderCells()
{
    mainShader->use();
    glBindVertexArray(VAO);
    // glDrawArrays(GL_POINTS, 0, );
}

void initSSBOS()
{
    // Init data
    // ---------
    std::vector<uint32> prevCells(NUMCELLS_X, NUMCELLS_Y);  // Prev cell states
    std::vector<uint32> newCells(NUMCELLS_X, NUMCELLS_Y);   // Current cell states
    // temp
    for (int i = 0; i < NUMCELLS_X; i++)
        for (int j = 0; j < NUMCELLS_Y; j++) {
            prevCells[i, j] = 0;
            newCells[i, j] = 0;
        }

    // Create buffers and bind to GL objects
    // -------------------------------------
    glGenBuffers(1, &prevCellsBuf);
    glGenBuffers(1, &newCellsBuf);
    
    GLuint bufferSize = (int)prevCells.size() * sizeof(prevCells[0]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, prevCellsBuf);   // Binds the buffer to an SSBO at binding index = 0 in the compute shader
    glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, prevCells.data(), GL_DYNAMIC_DRAW);  // Send buffer data to SSBO target

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, newCellsBuf);
    glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, newCells.data(), GL_DYNAMIC_DRAW);

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, newCellsBuf);   // Same buffer is bound as an SSBO and as 'GL_ARRAY_BUFFER' in normal vertex shader pipeline
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);   // Unbind VAO
}

void executeCompShader()
{
    computeShader->use();
    glDispatchCompute(NUMCELLS_X*NUMCELLS_Y, NUMCELLS_X, NUMCELLS_Y);
}


// GLFW: INIT & SETUP WINDOW OBJECT
// --------------------------------
GLFWwindow* configGLFW()
{
    if (!glfwInit())
    {
        std::cout << "Failed to init GLFW" << std::endl;
        return NULL;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  
    // CREATE WINDOW OBJ
    // -----------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return NULL;
    }
    glfwMakeContextCurrent(window); // SETS CREATED WINDOW OBJ AS THE MAIN CONTEXT ON THE CURRENT THREAD
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);  // FUNCTION IS CALLED ON WINDOW RESIZE
    glfwSwapInterval(0);    // Gets bigger refresh rate

    // Set glfw mouse configs
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouseMoveCallback);
    glfwSetScrollCallback(window, mouseScrollCallback);
    return window;
}

// PROCESSES INPUT BY QUERING GLFW ABOUT CURRENT FRAME
// ---------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)   // GLFW_RELEASE RETURNED FROM GetKey IF NOT PRESSED 
        glfwSetWindowShouldClose(window, true);
}

void outputGLLimits()
{
    // query limitations
	int max_compute_work_group_count[3];
	int max_compute_work_group_size[3];
	int max_compute_work_group_invocations;

	for (int idx = 0; idx < 3; idx++) {
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, idx, &max_compute_work_group_count[idx]);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, idx, &max_compute_work_group_size[idx]);
	}	
	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &max_compute_work_group_invocations);

	std::cout << "OpenGL Limitations: " << std::endl;
	std::cout << "maximum number of work groups in X dimension " << max_compute_work_group_count[0] << std::endl;
	std::cout << "maximum number of work groups in Y dimension " << max_compute_work_group_count[1] << std::endl;
	std::cout << "maximum number of work groups in Z dimension " << max_compute_work_group_count[2] << std::endl;

	std::cout << "maximum size of a work group in X dimension " << max_compute_work_group_size[0] << std::endl;
	std::cout << "maximum size of a work group in Y dimension " << max_compute_work_group_size[1] << std::endl;
	std::cout << "maximum size of a work group in Z dimension " << max_compute_work_group_size[2] << std::endl;

	std::cout << "Number of invocations in a single local work group that may be dispatched to a compute shader " << max_compute_work_group_invocations << std::endl;
}


//  SETS SIZE OF RENDERING SPACE WITH RESPECT TO WINDOW OBJ
// --------------------------------------------------------
void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouseMoveCallback(GLFWwindow* window, double mouseX, double mouseY)
{
}

void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
}