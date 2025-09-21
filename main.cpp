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
const uint NUMCELLS_X = 5, NUMCELLS_Y = 5;
int SCR_WIDTH = 600, SCR_HEIGHT = 600;
int CELL_WIDTH = SCR_WIDTH / NUMCELLS_X, CELL_HEIGHT = SCR_HEIGHT / NUMCELLS_Y;


// FUNCTIONS
// ---------
void initCellsComputeShader();
void initGridShader();
void initLiveCellsShader();
void bindNewLiveCellVertices();
void renderGrid();
void renderLiveCells();

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
class Grid {
public:
    VFShaderProgram* Shader;
    GLuint VBO, VAO;
} grid;
class LiveCells {
public:
    VFShaderProgram* Shader;
    GLuint VBO, VAO, EBO;
} liveCells;

ComputeShaderProgram* computeShader;
std::vector<uint32> newCells(NUMCELLS_X * NUMCELLS_Y);   // Current cell states
int newLiveCellsCount = 0;

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

    initCellsComputeShader();
    initGridShader();
    initLiveCellsShader();

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

        renderGrid();
        // bindNewLiveCellVertices();
        renderLiveCells();

        // GLFW: POLL & CALL IOEVENTS + SWAP BUFFERS
        // -----------------------------------------
        glfwSwapBuffers(window);    // For reader - search 'double buffer'
        glfwPollEvents();
    }

    // GLFW: TERMINATE GLFW, CLEARING ALL PREVIOUSLY ALLOCATED GLFW RESOURCES
    glfwTerminate();
    return 0;
}

void renderGrid()
{
    grid.Shader->use();
    int numGridLines = (NUMCELLS_X+1)*2 + (NUMCELLS_Y+1)*2;
    glBindVertexArray(grid.VAO);
    glDrawArrays(GL_LINES, 0, numGridLines * 2);
    glBindVertexArray(0);
}

void renderLiveCells()
{
    liveCells.Shader->use();
    glBindVertexArray(liveCells.VAO);
    glDrawElements(GL_TRIANGLES, newLiveCellsCount * 6, GL_UNSIGNED_INT, 0);
}

// This shader computes the core logic of the cellular automata (not used for drawing)
void initCellsComputeShader()
{
    computeShader = new ComputeShaderProgram("src//shaders//computeShader.comp");

    computeShader->setInt_w_Name("numCellsX", NUMCELLS_X);
    computeShader->setInt_w_Name("numCellsY", NUMCELLS_Y);

    // Create & bind 'cell state' buffers
    // ----------------------------------
    // Init cell data
    std::vector<uint32> prevCells(NUMCELLS_X, NUMCELLS_Y);  // Prev cell states
    // temp
    for (int i = 0; i < NUMCELLS_X; i++)
        for (int j = 0; j < NUMCELLS_Y; j++) {
            prevCells[i, j] = 0;
            newCells[i, j] = 0;
        }

    
    // Bind buffers to SSBOS
    GLuint bufferSize = (int)prevCells.size() * sizeof(prevCells[0]);

    glGenBuffers(1, &prevCellsBuf);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, prevCellsBuf);   // Binds the buffer to an SSBO at binding index = 0 in the compute shader
    glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, prevCells.data(), GL_DYNAMIC_DRAW);  // Send buffer data to SSBO target

    glGenBuffers(1, &newCellsBuf);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, newCellsBuf);
    glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, newCells.data(), GL_DYNAMIC_DRAW);
}

// This shader draws an unchanging base grid with lines
void initGridShader()
{
    grid.Shader = new VFShaderProgram("src//shaders//vert.vert", "src//shaders//frag.frag");

    // Create & bind buffers
    // ---------------------
    glGenVertexArrays(1, &grid.VAO); // // Generates the object and stores the resulting id in passed in integer
    glBindVertexArray(grid.VAO); // Binds 'VAO' as current active vertex array object

    // INIT, BIND & SET VBO
    std::vector<GLfloat> gridVertices(2*((NUMCELLS_X+1)*2 + (NUMCELLS_Y+1)*2));  // Every 4 consec. ints corresponds to 2 boundary vertices connected by a line
    
    int i = 0;
    // Vertical lines
    GLfloat xIncr = 2.0f / static_cast<GLfloat>(NUMCELLS_X);
    for (GLfloat xPos = -1.0f; xPos <= 1.0f; xPos += xIncr) {
        gridVertices[i++]   = xPos;    gridVertices[i++] = -1.0f;
        gridVertices[i++]   = xPos;    gridVertices[i++] = 1.0f;
    }
    // // Horizontal lines
    GLfloat yIncr = 2.0f / static_cast<GLfloat>(NUMCELLS_Y);
    for (GLfloat yPos = -1.0f; yPos <= 1.0f; yPos += yIncr) {
        gridVertices[i++]   = -1.0f;       gridVertices[i++] = yPos;
        gridVertices[i++]   = 1.0f;    gridVertices[i++] = yPos;
    }

    glGenBuffers(1, &grid.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, grid.VBO);  // Binds newly created object to the correct buffer type, which when updated/configured will update 'VBO' (as seen below)
    glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(GLfloat), gridVertices.data(), GL_STATIC_DRAW);  // Copies vertex data into the buffer

    // CONFIG VAO
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);   // Describes to OpenGL how to interpet vertex POSITION data
    glEnableVertexAttribArray(0);   /// Enables vertex attribute at location = 0, since they are disabled by default

    // UNBIND VBO FROM CURRENT ACTIVE BUFFER
    glBindBuffer(GL_ARRAY_BUFFER, 0); // This is allowed, the call to glVertexAttribPointer registered 'VBO' as the vertex attribute's bound VBO, so can safely unbind after
}

