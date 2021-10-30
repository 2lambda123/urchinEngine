#include <algorithm>
#include <utility>

#include <scene/ui/UIRenderer.h>
#include <resources/ResourceRetriever.h>
#include <resources/font/Font.h>
#include <api/render/shader/builder/ShaderBuilder.h>

namespace urchin {

    //debug parameters
    bool DEBUG_DISPLAY_FONT_TEXTURE = false;

    UIRenderer::UIRenderer(RenderTarget& renderTarget, I18nService& i18nService) :
            renderTarget(renderTarget),
            i18nService(i18nService),
            uiResolution(renderTarget.getWidth(), renderTarget.getHeight()) {
        if (renderTarget.isValidRenderTarget()) {
            uiShader = ShaderBuilder::createShader("ui.vert.spv", "", "ui.frag.spv");
        } else {
            uiShader = ShaderBuilder::createNullShader();
        }
    }

    void UIRenderer::setupUi3d(const Camera* camera, const Transform<float>& transform, const Point2<unsigned int>& uiResolution,
                               const Point2<float>& uiSize, float ambient) {
        assert(widgets.empty());
        assert(ui3dData == nullptr);
        assert(MathFunction::isEqual((float)uiResolution.X / (float)uiResolution.Y, uiSize.X / uiSize.Y, 0.1f)); //proportion must be equal for a good visual

        float xScale = uiSize.X / (float)uiResolution.X;
        float yScale = uiSize.Y / (float)uiResolution.Y;
        Matrix4<float> uiViewMatrix(xScale, 0.0f, 0.0f, 0.0f,
                                    0.0f, -yScale /* negate for flip on Y axis */, 0.0f, 0.0f,
                                    0.0f, 0.0f, 1.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f, 1.0f);
        this->ui3dData = std::make_unique<UI3dData>();
        this->ui3dData->modelMatrix = transform.getTransformMatrix() * uiViewMatrix;
        this->ui3dData->normalMatrix = ui3dData->modelMatrix.inverse().transpose();

        if (renderTarget.isValidRenderTarget()) {
            std::vector<std::size_t> variablesSize = {sizeof(ambient)};
            auto shaderConstants = std::make_unique<ShaderConstants>(variablesSize, &ambient);
            this->uiShader = ShaderBuilder::createShader("ui3d.vert.spv", "", "ui3d.frag.spv", std::move(shaderConstants));
        }

        onResize(uiResolution.X, uiResolution.Y);
        if (camera) {
            onCameraProjectionUpdate(*camera);
        }
    }

    void UIRenderer::onCameraProjectionUpdate(const Camera& camera) {
        if (!ui3dData) {
            throw std::runtime_error("UI renderer has not been initialized for UI 3d");
        }
        this->ui3dData->cameraProjectionMatrix = camera.getProjectionMatrix();
        this->ui3dData->cameraSpaceService = std::make_unique<CameraSpaceService>(camera);

        for (long i = (long)widgets.size() - 1; i >= 0; --i) {
            widgets[(std::size_t)i]->onCameraProjectionUpdate();
        }
    }

    void UIRenderer::onResize(unsigned int sceneWidth, unsigned int sceneHeight) {
        this->uiResolution = Point2<unsigned int>(sceneWidth, sceneHeight);

        //widgets resize
        for (long i = (long)widgets.size() - 1; i >= 0; --i) {
            widgets[(std::size_t)i]->onResize();
        }

        //debug
        if (DEBUG_DISPLAY_FONT_TEXTURE) {
            auto font = ResourceRetriever::instance().getResource<Font>("UI/fontText.ttf", {{"fontSize", "16"}, {"fontColor", "1.0 1.0 1.0"}});

            auto textureDisplayer = std::make_unique<TextureRenderer>(font->getTexture(), TextureRenderer::DEFAULT_VALUE);
            textureDisplayer->setPosition(TextureRenderer::USER_DEFINED_X, TextureRenderer::USER_DEFINED_Y);
            textureDisplayer->setSize(20.0f, (float)font->getDimensionTexture() + 20.0f, 20.0f, (float)font->getDimensionTexture() + 20.0f);
            textureDisplayer->enableTransparency();
            textureDisplayer->initialize("[DEBUG] font texture", renderTarget, sceneWidth, sceneHeight, -1.0f, -1.0f);
            debugFont = std::move(textureDisplayer);
        }
    }

    void UIRenderer::notify(Observable* observable, int notificationType) {
        if (auto* widget = dynamic_cast<Widget*>(observable)) {
            if (notificationType == Widget::SET_IN_FOREGROUND) {
                auto itFind = std::find_if(widgets.begin(), widgets.end(), [&widget](const auto& o){return widget == o.get();});
                std::shared_ptr<Widget> widgetPtr = *itFind;
                widgets.erase(itFind);
                widgets.push_back(widgetPtr);

                //reset the others widgets
                for (long i = (long)widgets.size() - 2; i >= 0; --i) {
                    widgets[(std::size_t)i]->onResetState();
                }
            }
        }
    }

    bool UIRenderer::onKeyPress(unsigned int key) {
        //keep a temporary copy of the widgets in case the underlying action goal is to destroy the widgets
        std::vector<std::shared_ptr<Widget>> widgetsCopy = widgets;
        for (long i = (long)widgetsCopy.size() - 1; i >= 0; --i) {
            if (!widgetsCopy[(std::size_t)i]->onKeyPress(key)) {
                return false;
            }
        }
        return true;
    }

