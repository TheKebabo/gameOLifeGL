#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include "shader_program.h"

ShaderProgram::~ShaderProgram()
{
    glDeleteProgram(ID);
}

void ShaderProgram::use()
{
    glUseProgram(ID);
}

GLint ShaderProgram::getUniformLocation(GLchar* name)
{
    GLuint loc = (GLuint)glGetUniformLocation(ID, name);

    if ((GLuint)loc == 0xffffffff) {
        std::cout << "Warning! Unable to get the location of uniform: \"" << name << "\"" << std::endl;
    }

    return loc;
}

void ShaderProgram::setBool_w_Name(const std::string &name, bool value) const
{         
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); 
}
void ShaderProgram::setBool_w_Loc(GLint location, bool value) const
{         
    glUniform1i(location, (int)value); 
}

void ShaderProgram::setInt_w_Name(const std::string &name, int value) const
{ 
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
}
void ShaderProgram::setInt_w_Loc(GLint location, int value) const
{ 
    glUniform1i(location, value); 
}

void ShaderProgram::setFloat_w_Name(const std::string &name, float value) const
{ 
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value); 
}
void ShaderProgram::setFloat_w_Loc(GLint location, float value) const
{ 
    glUniform1f(location, value); 
}
void ShaderProgram::set4Floats_w_Name(const std::string &name, float value0, float value1, float value2, float value3) const
{
    glUniform4f(glGetUniformLocation(ID, name.c_str()), value0, value1, value2, value3);
}
void ShaderProgram::set4Floats_w_Loc(GLint location, float value0, float value1, float value2, float value3) const
{
    glUniform4f(location, value0, value1, value2, value3);
}

void ShaderProgram::setMat4_w_Name(const std::string &name, GLboolean transpose, const GLfloat* value) const
{
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, transpose, value);
}
void ShaderProgram::setMat4_w_Loc(GLint location, GLboolean transpose, const GLfloat* value) const
{
    glUniformMatrix4fv(location, 1, transpose, value);
}

#include <vector>
#include <iomanip>

// UTILITIES
unsigned ShaderProgram::readAndCompileShaderFile(const char* shaderPath, unsigned& shaderID, std::string shaderType)
{
    std::ifstream file(shaderPath, std::ios::binary); // Open as binary to try to stop formatting errors
    
    if (!file.is_open()) {
        // This printout will tell you EXACTLY where the program is looking
        std::cerr << "ERROR: Shader file NOT FOUND!" << std::endl;
        std::cerr << "Expected Path: " << shaderPath << std::endl;
        std::cerr << "Current Working Dir: " << std::filesystem::current_path() << std::endl;
        
        // Block the program so it doesn't spam OpenGL with empty strings
        std::cout << "Press Enter to exit..." << std::endl;
        std::cin.get();
        exit(1);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    // THE FIX: If the first character isn't '#', check for a BOM
    // UTF-8 BOM is usually 3 bytes: 239, 187, 191
    if (!content.empty() && (unsigned char)content[0] != '#') {
        size_t firstHash = content.find('#');
        if (firstHash != std::string::npos) {
            // Delete everything before the first '#'
            content.erase(0, firstHash);
        }
    }

    const char* shaderCode = content.c_str();
    glShaderSource(shaderID, 1, &shaderCode, NULL);
    glCompileShader(shaderID);

    return shaderID;

//     std::string shaderCode;
//     std::ifstream shaderFile;
//     // ensure ifstream object can throw exceptions:
//     shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
//     try 
//     {
//         // open file
//         shaderFile.open(shaderPath);
        
//         if (!shaderFile.is_open()) {
//         // This will stop the program and tell you EXACTLY what's wrong
//             std::cerr << "FATAL ERROR: Could not open shader file at: " << shaderPath << std::endl;
//             std::cerr << "Current working directory: " << std::filesystem::current_path() << std::endl;
//             exit(EXIT_FAILURE); 
//         }


//         std::stringstream shaderStream;
//         shaderStream << shaderFile.rdbuf(); // read file's buffer contents into stream
        
//         std::cout << "--- SHADER SOURCE START ---" << std::endl;
//         std::cout << shaderFile.rdbuf() << std::endl;
//         std::cout << "--- SHADER SOURCE END ---" << std::endl;
//         system("pause");

//         shaderFile.close();    // close file handler

//         shaderCode = shaderStream.str();
//     }
//     catch(const std::ifstream::failure& e)
//     {
//         std::cout << "ERROR::SHADER::" << shaderType << "::FILE_NOT_SUCCESFULLY_READ" << std::endl;
//     }

//     // const char* shaderCodeCStr = shaderCode.c_str();
//     const char* shaderCodeCStr = R"(#version 430 core

//     layout (local_size_x = 1, local_size_y = 1) in;

//     // uniforms
//     uniform int numCellsX;
//     uniform int numCellsY;

//     // I/Os
//     layout (std430, binding = 0) buffer Prev {   // An SSBO
//         uint PrevCellStates[];
//     };
//     layout (std430, binding = 1) buffer New {   // An SSBO
//         uint NewCellStates[];
//     };


//     void main() {
//         uint x = gl_GlobalInvocationID.x;   // Current work group x position
//         uint y = gl_GlobalInvocationID.y;

//         uint curState = PrevCellStates[y*numCellsX + x];

//         // Compute sum of neighbour states
//         int neighbourStatesSum = 0;
//         for (int j = -1; j <= 1; j++) {
//             for (int i = -1; i <= 1; i++) {
//                 if (x+i >=0 && x+i < numCellsX && y+j>=0 && y+j < numCellsY) { // Skip boundaries
//                     uint index = (int(y)+j)*int(numCellsX) + (int(x)+i);
//                     neighbourStatesSum += int(PrevCellStates[index]);
//                 }
//             }
//         }

//         neighbourStatesSum -= int(curState);
        
//         if (neighbourStatesSum == 3 || neighbourStatesSum + curState == 3)
//             NewCellStates[y*numCellsX + x] = 1;
//         else
//             NewCellStates[y*numCellsX + x] = 0;
// })";

//     glShaderSource(shaderID, 1, &shaderCodeCStr, NULL);
//     glCompileShader(shaderID);

//     return shaderID;
}

void ShaderProgram::checkCompileErrors(unsigned& shaderID, std::string shaderType)
{
    int success;
    char infoLog[512];
    
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(shaderID, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::" << shaderType << "::COMPILATION_FAILED\n" << infoLog << std::endl;

        // STOP EVERYTHING SO YOU CAN READ
        std::cout << "\nPress Enter to close..." << std::endl;
        std::cin.get(); 
        exit(1);
    };
}

void ShaderProgram::checkLinkErrors()
{
    int success;
    char infoLog[512];

    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(ID, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
}