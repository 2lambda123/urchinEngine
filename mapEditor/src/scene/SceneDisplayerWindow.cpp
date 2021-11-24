#include <utility>
#include <QtWidgets/QShortcut>

#include <scene/SceneDisplayerWindow.h>
#include <scene/SceneWindowController.h>
#include <widget/controller/mouse/MouseController.h>

namespace urchin {

    SceneDisplayerWindow::SceneDisplayerWindow(QWidget* parent, StatusBarController& statusBarController, std::string mapEditorPath) :
            statusBarController(statusBarController),
            mapEditorPath(std::move(mapEditorPath)),
            sceneWindowController(std::make_unique<SceneWindowController>(this)),
            mouseController(MouseController(this)),
            viewProperties(),
            mouseX(0),
            mouseY(0) {
        GraphicService::enableUniqueSurface();
        setSurfaceType(QSurface::VulkanSurface);

        QObject::connect(new QShortcut(QKeySequence((int)Qt::CTRL + Qt::Key_X), parent), SIGNAL(activated()), this, SLOT(onCtrlXPressed()));
        QObject::connect(new QShortcut(QKeySequence((int)Qt::CTRL + Qt::Key_Y), parent), SIGNAL(activated()), this, SLOT(onCtrlYPressed()));
        QObject::connect(new QShortcut(QKeySequence((int)Qt::CTRL + Qt::Key_Z), parent), SIGNAL(activated()), this, SLOT(onCtrlZPressed()));
    }

    SceneDisplayerWindow::~SceneDisplayerWindow() {
        GraphicService::destroySurface();
    }

    void SceneDisplayerWindow::exposeEvent(QExposeEvent *) {
        if (isExposed()) {
            render(); //launch first frame
        }
    }

    bool SceneDisplayerWindow::event(QEvent *e) {
        switch (e->type()) {
            case QEvent::UpdateRequest: {
                render();
                break;
            } case QEvent::Resize: {
                if (sceneDisplayer) {
                    unsigned int width = 0, height = 0;
                    SceneWindowController(this).getFramebufferSizeRetriever()->getFramebufferSizeInPixel(width, height);
                    sceneDisplayer->resize(width, height);
                }
                break;
            } case QEvent::Leave: { //mouse leaves widget
                Point2<int> mousePositionInPixel = mouseController.getMousePosition();
                if (sceneDisplayer && sceneDisplayer->getModelMoveController() && !geometry().contains(QPoint(mousePositionInPixel.X, mousePositionInPixel.Y))) {
                    sceneDisplayer->getModelMoveController()->onMouseOut();
                }
                break;
            } default: {
                break;
            }
        }
        return QWindow::event(e);
    }

    void SceneDisplayerWindow::setupVkInstance(VkInstance instance) {
        vulkanInstance.setVkInstance(instance);
        if (!vulkanInstance.create()) {
            throw std::runtime_error("Failed to create Vulkan instance: " + std::to_string(vulkanInstance.errorCode()));
        }
        setVulkanInstance(&vulkanInstance);
    }

    void SceneDisplayerWindow::clearVkInstance() {
        vulkanInstance.destroy();
        setVulkanInstance(nullptr);
    }

    void SceneDisplayerWindow::loadMap(SceneController& sceneController, const std::string& mapFilename, const std::string& relativeWorkingDirectory) {
        closeMap();
        statusBarController.applyState(StatusBarState::MAP_LOADED);

        sceneDisplayer = std::make_unique<SceneDisplayer>(*sceneWindowController, &sceneController, mouseController, statusBarController);
        sceneDisplayer->loadMap(mapEditorPath, mapFilename, relativeWorkingDirectory);
        sceneDisplayer->resize( //size is computed in pixel
                MathFunction::roundToUInt((float)geometry().width() * (float)devicePixelRatio()),
                MathFunction::roundToUInt((float)geometry().height() * (float)devicePixelRatio()));
        sceneController.setup(&sceneDisplayer->getMapHandler());
        updateSceneDisplayerViewProperties();
    }

