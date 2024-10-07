#include "Renderer.h"
#include "Geode/cocos/platform/CCImage.h"
#include "Geode/cocos/platform/win32/CCEGLView.h"
#include "Geode/cocos/shaders/CCGLProgram.h"
#include "Geode/loader/Log.hpp"
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/FileInterface.h>
#include <RmlUi/Core/Log.h>
#include <RmlUi/Core/Platform.h>
#include <string.h>

#include <Geode/Geode.hpp>
#include <cocos2d.h>

#include "./shaders/test/Test.h"

#if defined RMLUI_PLATFORM_WIN32
	#include "RmlUi_Include_Windows.h"
	#include <gl/Gl.h>
	#include <gl/Glu.h>
#elif defined RMLUI_PLATFORM_MACOSX
	#include <AGL/agl.h>
	#include <OpenGL/gl.h>
	#include <OpenGL/glext.h>
	#include <OpenGL/glu.h>
#elif defined RMLUI_PLATFORM_UNIX
	#include "RmlUi_Include_Xlib.h"
	#include <GL/gl.h>
	#include <GL/glext.h>
	#include <GL/glu.h>
	#include <GL/glx.h>
#endif



#define GL_CLAMP_TO_EDGE 0x812F

void RenderInterface_GD::saveGdState() {
    glGetIntegerv(GL_CURRENT_PROGRAM, &gdState.shaderProgram);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &gdState.vao);
    glGetIntegerv(GL_TEXTURE_2D, &gdState.texture);
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &gdState.fbo);
}

void RenderInterface_GD::restoreGdState() {
    glUseProgram(gdState.shaderProgram);
    glBindVertexArray(gdState.vao);
    glBindTexture(GL_TEXTURE_2D, gdState.texture); 
    glBindFramebuffer(GL_FRAMEBUFFER, gdState.fbo);

    glViewport(0, 0, viewport_width, viewport_height);
}

// RenderInterface_GD::~RenderInterface_GD() {
//     glDeleteVertexArrays(1, &rmluiState.vao);
//     glDeleteBuffers(1, &VBO);
//     glDeleteBuffers(1, &EBO);
//     glDeleteProgram(shaderProgram);
// }

RenderInterface_GD::RenderInterface_GD() {}

bool RenderInterface_GD::Initialise() {
    saveGdState();

    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    // create a color attachment texture
    unsigned int textureColorbuffer;
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, viewport_width, viewport_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

    rmluiFbo = framebuffer;

    glDeleteFramebuffers(1, &framebuffer);  

    auto shader = Test::compile();
    if (!shader.has_value()) return false;
    testShader = shader.value();

    float vertices[] = {
        // positions          // colors           // texture coords
         0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
         0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
        -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 
    };
    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    rmluiVao = VAO;

    auto docPath = geode::Mod::get()->getResourcesDir() /  "container.jpg";

    auto dimensions = Rml::Vector2i();
    rmluiTexture = LoadTexture(dimensions, docPath.string());

    // geode::log::warn("TEXTURE {}", rmluiTexture);

    restoreGdState();

    return true;
}

void RenderInterface_GD::SetViewport(int in_viewport_width, int in_viewport_height)
{
	viewport_width = in_viewport_width;
	viewport_height = in_viewport_height;
}

void RenderInterface_GD::BeginFrame()
{
	RMLUI_ASSERT(viewport_width >= 0 && viewport_height >= 0);

    saveGdState();

    glBindFramebuffer(GL_FRAMEBUFFER, rmluiFbo);

    glViewport(0, 0, viewport_width, viewport_height);

	glClearStencil(0);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    // geode::log::debug("TEST SHADER {}", testShader.programId);

    // int vertexColorLocation = glGetUniformLocation(rmluiProgram, "ourColor");
    // glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rmluiTexture);

    testShader->use();

    glBindVertexArray(rmluiVao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    // glDrawArrays(GL_TRIANGLES, 0, 3);

	transform_enabled = false;
}

void RenderInterface_GD::EndFrame() {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, rmluiFbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gdState.fbo); 

    glBlitFramebuffer(0, 0, viewport_width, viewport_height, 0, 0, viewport_width, viewport_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    restoreGdState();
}

void RenderInterface_GD::FinaliseFrame() {
}

void RenderInterface_GD::Clear()
{
}

Rml::CompiledGeometryHandle RenderInterface_GD::CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices)
{
	GeometryView* data = new GeometryView{vertices, indices};
	return reinterpret_cast<Rml::CompiledGeometryHandle>(data);
}

void RenderInterface_GD::ReleaseGeometry(Rml::CompiledGeometryHandle geometry)
{
	delete reinterpret_cast<GeometryView*>(geometry);
}


void RenderInterface_GD::RenderGeometry(Rml::CompiledGeometryHandle handle, Rml::Vector2f translation, Rml::TextureHandle texture)
{

    // cocos2d::CCEGLView::sharedOpenGLView()->swapBuffers();
}

void RenderInterface_GD::EnableScissorRegion(bool enable)
{
	
}

void RenderInterface_GD::EnableClipMask(bool enable)
{

}

void RenderInterface_GD::RenderToClipMask(Rml::ClipMaskOperation operation, Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation)
{
    
}

void RenderInterface_GD::SetScissorRegion(Rml::Rectanglei region)
{
    
}

void RenderInterface_GD::SetTransform(const Rml::Matrix4f* transform)
{
    
}

// TODO: im not sure if putting in the cache is a good idea or not
Rml::TextureHandle RenderInterface_GD::GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i source_dimensions)
{
    geode::Ref<cocos2d::CCImage> image = new cocos2d::CCImage();
    image->initWithImageData((void*)source.data(), source.size(), cocos2d::CCImage::kFmtRawData, source_dimensions.x, source_dimensions.y);

    geode::Ref<cocos2d::CCTexture2D> texture = new cocos2d::CCTexture2D(); 
    texture->initWithImage(image);

	return (Rml::TextureHandle)texture->getName();
}

void RenderInterface_GD::ReleaseTexture(Rml::TextureHandle texture_handle)
{
    auto name = textureIdMap[texture_handle].c_str();

    // geode::log::debug("[RenderInterface_GD::ReleaseTexture] removing texture {}", name);

    cocos2d::CCTextureCache::sharedTextureCache()->removeTextureForKey(name);
}

Rml::TextureHandle RenderInterface_GD::LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source)
{   
    auto name = source.c_str();
    geode::Ref<cocos2d::CCTexture2D> texture = cocos2d::CCTextureCache::sharedTextureCache()->addImage(name, false);

    // geode::log::debug("[RenderInterface_GD::LoadTexture] loading texture {}", source);

    textureIdMap[texture->getName()] = source;

    texture_dimensions.x = texture->getPixelsWide();
    texture_dimensions.y = texture->getPixelsHigh();

	return (Rml::TextureHandle)texture->getName();
}

