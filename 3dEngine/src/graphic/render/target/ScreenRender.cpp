#include "libs/glad/glad.h"
#include "ScreenRender.h"
#include "graphic/render/GenericRenderer.h"

namespace urchin {

    void ScreenRender::resetDisplay() const {
        glClear((unsigned int)GL_DEPTH_BUFFER_BIT | (unsigned int)GL_COLOR_BUFFER_BIT);
    }

    void ScreenRender::display(const std::unique_ptr<GenericRenderer>& renderer) const {
        renderer->getShader()->bind();

        glViewport(0, 0, getTargetWidth(), getTargetHeight());

        executeRenderer(renderer);
    }

}
