#ifndef VF_SHADER_PROGRAM_H
#define VF_SHADER_PROGRAM_H

#include "shader_program.h"

// Vertex / fragment shader program
class VFShaderProgram : public ShaderProgram
{
public: 
    VFShaderProgram(const char* vertexPath, const char* fragmentPath);
};
  
#endif