    bool UIRenderer::onKeyRelease(unsigned int key) {
        //keep a temporary copy of the widgets in case the underlying action goal is to destroy the widgets
        std::vector<std::shared_ptr<Widget>> widgetsCopy = widgets;
        for (long i = (long)widgetsCopy.size() - 1; i >= 0; --i) {
            if (!widgetsCopy[(std::size_t)i]->onKeyRelease(key)) {
                return false;
            }
        }
        return true;
    }

    bool UIRenderer::onChar(char32_t unicodeCharacter) {
        if (unicodeCharacter > 0x00 && unicodeCharacter < 0xFF //accept 'Basic Latin' and 'Latin-1 Supplement'
                && unicodeCharacter > 0x1F //ignore 'Controls C0' unicode
                && (unicodeCharacter < 0x80 || unicodeCharacter > 0x9F) //ignore 'Controls C1' unicode
                && unicodeCharacter != 127 //ignore 'Delete' unicode
        ) {
            for (long i = (long) widgets.size() - 1; i >= 0; --i) {
                if (!widgets[(std::size_t) i]->onChar(unicodeCharacter)) {
                    return false;
                }
            }
        }
        return true;
    }

    bool UIRenderer::onMouseMove(double mouseX, double mouseY) {
        if (ui3dData && ui3dData->cameraSpaceService) { //TODO ...
//            float clipSpaceX = (2.0f * (float)mouseX) / ((float)renderTarget.getWidth()) - 1.0f;
//            float clipSpaceY = (2.0f * (float)mouseY) / ((float)renderTarget.getHeight()) - 1.0f;
//            Point4<float> mousePos(clipSpaceX, clipSpaceY, 0.944f, 1.0f);
//
//            Matrix4<float> inverseMatrix = (getUi3dData()->cameraProjectionMatrix * viewMatrix * getUi3dData()->modelMatrix).inverse();
//            std::cout<<(inverseMatrix * mousePos).divideByW()<<std::endl;

            Ray<float> ray = ui3dData->cameraSpaceService->screenPointToRay(Point2<float>((float)mouseX, (float)mouseY), 20.0f /*TODO review param */); //TOOD return Line3D
            Line3D<float> line(ray.getOrigin(), ray.computeTo());

            Matrix4<float> matrix = getUi3dData()->cameraProjectionMatrix * viewMatrix * getUi3dData()->modelMatrix;
            Point4<float> topLeft = (matrix * Point4<float>(0.0f, 0.0f, 0.0f, 1.0f)).divideByW();
            Point4<float> topRight = (matrix * Point4<float>(800.0f, 0.0f, 0.0f, 1.0f)).divideByW();
            Point4<float> bottomRight = (matrix * Point4<float>(800.0f, 600.0f, 0.0f, 1.0f)).divideByW();
            Plane<float> plane(topLeft.toPoint3(), topRight.toPoint3(), bottomRight.toPoint3());

            bool intersection = false;
            Point3<float> intersectionPoint = plane.intersectPoint(line, intersection);
            if (intersection) {
                std::cout<<"Intersection at: "<<intersectionPoint<<std::endl;
            } else {
                std::cout<<"No intersection"<<std::endl;
            }
        }

        for (long i = (long)widgets.size() - 1; i >= 0; --i) {
            if (!widgets[(std::size_t)i]->onMouseMove((int)mouseX, (int)mouseY)) {
                return false;
            }
        }
        return true;
    }

    bool UIRenderer::onScroll(double offsetY) {
        for (long i = (long)widgets.size() - 1; i >= 0; --i) {
            if (!widgets[(std::size_t)i]->onScroll(offsetY)) {
                return false;
            }
        }
        return true;
    }

    void UIRenderer::onDisable() {
        for (long i = (long)widgets.size() - 1; i >= 0; --i) {
            widgets[(std::size_t)i]->onResetState();
        }
    }

    RenderTarget& UIRenderer::getRenderTarget() const {
        return renderTarget;
    }

    I18nService& UIRenderer::getI18nService() const {
        return i18nService;
    }

    const Point2<unsigned int>& UIRenderer::getUiResolution() const {
        return uiResolution;
    }

    Shader& UIRenderer::getShader() const {
        return *uiShader;
    }

    UI3dData* UIRenderer::getUi3dData() const {
        return ui3dData.get();
    }

    void UIRenderer::addWidget(const std::shared_ptr<Widget>& widget) {
        if (widget->getParent()) {
            throw std::runtime_error("Cannot add a widget having a parent to the UI renderer");
        }
        widgets.push_back(widget);

        widget->initialize(*this);
        widget->addObserver(this, Widget::SET_IN_FOREGROUND);
    }

    void UIRenderer::removeWidget(Widget& widget) {
        auto itFind = std::find_if(widgets.begin(), widgets.end(), [&widget](const auto& o){return &widget == o.get();});
        if (itFind == widgets.end()) {
            throw std::runtime_error("The provided widget is not widget of this UI renderer");
        }
        widgets.erase(itFind);
    }

    void UIRenderer::removeAllWidgets() {
        widgets.clear();
    }

    const std::vector<std::shared_ptr<Widget>>& UIRenderer::getWidgets() const {
        return widgets;
    }

    void UIRenderer::prepareRendering(float dt, unsigned int& screenRenderingOrder) {
        prepareRendering(dt, screenRenderingOrder, Matrix4<float>());
    }

    void UIRenderer::prepareRendering(float dt, unsigned int& renderingOrder, const Matrix4<float>& viewMatrix) {
        ScopeProfiler sp(Profiler::graphic(), "uiPreRendering");

        this->viewMatrix = viewMatrix;

        for (auto& widget : widgets) {
            renderingOrder++;
            widget->prepareRendering(dt, renderingOrder, viewMatrix);
        }

        //debug
        if (debugFont) {
            renderingOrder++;
            debugFont->prepareRendering(renderingOrder);
        }
    }

}
