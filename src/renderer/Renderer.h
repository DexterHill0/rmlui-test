#ifndef RMLUI_BACKENDS_RENDERER_GD
#define RMLUI_BACKENDS_RENDERER_GD

#include <RmlUi/Core/RenderInterface.h>

#include <unordered_map>

#include "./Shader.h"

class RenderInterface_GD: public Rml::RenderInterface {
  public:
    RenderInterface_GD();
    ~RenderInterface_GD();

    bool Initialise(int viewportWidth, int viewportHeight);

    // The viewport should be updated whenever the window size changes.
    void SetViewport(int viewportWidth, int viewportHeight);

    // Sets up OpenGL states for taking rendering commands from RmlUi.
    void BeginFrame();
    void EndFrame();

    // -- Inherited from Rml::RenderInterface --

    Rml::CompiledGeometryHandle CompileGeometry(
        Rml::Span<const Rml::Vertex> vertices,
        Rml::Span<const int> indices
    ) override;
    void ReleaseGeometry(Rml::CompiledGeometryHandle geometry) override;
    void RenderGeometry(
        Rml::CompiledGeometryHandle handle,
        Rml::Vector2f translation,
        Rml::TextureHandle texture
    ) override;

    void EnableScissorRegion(bool enable) override;
    void SetScissorRegion(Rml::Rectanglei region) override;

    Rml::TextureHandle LoadTexture(
        Rml::Vector2i& texture_dimensions,
        const Rml::String& source
    ) override;
    Rml::TextureHandle GenerateTexture(
        Rml::Span<const Rml::byte> source,
        Rml::Vector2i source_dimensions
    ) override;
    void ReleaseTexture(Rml::TextureHandle texture_handle) override;

    void EnableClipMask(bool enable) override;
    void RenderToClipMask(
        Rml::ClipMaskOperation operation,
        Rml::CompiledGeometryHandle geometry,
        Rml::Vector2f translation
    ) override;

    void SetTransform(const Rml::Matrix4f* transform) override;

    // Can be passed to RenderGeometry() to enable texture rendering without changing the bound texture.
    static const Rml::TextureHandle TextureEnableWithoutBinding =
        Rml::TextureHandle(-1);

  private:
    Shader* posTexColShader;

    struct CompiledGeometryData {
        GLuint vao;
        GLuint vbo;
        GLuint ibo;
        GLsizei drawCount;
    };

    struct GdState {
        GLint shaderProgram = 0;
        GLint vao = 0;
        GLint vbo = 0;
        GLint ebo = 0;
        GLint texture = 0;
        GLint fbo = 0;
    };

    GdState gdState = {};

    void saveGdState();
    void restoreGdState();

    int viewportWidth = 0;
    int viewportHeight = 0;

    // GLuint rmluiVao;
    // GLuint rmluiVbo;
    // GLuint rmluiEbo;
    GLuint rmluiFbo;

    Rml::TextureHandle rmluiTexture;

    std::unordered_map<GLuint, std::string> textureIdMap;
};

#endif
