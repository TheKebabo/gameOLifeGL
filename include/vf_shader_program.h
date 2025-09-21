#ifndef VF_SHADER_PROGRAM_H
#define VF_SHADER_PROGRAM_H

#include "shader_program.h"

// Vertex / fragment shader program
class VFShaderProgram : public ShaderProgram
{
public: 
    VFShaderProgram(const char* vertexPath, const char* fragmentPath);
protected:
    const char* _vertexPath;
    const char* _fragmentPath;
    void checkLinkErrors();
};
  
#endif