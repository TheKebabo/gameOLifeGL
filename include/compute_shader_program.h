#ifndef COMPUTE_SHADER_PROGRAM_H
#define COMPUTE_SHADER_PROGRAM_H

#include "shader_program.h"

class ComputeShaderProgram : public ShaderProgram
{
public:
    ComputeShaderProgram(const char* computePath);
};

#endif