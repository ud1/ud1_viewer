#ifndef SHADER_H
#define SHADER_H

#include <map>
#include <string>
#include <GL/glew.h>

struct Shader
{
    Shader(const std::string &name) : shaderName(name) {}
    GLuint program = 0;
    std::map<std::string, GLint> uniforms;
    std::string shaderName;

    bool buildShaderProgram(const char *vsPath, const char *fsPath);
    bool buildShaderProgram(const char *vsPath, const char *gsPath, const char *fsPath);
};

#endif // SHADER_H
