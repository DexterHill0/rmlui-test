#include "Renderer.h"

#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/FileInterface.h>
#include <RmlUi/Core/Log.h>
#include <RmlUi/Core/Platform.h>
#include <cocos2d.h>
#include <string.h>

#include <Geode/Geode.hpp>

#include "./shaders/PositionColorTexture/Shader.h"
#include "Geode/cocos/kazmath/include/kazmath/mat4.h"
#include "Geode/cocos/platform/CCImage.h"
#include "Geode/cocos/platform/win32/CCEGLView.h"
#include "Geode/cocos/shaders/CCGLProgram.h"
#include "Geode/loader/Log.hpp"
#include "RmlUi/Core/Vertex.h"

#if defined RMLUI_PLATFORM_WIN32
    #include <gl/Gl.h>
    #include <gl/Glu.h>

    #include "RmlUi_Include_Windows.h"

#elif defined RMLUI_PLATFORM_MACOSX
    #include <AGL/agl.h>
    #include <OpenGL/gl.h>
    #include <OpenGL/glext.h>
    #include <OpenGL/glu.h>
#elif defined RMLUI_PLATFORM_UNIX
    #include <GL/gl.h>
    #include <GL/glext.h>
    #include <GL/glu.h>
    #include <GL/glx.h>

    #include "RmlUi_Include_Xlib.h"

#endif

#define GL_CLAMP_TO_EDGE 0x812F

constexpr int VERTEX_SIZE = sizeof(Rml::Vertex);

void RenderInterface_GD::saveGdState() {
    glGetIntegerv(GL_CURRENT_PROGRAM, &gdState.shaderProgram);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &gdState.vao);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &gdState.vbo);
    glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &gdState.ebo);
    glGetIntegerv(GL_TEXTURE_2D, &gdState.texture);
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &gdState.fbo);
}

void RenderInterface_GD::restoreGdState() {
    glUseProgram(gdState.shaderProgram);
    glBindVertexArray(gdState.vao);
    glBindBuffer(GL_ARRAY_BUFFER, gdState.vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gdState.ebo);
    glBindTexture(GL_TEXTURE_2D, gdState.texture);
    glBindFramebuffer(GL_FRAMEBUFFER, gdState.fbo);

    glViewport(0, 0, viewportWidth, viewportHeight);
}

RenderInterface_GD::~RenderInterface_GD() {
    // TODO: what else should be cleared? textures, framebuffer..?
    // glDeleteVertexArrays(1, &rmluiVao);
    // glDeleteBuffers(1, &rmluiVbo);
    // glDeleteBuffers(1, &rmluiEbo);
    posTexColShader->remove();
}

RenderInterface_GD::RenderInterface_GD() {}

bool RenderInterface_GD::Initialise(int viewportWidth, int viewportHeight) {
    SetViewport(viewportWidth, viewportHeight);

    // save previous gd gl state
    saveGdState();

    // initialise what we need for rendering rmlui

    // setup our framebuffer
    glGenFramebuffers(1, &rmluiFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, rmluiFbo);

    unsigned int texBuf;
    glGenTextures(1, &texBuf);
    glBindTexture(GL_TEXTURE_2D, texBuf);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        viewportWidth,
        viewportHeight,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        NULL
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D,
        texBuf,
        0
    );

    const GLuint fbStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (fbStatus != GL_FRAMEBUFFER_COMPLETE) {
        geode::log::error(
            "[ERROR_FRAMBUFFER_CREATION] framebuffer could not be generated, {}",
            fbStatus
        );
        return false;
    }

    // glDeleteFramebuffers(1, &rmluiFbo);

    auto shader = PositionColorTexture::compile();
    if (!shader.has_value())
        return false;
    posTexColShader = shader.value();

    // return to gd gl state
    restoreGdState();

    return true;
}

void RenderInterface_GD::SetViewport(
    int in_viewportWidth,
    int in_viewportHeight
) {
    viewportWidth = in_viewportWidth;
    viewportHeight = in_viewportHeight;
}

