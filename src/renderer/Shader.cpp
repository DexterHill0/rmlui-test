#include "./Shader.h"

#include <optional>

#include "Geode/loader/Log.hpp"

Shader::Shader() {}

Shader::~Shader() {
    remove();
}

std::optional<Shader*>
Shader::compile(const char* vertex, const char* fragment) {
    Shader* shader = new Shader;

    int success;
    char infoLog[512];

    // vertex
    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertex, NULL);
    glCompileShader(vertexShader);

    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        geode::log::error("[SHADER_VERTEX_COMPILATION_FAILED] {}", infoLog);

        return std::nullopt;
    }

    // frag
    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragment, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        geode::log::error("[SHADER_FRAGMENT_COMPILATION_FAILED] {}", infoLog);

        return std::nullopt;
    }

    // program
    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();

    shader->programId = shaderProgram;

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        geode::log::error("[SHADER_LINK_FAILED] {}", infoLog);

        return std::nullopt;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return std::make_optional(shader);
}

void Shader::use() {
    glUseProgram(programId);
}

void Shader::remove() {
    glDeleteProgram(programId);
}

GLint Shader::getUniformLocation(const char* name) {
    // assume uniforms have been cached already
    return uniforms[name];
}

void Shader::setFloat4(const char* name, float a, float b, float c, float d) {
    glUniform4f(getUniformLocation(name), a, b, c, d);
}

void Shader::setFloatMat4(const char* name, float* mat, int count) {
    glUniformMatrix4fv(getUniformLocation(name), count, GL_FALSE, mat);
}

void Shader::setFloatVec2(const char* name, float* mat, int count) {
    glUniform2fv(getUniformLocation(name), count, mat);
}