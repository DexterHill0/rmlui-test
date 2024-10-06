#ifndef SHADER_H
#define SHADER_H

#include <optional>
#include <unordered_map>

class Shader
{
public:
    // the program ID
    unsigned int programId;
  
    // constructor reads and builds the shader
    Shader();
    ~Shader();

    static std::optional<Shader*> compile(const char* vertex, const char* fragment);

    void use();

    template <size_t N>
    void cacheUniformLocations(std::array<const char*, N> uniformNames) {
        for(auto i = 0; i < uniformNames.size(); i++) {
            auto name = uniformNames[i];
            uniforms[name] = glGetUniformLocation(programId, name);
        }
    }
        
    void setFloat4(const char*, float a, float b, float c, float d);

private:
    GLint getUniformLocation(const char*);

    std::unordered_map<const char*, GLint> uniforms = {};
};
  
#endif