void RenderInterface_GD::BeginFrame() {
    RMLUI_ASSERT(viewportWidth >= 0 && viewportHeight >= 0);

    saveGdState();

    // geode::log::debug("FFFF {}", rmluiFbo);

    glBindFramebuffer(GL_FRAMEBUFFER, rmluiFbo);

    glViewport(0, 0, viewportWidth, viewportHeight);

    glClearStencil(0);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_2D, rmluiTexture);

    // posTexColShader->use();

    // glBindVertexArray(rmluiVao);
    // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void RenderInterface_GD::EndFrame() {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, rmluiFbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gdState.fbo);

    glBlitFramebuffer(
        0,
        0,
        viewportWidth,
        viewportHeight,
        0,
        0,
        viewportWidth,
        viewportHeight,
        GL_COLOR_BUFFER_BIT,
        GL_NEAREST
    );

    restoreGdState();
}

Rml::CompiledGeometryHandle RenderInterface_GD::CompileGeometry(
    Rml::Span<const Rml::Vertex> vertices,
    Rml::Span<const int> indices
) {
    geode::log::debug("COMPILE GEOMETRY");
    // saveGdState();

    // create the vaos for this geometry
    // pretty much taken from the default GL3 renderer
    constexpr GLenum drawUsage = GL_STATIC_DRAW;

    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ibo = 0;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ibo);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(Rml::Vertex) * vertices.size(),
        (const void*)vertices.data(),
        drawUsage
    );

    glEnableVertexAttribArray(PositionColorTexture::Attribute::Position);
    glVertexAttribPointer(
        PositionColorTexture::Attribute::Position,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Rml::Vertex),
        (const GLvoid*)(offsetof(Rml::Vertex, position))
    );

    glEnableVertexAttribArray(PositionColorTexture::Attribute::Color);
    glVertexAttribPointer(
        PositionColorTexture::Attribute::Color,
        4,
        GL_UNSIGNED_BYTE,
        GL_TRUE,
        sizeof(Rml::Vertex),
        (const GLvoid*)(offsetof(Rml::Vertex, colour))
    );

    glEnableVertexAttribArray(PositionColorTexture::Attribute::TexCoord);
    glVertexAttribPointer(
        PositionColorTexture::Attribute::TexCoord,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Rml::Vertex),
        (const GLvoid*)(offsetof(Rml::Vertex, tex_coord))
    );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        sizeof(int) * indices.size(),
        (const void*)indices.data(),
        drawUsage
    );

    CompiledGeometryData* geometry = new CompiledGeometryData;
    geometry->vao = vao;
    geometry->vbo = vbo;
    geometry->ibo = ibo;
    geometry->drawCount = (GLsizei)indices.size();

    // restoreGdState();

    return (Rml::CompiledGeometryHandle)geometry;
}

void RenderInterface_GD::ReleaseGeometry(Rml::CompiledGeometryHandle geometry) {
    delete reinterpret_cast<CompiledGeometryData*>(geometry);
}