    void SceneDisplayerWindow::loadEmptyScene() {
        sceneDisplayer = std::make_unique<SceneDisplayer>(*sceneWindowController, nullptr, mouseController, statusBarController);
        sceneDisplayer->loadEmptyScene(mapEditorPath);
        sceneDisplayer->resize( //size is computed in pixel
                MathFunction::roundToUInt((float)geometry().width() * (float)devicePixelRatio()),
                MathFunction::roundToUInt((float)geometry().height() * (float)devicePixelRatio()));
    }

    void SceneDisplayerWindow::saveState(const std::string& mapFilename) const {
        if (sceneDisplayer && sceneDisplayer->getCamera()) {
            sceneDisplayer->getCamera()->saveCameraState(mapFilename);
        }
    }

    void SceneDisplayerWindow::closeMap() {
        statusBarController.clearState();
        clearVkInstance();

        sceneDisplayer.reset(nullptr);
    }

    void SceneDisplayerWindow::setViewProperties(SceneDisplayer::ViewProperties viewProperty, bool value) {
        viewProperties[viewProperty] = value;
        updateSceneDisplayerViewProperties();
    }

    void SceneDisplayerWindow::setHighlightSceneModel(const SceneModel* highlightSceneModel) {
        if (sceneDisplayer) {
            sceneDisplayer->setHighlightSceneModel(highlightSceneModel);
        }
    }

    void SceneDisplayerWindow::setHighlightCompoundShapeComponent(const LocalizedCollisionShape* selectedCompoundShapeComponent) {
        if (sceneDisplayer) {
            sceneDisplayer->getBodyShapeDisplayer()->setSelectedCompoundShapeComponent(selectedCompoundShapeComponent);
        }
    }

    void SceneDisplayerWindow::setHighlightSceneLight(const SceneLight* highlightSceneLight) {
        if (sceneDisplayer) {
            sceneDisplayer->setHighlightSceneLight(highlightSceneLight);
        }
    }

    void SceneDisplayerWindow::setHighlightSceneSound(const SceneSound* highlightSceneSound) {
        if (sceneDisplayer) {
            sceneDisplayer->setHighlightSceneSound(highlightSceneSound);
        }
    }

    void SceneDisplayerWindow::updateSceneDisplayerViewProperties() {
        if (sceneDisplayer) {
            for (unsigned int i = 0; i < SceneDisplayer::LAST_VIEW_PROPERTIES; ++i) {
                sceneDisplayer->setViewProperties(static_cast<SceneDisplayer::ViewProperties>(i), viewProperties[i]);
            }
        }
    }

    void SceneDisplayerWindow::render() {
        if (!sceneDisplayer) {
            loadEmptyScene();
        }

        sceneDisplayer->paint();

        requestUpdate();
    }

    void SceneDisplayerWindow::keyPressEvent(QKeyEvent* event) {
        if (sceneDisplayer) {
            if (event->key() < 256) {
                sceneDisplayer->getScene().onKeyPress((unsigned int)event->key());
                sceneDisplayer->getScene().onChar((unsigned int)event->text().toLatin1()[0]);
            } else if (event->key() == Qt::Key_Left) {
                sceneDisplayer->getScene().onKeyPress((unsigned int)InputDeviceKey::LEFT_ARROW);
            } else if (event->key() == Qt::Key_Right) {
                sceneDisplayer->getScene().onKeyPress((unsigned int)InputDeviceKey::RIGHT_ARROW);
            } else if (event->key() == Qt::Key_Backspace) {
                sceneDisplayer->getScene().onChar(8);
            } else if (event->key() == Qt::Key_Delete) {
                sceneDisplayer->getScene().onChar(127);
            }
        }
    }

    void SceneDisplayerWindow::keyReleaseEvent(QKeyEvent* event) {
        if (sceneDisplayer) {
            if (event->key() < 256) {
                sceneDisplayer->getScene().onKeyRelease((unsigned int)event->key());
            } else if (event->key() == Qt::Key_Left) {
                sceneDisplayer->getScene().onKeyRelease((unsigned int)InputDeviceKey::LEFT_ARROW);
            } else if (event->key() == Qt::Key_Right) {
                sceneDisplayer->getScene().onKeyRelease((unsigned int)InputDeviceKey::RIGHT_ARROW);
            } else if (event->key() == Qt::Key_Escape) {
                sceneDisplayer->getModelMoveController()->onEscapeKey();
            }
        }
    }

