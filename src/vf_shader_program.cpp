#include <iostream>
#include "vf_shader_program.h"

VFShaderProgram::VFShaderProgram(const char* vertexPath, const char* fragmentPath)
{
    _vertexPath = vertexPath;   _fragmentPath = fragmentPath;
    unsigned vertex = glCreateShader(GL_VERTEX_SHADER);
    unsigned fragment = glCreateShader(GL_FRAGMENT_SHADER);
    readAndCompileShaderFile(_vertexPath, vertex, "VERTEX");
    checkCompileErrors(vertex, "VERTEX");   // print compile errors if any
    readAndCompileShaderFile(_fragmentPath, fragment, "FRAGMENT");
    checkCompileErrors(fragment, "FRAGMENT");   // print compile errors if any
 
    // 2. create shader program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    checkLinkErrors();   // print link errors if any
    
    // 3. delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void VFShaderProgram::checkLinkErrors()
{
    int success;
    char infoLog[512];

    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(ID, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << _vertexPath << ", " << _fragmentPath << "\n" << infoLog << std::endl;
    }
}