// This shader draws coloured squares upon only the live cells
void initLiveCellsShader()
{
    liveCells.Shader = new VFShaderProgram("src//shaders//vert.vert", "src//shaders//frag.frag");

    // Create & bind buffers
    // ---------------------
    glGenVertexArrays(1, &liveCells.VAO); // // Generates the object and stores the resulting id in passed in integer

    // INIT, BIND & SET VBO, EBO
    bindNewLiveCellVertices();

    // Config VAO
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);   // Describes to OpenGL how to interpet vertex POSITION data
    glEnableVertexAttribArray(0);   /// Enables vertex attribute at location = 0, since they are disabled by default

    // UNBIND VBO FROM CURRENT ACTIVE BUFFER
    glBindBuffer(GL_ARRAY_BUFFER, 0); // This is allowed, the call to glVertexAttribPointer registered 'VBO' as the vertex attribute's bound VBO, so can safely unbind after
}

void bindNewLiveCellVertices()
{
    std::vector<GLfloat> liveCellVertices; // 4 points (of 2 values) = 8 values for each live cell

    GLfloat scaleX = 2.0f / static_cast<GLfloat>(NUMCELLS_X);
    GLfloat scaleY = 2.0f / static_cast<GLfloat>(NUMCELLS_Y);

    newLiveCellsCount = 0;
    for (GLint y = 0; y < NUMCELLS_Y; ++y) {
        for (GLint x = 0; x < NUMCELLS_X; ++x) {
            if (newCells[y*NUMCELLS_X + x] == 0) continue;  // Dead cell

            newLiveCellsCount++;

            GLfloat xF = static_cast<GLfloat>(x);   GLfloat yF = static_cast<GLfloat>(y);

            liveCellVertices.push_back(xF * scaleX - 1.0f);     // BOTTOM-LEFT
            liveCellVertices.push_back(yF * scaleY - 1.0f);

            liveCellVertices.push_back((xF+1.0f) * scaleX -1.0f); // BOTTOM-RIGHT
            liveCellVertices.push_back(yF * scaleY - 1.0f);

            liveCellVertices.push_back((xF+1.0f) * scaleX -1.0f); // TOP-RIGHT
            liveCellVertices.push_back((yF+1.0f) * scaleY -1.0f);

            liveCellVertices.push_back(xF * scaleX - 1.0f);     // TOP-LEFT
            liveCellVertices.push_back((yF+1) * scaleY - 1.0f);
        }
    }

    std::vector<GLint> liveCellIndeces; // 2 triangles of 3 indices = 6 ints for each live cell

    for (GLint vertex = 0; vertex < liveCellVertices.size() / 2; vertex += 4) {
        // First triangle
        liveCellIndeces.push_back(vertex);
        liveCellIndeces.push_back(vertex+1);
        liveCellIndeces.push_back(vertex+2);

        // Second triangle
        liveCellIndeces.push_back(vertex+2);
        liveCellIndeces.push_back(vertex+3);
        liveCellIndeces.push_back(vertex);
    }

    glBindVertexArray(liveCells.VAO); // Binds 'VAO' as current active vertex array object

    // VBO (vertex data)
    glGenBuffers(1, &liveCells.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, liveCells.VBO);  // Binds newly created object to the correct buffer type, which when updated/configured will update 'VBO' (as seen below)
    glBufferData(GL_ARRAY_BUFFER, liveCellVertices.size() * sizeof(GLfloat), liveCellVertices.data(), GL_DYNAMIC_DRAW);  // Copies vertex data into the buffer
    
    // EBO (index data)
    glGenBuffers(1, &liveCells.EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, liveCells.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, liveCellIndeces.size() * sizeof(GLint), liveCellIndeces.data(), GL_STATIC_DRAW);
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