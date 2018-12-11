#include "shader.h"

#include <QFile>
#include <QTextStream>
#include <vector>
#include <QDebug>

static GLuint createShader(GLenum eShaderType, const char *strShaderFile, const std::string &shaderName)
{
    QFile f(strShaderFile);
    if (!f.open(QFile::ReadOnly | QFile::Text))
        return 1;

    QTextStream in(&f);
    QString text = in.readAll();
    QByteArray data = text.toUtf8();
    const char *dataStr = data.constData();

    GLuint shader = glCreateShader(eShaderType);
    glShaderSource(shader, 1, &dataStr, NULL);

    glCompileShader(shader);

    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    if (status == GL_FALSE)
    {
        GLint infoLogLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

        GLchar strInfoLog[4096];
        glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);

        char strShaderType[16];

        switch (eShaderType)
        {
        case GL_VERTEX_SHADER:
            sprintf(strShaderType, "vertex");
            break;

        case GL_GEOMETRY_SHADER:
            sprintf(strShaderType, "geometry");
            break;

        case GL_FRAGMENT_SHADER:
            sprintf(strShaderType, "fragment");
            break;
        }

        qDebug() << "Compile failure in " <<  strShaderType << " shader(" << shaderName.c_str() << "):\n" << strInfoLog;
        return -1;
    }
    else
    {
        qDebug() << "Shader compiled sucessfully! " << shaderName.c_str();
    }

    return shader;
}

bool Shader::buildShaderProgram(const char *vsPath, const char *fsPath)
{
    GLuint vertexShader;
    GLuint fragmentShader;

    vertexShader = createShader(GL_VERTEX_SHADER, vsPath, shaderName);
    fragmentShader = createShader(GL_FRAGMENT_SHADER, fsPath, shaderName);

    program = glCreateProgram();

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program); //linking!

    //error checking
    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);

    if (status == GL_FALSE)
    {
        GLint infoLogLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

        GLchar strInfoLog[4096];
        glGetProgramInfoLog(program, infoLogLength, NULL, strInfoLog);
        qDebug() << "Shader linker failure " << shaderName.c_str() << ": " << strInfoLog;
        return false;
    }
    else
    {
        qDebug() << "Shader linked sucessfully! " << shaderName.c_str();
    }

    /*glDetachShader(program, vertexShader);
    glDetachShader(program, fragmentShader);*/

    GLint nuni;
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &nuni);
    char name[256];

    for (GLint i = 0; i < nuni; ++i)
    {
        GLint size;
        GLenum type;

        glGetActiveUniform(program, i, sizeof(name), NULL, &size, &type, name);
        GLint location = glGetUniformLocation(program, name);
        uniforms[name] = location;
        qDebug() << "Shader " << shaderName.c_str() << ": " << name << " " << location;
    }

    return true;
}

bool Shader::buildShaderProgram(const char *vsPath, const char *gsPath, const char *fsPath)
{
    GLuint vertexShader;
    GLuint geometryShader;
    GLuint fragmentShader;

    vertexShader = createShader(GL_VERTEX_SHADER, vsPath, shaderName);
    geometryShader = createShader(GL_GEOMETRY_SHADER, gsPath, shaderName);
    fragmentShader = createShader(GL_FRAGMENT_SHADER, fsPath, shaderName);

    program = glCreateProgram();

    glAttachShader(program, vertexShader);
    glAttachShader(program, geometryShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program); //linking!

    //error checking
    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);

    if (status == GL_FALSE)
    {
        GLint infoLogLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

        GLchar strInfoLog[4096];
        glGetProgramInfoLog(program, infoLogLength, NULL, strInfoLog);

        qDebug() << "Shader linker failure " << shaderName.c_str() << ": " << strInfoLog;
        return false;
    }
    else
    {
        qDebug() << "Shader linked sucessfully! " << shaderName.c_str();
    }

    /*glDetachShader(program, vertexShader);
    glDetachShader(program, geometryShader);
    glDetachShader(program, fragmentShader);*/

    GLint nuni;
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &nuni);
    char name[256];

    for (GLint i = 0; i < nuni; ++i)
    {
        GLint size;
        GLenum type;

        glGetActiveUniform(program, i, sizeof(name), NULL, &size, &type, name);
        GLint location = glGetUniformLocation(program, name);
        uniforms[name] = location;
        qDebug() << "Shader "<< shaderName.c_str() << ": " << name << " " << location;
    }

    return true;
}
