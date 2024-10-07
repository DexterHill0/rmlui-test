#include <RmlUi/Core.h>
#include <RmlUi/Core/FileInterface.h>
#include <RmlUi/Core/RenderInterface.h>

#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>

#include "Geode/cocos/base_nodes/CCNode.h"
#include "Geode/loader/Log.hpp"
#include "RmlUi/Core/Context.h"
#include "RmlUi/Core/Core.h"
#include "RmlUi/Core/ElementDocument.h"

#define RMLUI_STATIC_LIB

#include <string>

#include "Backend.h"
#include "renderer/Renderer.h"

using namespace geode::prelude;

auto getFrameSize() {
    auto* director = cocos2d::CCDirector::sharedDirector();
    const auto frameSize = director->getOpenGLView()->getFrameSize()
        * geode::utils::getDisplayFactor();

    return frameSize;
}

// currently u can only have 1 of these nodes
// it would be cool in the future to be able to have more than 1 if possible
class RmlUINode: public cocos2d::CCNode {
  public:
    Rml::Context* context;
    Rml::ElementDocument* document;

    RmlUINode() {}

    bool init() {
        if (!CCNode::init()) {
            return false;
        }

        auto frameSize = getFrameSize();
        Backend::Initialize(frameSize.width, frameSize.height);

        Rml::SetRenderInterface(Backend::GetRenderInterface());

        Rml::Initialise();

        if (!Backend::GetRenderInterface()
                 ->Initialise(frameSize.width, frameSize.height)) {
            return false;
        }

        context = Rml::CreateContext(
            "main",
            Rml::Vector2i(frameSize.width, frameSize.height)
        );
        if (context == nullptr) {
            log::error("Failed to create rml context");
            return false;
        }

        // test resources
        auto fontPath = Mod::get()->getResourcesDir() / "Roboto.ttf";
        Rml::LoadFontFace(fontPath.string());

        auto docPath = Mod::get()->getResourcesDir() / "test.rml";
        document = context->LoadDocument(docPath.string());

        if (!document) {
            log::error("Failed to load rml document");
            return false;
        }

        document->Show();

        scheduleUpdate();

        return true;
    }

    void onEnter() {
        auto parentSize = getParent()->getContentSize();
        setContentSize(parentSize);
        setAnchorPoint({0, 0});
    }

    void draw() {
        // GLint current_fbo;
        // glGetIntegerv(GL_FRAMEBUFFER_BINDING, &current_fbo);
        // log::debug("FBO before render: {}", current_fbo);

        context->Update();
        Backend::BeginFrame();
        context->Render();
        Backend::PresentFrame();

        // glGetIntegerv(GL_FRAMEBUFFER_BINDING, &current_fbo);
        // log::debug("FBO in render: {}", current_fbo);

        // glGetIntegerv(GL_FRAMEBUFFER_BINDING, &current_fbo);
        // log::debug("FBO after render: {}", current_fbo);
    }

    ~RmlUINode() {
        Backend::Shutdown();
        Rml::Shutdown();
    }

    static RmlUINode* create() {
        auto node = new RmlUINode;
        if (!node->init()) {
            CC_SAFE_DELETE(node);
            return nullptr;
        }
        node->autorelease();
        return node;
    }
};

// clang-format off
class $modify(MyMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) {
            return false;
        }

        auto node = RmlUINode::create();
        node->setID("rmlui-test-node");

        this->addChild(node);

        return true;
    }
};
