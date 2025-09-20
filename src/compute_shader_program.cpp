#include "compute_shader_program.h"

ComputeShaderProgram::ComputeShaderProgram(const char* computePath)
{
    // 1. retrieve the computer shader source code from file path and compile
    unsigned computeShader = glCreateShader(GL_COMPUTE_SHADER);
    readAndCompileShaderFile(computePath, computeShader, "COMPUTE");
    checkCompileErrors(computeShader, "COMPUTE");   // print compile errors if any

    // 2. create shader Program
    ID = glCreateProgram();
    glAttachShader(ID, computeShader);
    glLinkProgram(ID);
    checkLinkErrors();

     // 3. delete the shader as it is linked into our program now and no longer necessary
    glDeleteShader(computeShader);
}