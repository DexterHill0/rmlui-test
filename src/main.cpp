#include "Geode/cocos/base_nodes/CCNode.h"
#include "RmlUi/Core/Core.h"
#include "RmlUi/Core/Log.h"
#define RMLUI_STATIC_LIB

#include <string>
#include <iostream>
#include <thread>

#include <RmlUi/Core.h>
#include <RmlUi/Core/RenderInterface.h>
#include <Geode/Geode.hpp>

#include "renderer/Renderer.h"
#include "platform/Platform.h"
#include "Backend.h"
// #include <RmlUi/Shell.h>

using namespace geode::prelude;

const int width = 1024;
const int height = 768;

// $execute {
//     log::debug("[$execute] LOADING");
//     // std::thread t1(makeWindow);
//     auto render_interface = std::make_unique<RenderInterface_GD>();
//     Rml::SetRenderInterface(render_interface.get());


//     // if (!Shell::Initialize())
// 	// 	return -1;


//     if (!Backend::Initialize("Demo Sample", width, height, true))
// 	{
// 		// Shell::Shutdown();
// 		return;
// 	}

//     Rml::SetSystemInterface(Backend::GetSystemInterface());
// 	Rml::SetRenderInterface(Backend::GetRenderInterface());

//     Rml::Initialise();

//     // Rml::Context* context = Rml::CreateContext("main", Rml::Vector2i(width, height));
// 	// if (!context)
// 	// {
// 	// 	Rml::Shutdown();
// 	// 	Backend::Shutdown();
// 	// 	// Shell::Shutdown();
// 	// 	return;
// 	// }

// }

// $on_mod(Loaded) {
//     log::debug("[$execute] LOADING");
//     // std::thread t1(makeWindow);
//     auto render_interface = std::make_unique<RenderInterface_GD>();
//     Rml::SetRenderInterface(render_interface.get());

//     auto system_interface = std::make_unique<SystemInterface_GD>();
//     Rml::SetSystemInterface(system_interface.get());

//     // if (!Shell::Initialize())
// 	// 	return -1;

// 	log::debug("COCK A");
//     if (!Backend::Initialize("Demo Sample", width, height, true))
// 	{
// 		// Shell::Shutdown();
// 		log::debug("COCK B");
// 		return;
// 	}
	

// 	log::debug("COCK C {}", (uint64_t)data.get());
//     // Rml::SetSystemInterface(Backend::GetSystemInterface());
// 	// Rml::SetRenderInterface(Backend::GetRenderInterface());
// 	log::debug("COCK D");

//     auto sdoj = Rml::Initialise();
// 	log::debug("COCK E {}", sdoj);
// 	Rml::Log::Message(Rml::Log::LT_WARNING, "Exoplod9ng Cock");

//     Rml::Context* context = Rml::CreateContext("main", Rml::Vector2i(width, height));
// 	log::debug("vajines {} {}", (uint64_t)context, context == nullptr);
	
// 	if (context == nullptr)
// 	{
// 		log::debug("COCK F");
// 		Rml::Shutdown();
// 		log::debug("COCK G");
// 		Backend::Shutdown();
// 		log::debug("COCK H");
// 		// Shell::Shutdown();
// 		return;
// 	}

//     bool running = true;
//     while(running) {
//         // running = Backend::ProcessEvents(context, &Shell::ProcessKeyDownShortcuts, true);
// 		context->Update();

// 		Backend::BeginFrame();
// 		context->Render();
// 		Backend::PresentFrame();
//     }

//     Rml::Shutdown();

// 	Backend::Shutdown();

// 	log::debug("COCK I");
// }





class TheNode : public cocos2d::CCNode {
public:
    Rml::Context* context;

    TheNode() {}

    bool init() {
		if (!CCNode::init()) {
            return false;
        }

        log::debug("[$execute] LOADING");
        // std::thread t1(makeWindow);
        auto render_interface = std::make_unique<RenderInterface_GD>();
        Rml::SetRenderInterface(render_interface.get());

        auto system_interface = std::make_unique<SystemInterface_GD>();
        Rml::SetSystemInterface(system_interface.get());

        // if (!Shell::Initialize())
        // 	return -1;

        log::debug("COCK A");
        if (!Backend::Initialize("Demo Sample", width, height, true))
        {
            // Shell::Shutdown();
            log::debug("COCK B");
            return false;
        }
        

        // log::debug("COCK C {}", (uint64_t)data.get());
        // Rml::SetSystemInterface(Backend::GetSystemInterface());
        // Rml::SetRenderInterface(Backend::GetRenderInterface());
        log::debug("COCK D");

        auto sdoj = Rml::Initialise();
        log::debug("COCK E {}", sdoj);
        Rml::Log::Message(Rml::Log::LT_WARNING, "Exoplod9ng Cock");

        context = Rml::CreateContext("main", Rml::Vector2i(width, height));
        log::debug("vajines {} {}", (uint64_t)context, context == nullptr);
        
        if (context == nullptr)
        {
            log::debug("COCK F");
            Rml::Shutdown();
            log::debug("COCK G");
            Backend::Shutdown();
            log::debug("COCK H");
            // Shell::Shutdown();
            return false;
        }

        scheduleUpdate();

        return true;
    }

    void draw() {
        // context->Update();

        Backend::BeginFrame();
        context->Render();
        Backend::PresentFrame();
    }
	
	static TheNode* create() {
        auto node = new TheNode;
        if (!node->init()) {
            CC_SAFE_DELETE(node);
            return nullptr;
        }
        node->autorelease();
        return node;
    }
};


#include <Geode/modify/MenuLayer.hpp>
class $modify(MyMenuLayer, MenuLayer) {

	bool init() {
		if (!MenuLayer::init()) {
			return false;
		}

        auto node = TheNode::create();

        this->addChild(node);

		return true;
	}

	void onMyButton(CCObject*) {
		FLAlertLayer::create("Geode", "Hello from my custom mod!", "OK")->show();
	}
};
