#pragma once

#include <chrono>
#include <vector>
#include <array>
#include <UrchinCommon.h>

#include <scene/renderer3d/Renderer3d.h>
#include <scene/UI/UIRenderer.h>
#include <graphic/setup/GraphicService.h>
#include <graphic/render/target/ScreenRender.h>

#define NUM_RENDERER 2

namespace urchin {

    class SceneManager {
        public:
            SceneManager(const std::vector<std::string>&, const std::unique_ptr<SurfaceCreator>&, std::unique_ptr<FramebufferSizeRetriever>);
            ~SceneManager();

            //scene properties
            void updateVerticalSync(bool);
            void onResize(unsigned int, unsigned int);
            unsigned int getSceneWidth() const;
            unsigned int getSceneHeight() const;

            //Fps
            float getFps() const;
            unsigned int getFpsForDisplay();
            float getDeltaTime() const;

            //renderer
            Renderer3d* newRenderer3d(bool);
            void enableRenderer3d(Renderer3d*);
            void removeRenderer3d(Renderer3d*);
            Renderer3d* getActiveRenderer3d() const;

            UIRenderer* newUIRenderer(bool);
            void enableUIRenderer(UIRenderer*);
            void removeUIRenderer(UIRenderer*);
            UIRenderer* getActiveUIRenderer() const;

            //events
            bool onKeyPress(unsigned int);
            bool onKeyRelease(unsigned int);
            bool onChar(unsigned int);
            bool onMouseMove(int, int);

            //scene
            void display();

        private:
            void computeFps();

            //scene properties
            unsigned int sceneWidth, sceneHeight;

            //renderer
            std::shared_ptr<ScreenRender> screenRenderTarget;
            std::vector<Renderer3d*> renderers3d;
            std::vector<UIRenderer*> uiRenderers;
            Renderer *activeRenderers[NUM_RENDERER];

            //fps
            std::chrono::high_resolution_clock::time_point previousTime;
            unsigned int indexFps;
            std::array<float, 3> previousFps;
            float fps;
            unsigned int fpsForDisplay;
    };

}