    void SceneDisplayerWindow::mousePressEvent(QMouseEvent* event) {
        if (sceneDisplayer) {
            if (event->buttons() == Qt::LeftButton) {
                sceneDisplayer->getScene().onKeyPress((unsigned int)InputDeviceKey::MOUSE_LEFT);
            } else if (event->button() == Qt::RightButton) {
                sceneDisplayer->getScene().onKeyPress((unsigned int)InputDeviceKey::MOUSE_RIGHT);
            }
        }
    }

    void SceneDisplayerWindow::mouseReleaseEvent(QMouseEvent* event) {
        if (sceneDisplayer) {
            if (event->button() == Qt::LeftButton) {
                bool propagateEvent = sceneDisplayer->getModelMoveController() == nullptr || sceneDisplayer->getModelMoveController()->onMouseLeftButton();
                if (propagateEvent) {
                    propagateEvent = onMouseClickBodyPickup();
                    if (propagateEvent) {
                        sceneDisplayer->getScene().onKeyRelease((unsigned int)InputDeviceKey::MOUSE_LEFT);
                    }
                }
            } else if (event->button() == Qt::RightButton) {
                sceneDisplayer->getScene().onKeyRelease((unsigned int)InputDeviceKey::MOUSE_RIGHT);
            }
        }
    }

    void SceneDisplayerWindow::mouseMoveEvent(QMouseEvent* event) {
        //mouse coordinate computed in pixel
        this->mouseX = MathFunction::roundToInt((float)event->x() * (float)devicePixelRatio());
        this->mouseY = MathFunction::roundToInt((float)event->y() * (float)devicePixelRatio());

        if (sceneDisplayer) {
            bool propagateEvent = sceneDisplayer->getScene().onMouseMove(mouseX, mouseY);
            if (propagateEvent && sceneDisplayer->getModelMoveController()) {
                sceneDisplayer->getModelMoveController()->onMouseMove(mouseX, mouseY);
            }
        }
    }

    bool SceneDisplayerWindow::onMouseClickBodyPickup() {
        bool propagateEvent = true;

        if (sceneDisplayer->getScene().getActiveRenderer3d()) {
            constexpr float PICKING_RAY_LENGTH = 100.0f;
            const Camera* camera = sceneDisplayer->getScene().getActiveRenderer3d()->getCamera();
            Ray<float> pickingRay = CameraSpaceService(*camera).screenPointToRay(Point2<float>((float) mouseX, (float) mouseY), PICKING_RAY_LENGTH);
            std::shared_ptr<const RayTestResult> rayTestResult = sceneDisplayer->getPhysicsWorld().rayTest(pickingRay);

            while (!rayTestResult->isResultReady()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
            const ccd_set& pickedModels = rayTestResult->getResults();
            if (!pickedModels.empty()) {
                lastPickedBodyId = (*pickedModels.begin())->getBody2().getId();
                notifyObservers(this, BODY_PICKED);
                propagateEvent = false;
            } else {
                lastPickedBodyId = "";
                notifyObservers(this, BODY_PICKED);
            }
        }

        return propagateEvent;
    }

    const std::string& SceneDisplayerWindow::getLastPickedBodyId() const {
        return lastPickedBodyId;
    }

    void SceneDisplayerWindow::addObserverModelMoveController(Observer* observer, int notificationType) {
        assert(sceneDisplayer);
        sceneDisplayer->getModelMoveController()->addObserver(observer, notificationType);
    }

    void SceneDisplayerWindow::onCtrlXPressed() {
        if (sceneDisplayer && sceneDisplayer->getModelMoveController()) {
            sceneDisplayer->getModelMoveController()->onCtrlXYZ(0);
        }
    }

    void SceneDisplayerWindow::onCtrlYPressed() {
        if (sceneDisplayer && sceneDisplayer->getModelMoveController()) {
            sceneDisplayer->getModelMoveController()->onCtrlXYZ(1);
        }
    }

    void SceneDisplayerWindow::onCtrlZPressed() {
        if (sceneDisplayer && sceneDisplayer->getModelMoveController()) {
            sceneDisplayer->getModelMoveController()->onCtrlXYZ(2);
        }
    }

}