void RenderInterface_GD::RenderGeometry(
    Rml::CompiledGeometryHandle handle,
    Rml::Vector2f translation,
    Rml::TextureHandle texture
) {
    CompiledGeometryData* geometry = (CompiledGeometryData*)handle;

    // glMatrixMode(GL_TEXTURE);
    // glLoadIdentity();
    // glMatrixMode(GL_MODELVIEW);
    // glLoadIdentity();
    // glMatrixMode(GL_PROJECTION);
    // glLoadIdentity();

    if (texture) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, (GLuint)texture);
    } else {
        // TODO: no texture
    }

    // the gooden projection for rmlui
    kmMat4 customOrthoMatrix;
    kmMat4OrthographicProjection(
        &customOrthoMatrix,
        0.0f,
        viewportWidth,
        viewportWidth,
        0.0f,
        -1.0f,
        1.0f
    );

    // get current cocos/opengl matricies
    kmMat4 cocosMatrixP;
    kmMat4 cocosMatrixMV;
    kmMat4 cocosMatrixMVP;
    kmGLGetMatrix(KM_GL_PROJECTION, &cocosMatrixP);
    kmGLGetMatrix(KM_GL_MODELVIEW, &cocosMatrixMV);
    kmMat4Multiply(&cocosMatrixMVP, &cocosMatrixP, &cocosMatrixMV);

    cocosMatrixMVP.mat[13] += 2;

    // // cocos uses a combination of ancient opengl apis and shader uniforms to set transformations
    // // we dont our rendering to be effected by the current transformation, however we also dont want to use the ancient apis to reset them
    // // instead, if we pass our matrix as a unform to the vertex shader, but also apply a negated version of the current cocos transformation
    // // this should correct for cocos' transformation while not having to change them
    // kmMat4 inverseCocosMatrixMVP;
    // kmMat4Inverse(&inverseCocosMatrixMVP, &cocosMatrixMVP);

    // kmMat4 finalMatrix;
    // kmMat4Multiply(&finalMatrix, &customOrthoMatrix, &inverseCocosMatrixMVP);

    // geode::log::debug("m {}", finalMatrix.mat);

    posTexColShader->use();
    posTexColShader->setFloatMat4(
        PositionColorTexture::Uniform::MatTransform,
        cocosMatrixMVP.mat,
        1
    );
    posTexColShader->setFloatVec2(
        PositionColorTexture::Uniform::Translation,
        &translation.x,
        1
    );

    glBindVertexArray(geometry->vao);
    glDrawElements(GL_TRIANGLES, geometry->drawCount, GL_UNSIGNED_INT, 0);
    // cocos2d::CCEGLView::sharedOpenGLView()->swapBuffers();
}

void RenderInterface_GD::EnableScissorRegion(bool enable) {}

void RenderInterface_GD::EnableClipMask(bool enable) {}

void RenderInterface_GD::RenderToClipMask(
    Rml::ClipMaskOperation operation,
    Rml::CompiledGeometryHandle geometry,
    Rml::Vector2f translation
) {}

void RenderInterface_GD::SetScissorRegion(Rml::Rectanglei region) {}

void RenderInterface_GD::SetTransform(const Rml::Matrix4f* transform) {}

// TODO: im not sure if putting in the cache is a good idea or not
Rml::TextureHandle RenderInterface_GD::GenerateTexture(
    Rml::Span<const Rml::byte> source,
    Rml::Vector2i source_dimensions
) {
    geode::Ref<cocos2d::CCImage> image = new cocos2d::CCImage();
    image->initWithImageData(
        (void*)source.data(),
        source.size(),
        cocos2d::CCImage::kFmtRawData,
        source_dimensions.x,
        source_dimensions.y
    );

    geode::Ref<cocos2d::CCTexture2D> texture = new cocos2d::CCTexture2D();
    texture->initWithImage(image);

    return (Rml::TextureHandle)texture->getName();
}

void RenderInterface_GD::ReleaseTexture(Rml::TextureHandle texture_handle) {
    auto name = textureIdMap[texture_handle].c_str();

    // geode::log::debug("[RenderInterface_GD::ReleaseTexture] removing texture
    // {}", name);

    cocos2d::CCTextureCache::sharedTextureCache()->removeTextureForKey(name);
}

Rml::TextureHandle RenderInterface_GD::LoadTexture(
    Rml::Vector2i& texture_dimensions,
    const Rml::String& source
) {
    auto name = source.c_str();
    geode::Ref<cocos2d::CCTexture2D> texture =
        cocos2d::CCTextureCache::sharedTextureCache()->addImage(name, false);

    // geode::log::debug("[RenderInterface_GD::LoadTexture] loading texture {}",
    // source);

    textureIdMap[texture->getName()] = source;

    texture_dimensions.x = texture->getPixelsWide();
    texture_dimensions.y = texture->getPixelsHigh();

    return (Rml::TextureHandle)texture->getName();
}
