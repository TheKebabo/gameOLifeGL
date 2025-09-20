#ifndef SHADER_PROGRAM_H
#define SHADER_PROGRAM_H

#include <glad/glad.h> // include glad to get all the required OpenGL headers
#include <string>
#include <sstream>

#define INVALID_UNIFORM_LOC 0xffffffff;

class ShaderProgram // ABSTRACT CLASS
{
public:
    unsigned ID;

    ShaderProgram() {}
    // Destructor
    ~ShaderProgram();
    // Activate the shader program
    void use();
    // Query uniform location
    GLint getUniformLocation(GLchar* name);
    // Utility uniform functions
    void setBool_w_Name(const std::string &name, bool value) const;  
    void setBool_w_Loc(GLint location, bool value) const;
    void setInt_w_Name(const std::string &name, int value) const;   
    void setInt_w_Loc(GLint location, int value) const;
    void setFloat_w_Name(const std::string &name, float value) const;
    void setFloat_w_Loc(GLint location, float value) const;
    void set4Floats_w_Name(const std::string &name, float value0, float value1, float value2, float value3) const;   
    void set4Floats_w_Loc(GLint location, float value0, float value1, float value2, float value3) const;   
    void setMat4_w_Name(const std::string &name, GLboolean transpose, const GLfloat* value) const;
    void setMat4_w_Loc(GLint location, GLboolean transpose, const GLfloat* value) const;
protected:
    unsigned readAndCompileShaderFile(const char* shaderPath, unsigned& shaderID, std::string shaderType);
    void checkCompileErrors(unsigned& shaderID, std::string shaderType);
    void checkLinkErrors();
};

#endif