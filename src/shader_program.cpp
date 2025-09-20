#include <fstream>
#include <sstream>
#include <iostream>
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

// UTILITIES
unsigned ShaderProgram::readAndCompileShaderFile(const char* shaderPath, unsigned& shaderID, std::string shaderType)
{
    std::string shaderCode;
    std::ifstream shaderFile;
    // ensure ifstream object can throw exceptions:
    shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try 
    {
        // open file
        shaderFile.open(shaderPath);
        std::stringstream shaderStream;
        shaderStream << shaderFile.rdbuf(); // read file's buffer contents into stream
        shaderFile.close();    // close file handler

        shaderCode = shaderStream.str();
    }
    catch(const std::ifstream::failure& e)
    {
        std::cout << "ERROR::SHADER::" << shaderType << "::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }

    const char* shaderCodeCStr = shaderCode.c_str();
    glShaderSource(shaderID, 1, &shaderCodeCStr, NULL);
    glCompileShader(shaderID);

    return shaderID;
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