#pragma once

#include <vector>
#include <memory>
#include <QVulkanInstance>
#include <Urchin3dEngine.h>

namespace urchin {

    class SceneDisplayerWindow;

    struct QtSurfaceCreator final : public urchin::SurfaceCreator {
        explicit QtSurfaceCreator(SceneDisplayerWindow*);
        void* createSurface(void*) override;

        SceneDisplayerWindow *window;
    };

    struct QtFramebufferSizeRetriever final : public urchin::FramebufferSizeRetriever {
        explicit QtFramebufferSizeRetriever(SceneDisplayerWindow*);
        void getFramebufferSizeInPixel(unsigned int&, unsigned int&) const override;

        SceneDisplayerWindow *window;
    };

    class SceneWindowController {
        public:
            explicit SceneWindowController(SceneDisplayerWindow*);

            static std::vector<std::string> windowRequiredExtensions();
            std::unique_ptr<QtSurfaceCreator> getSurfaceCreator() const;
            std::unique_ptr<QtFramebufferSizeRetriever> getFramebufferSizeRetriever() const;

        private:
            SceneDisplayerWindow *window;
    };

}
