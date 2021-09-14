#include <QtWidgets/QHBoxLayout>
#include <UrchinMapHandler.h>

#include <widget/style/GroupBoxStyleHelper.h>
#include <widget/style/SpinBoxStyleHelper.h>
#include <widget/style/ButtonStyleHelper.h>
#include <widget/style/ComboBoxStyleHelper.h>
#include <widget/style/FrameStyleHelper.h>
#include <panel/models/ModelPanelWidget.h>
#include <panel/models/dialog/NewModelDialog.h>
#include <panel/models/dialog/CloneModelDialog.h>
#include <panel/models/dialog/ChangeBodyShapeDialog.h>
#include <panel/models/bodyshape/BodyShapeWidgetRetriever.h>
#include <scene/models/move/ModelMoveController.h>
#include <panel/models/bodyshape/NoBodyShapeWidget.h>
#include <scene/SceneDisplayerWindow.h>

namespace urchin {

    ModelPanelWidget::ModelPanelWidget() :
            modelController(nullptr),
            modelTableView(nullptr),
            addModelButton(nullptr),
            removeModelButton(nullptr),
            cloneModelButton(nullptr),
            tabWidget(nullptr),
            disableModelEvent(false),
            positionX(nullptr), positionY(nullptr), positionZ(nullptr),
            orientationType(nullptr),
            eulerAxis0(nullptr), eulerAxis1(nullptr), eulerAxis2(nullptr),
            scale(nullptr),
            produceShadowCheckBox(nullptr),
            tags(nullptr),
            hasRigidBody(nullptr),
            tabPhysicsRigidBody(nullptr),
            physicsShapeLayout(nullptr),
            mass(nullptr), restitution(nullptr), friction(nullptr), rollingFriction(nullptr),
            linearDamping(nullptr), angularDamping(nullptr),
            linearFactorX(nullptr), linearFactorY(nullptr), linearFactorZ(nullptr),
            angularFactorX(nullptr), angularFactorY(nullptr), angularFactorZ(nullptr),
            shapeTypeValueLabel(nullptr),
            changeBodyShapeButton(nullptr),
            bodyShapeWidget(nullptr) {
        auto* mainLayout = new QVBoxLayout(this);
        mainLayout->setAlignment(Qt::AlignTop);
        mainLayout->setContentsMargins(1, 1, 1, 1);

        modelTableView = new ModelTableView(this);
        mainLayout->addWidget(modelTableView);
        modelTableView->addObserver(this, ModelTableView::MODEL_SELECTION_CHANGED);
        modelTableView->setFixedHeight(220);

        auto* buttonsLayout = new QHBoxLayout();
        mainLayout->addLayout(buttonsLayout);
        buttonsLayout->setAlignment(Qt::AlignmentFlag::AlignLeft);

        addModelButton = new QPushButton("New Model");
        buttonsLayout->addWidget(addModelButton);
        ButtonStyleHelper::applyNormalStyle(addModelButton);
        connect(addModelButton, SIGNAL(clicked()), this, SLOT(showAddModelDialog()));

        removeModelButton = new QPushButton("Remove Model");
        buttonsLayout->addWidget(removeModelButton);
        ButtonStyleHelper::applyNormalStyle(removeModelButton);
        removeModelButton->setEnabled(false);
        connect(removeModelButton, SIGNAL(clicked()), this, SLOT(removeSelectedModel()));

        cloneModelButton = new QPushButton("Clone Model");
        buttonsLayout->addWidget(cloneModelButton);
        ButtonStyleHelper::applyNormalStyle(cloneModelButton);
        cloneModelButton->setEnabled(false);
        connect(cloneModelButton, SIGNAL(clicked()), this, SLOT(showCloneModelDialog()));

        tabWidget = new QTabWidget();
        mainLayout->addWidget(tabWidget);
        tabWidget->hide();

        //general properties
        auto* tabGeneral = new QWidget();
        auto* generalLayout = new QVBoxLayout(tabGeneral);
        generalLayout->setAlignment(Qt::AlignmentFlag::AlignTop);
        generalLayout->setContentsMargins(1, 1, 1, 1);
        setupTransformBox(generalLayout);
        setupFlagsBox(generalLayout);
        setupTagsBox(generalLayout);
        tabWidget->addTab(tabGeneral, "General");

        //physics properties
        auto* tabPhysics = new QWidget();
        auto* physicsLayout = new QVBoxLayout(tabPhysics);
        physicsLayout->setAlignment(Qt::AlignmentFlag::AlignTop);
        physicsLayout->setContentsMargins(1, 1, 1, 1);
        setupPhysicsBox(physicsLayout);
        tabWidget->addTab(tabPhysics, "Physics");
    }

    ModelTableView* ModelPanelWidget::getModelTableView() const {
        return modelTableView;
    }

    BodyShapeWidget* ModelPanelWidget::getBodyShapeWidget() const {
        return bodyShapeWidget.get();
    }

    void ModelPanelWidget::setupTransformBox(QVBoxLayout* generalLayout) {
        auto* transformGroupBox = new QGroupBox("Transform");
        generalLayout->addWidget(transformGroupBox);
        GroupBoxStyleHelper::applyNormalStyle(transformGroupBox);
        transformGroupBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

        auto* transformLayout = new QGridLayout(transformGroupBox);

        setupPosition(transformLayout);
        setupOrientation(transformLayout);
        setupScale(transformLayout);
    }

    void ModelPanelWidget::setupPosition(QGridLayout* transformLayout) {
        auto* positionLabel= new QLabel("Position:");
        transformLayout->addWidget(positionLabel, 0, 0);

        auto* positionLayout = new QHBoxLayout();
        transformLayout->addLayout(positionLayout, 0, 1);
        positionX = new QDoubleSpinBox();
        positionLayout->addWidget(positionX);
        SpinBoxStyleHelper::applyDefaultStyleOn(positionX);
        connect(positionX, SIGNAL(valueChanged(double)), this, SLOT(updateModelTransform()));
        positionY = new QDoubleSpinBox();
        positionLayout->addWidget(positionY);
        SpinBoxStyleHelper::applyDefaultStyleOn(positionY);
        connect(positionY, SIGNAL(valueChanged(double)), this, SLOT(updateModelTransform()));
        positionZ = new QDoubleSpinBox();
        positionLayout->addWidget(positionZ);
        SpinBoxStyleHelper::applyDefaultStyleOn(positionZ);
        connect(positionZ, SIGNAL(valueChanged(double)), this, SLOT(updateModelTransform()));
    }

    void ModelPanelWidget::setupOrientation(QGridLayout* transformLayout) {
        auto* orientationTypeLabel = new QLabel("Orient. Type:");
        transformLayout->addWidget(orientationTypeLabel, 1, 0);

        orientationType = new QComboBox();
        transformLayout->addWidget(orientationType, 1, 1);
        ComboBoxStyleHelper::applyOrientationStyleOn(orientationType);
        connect(orientationType, SIGNAL(currentIndexChanged(int)), this, SLOT(updateModelOrientationType()));

        auto* eulerAngleLabel = new QLabel("Euler Angle:");
        transformLayout->addWidget(eulerAngleLabel, 2, 0);

        auto* eulerAxisLayout = new QHBoxLayout();
        transformLayout->addLayout(eulerAxisLayout, 2, 1);
        eulerAxis0 = new QDoubleSpinBox();
        eulerAxisLayout->addWidget(eulerAxis0);
        SpinBoxStyleHelper::applyAngleStyleOn(eulerAxis0);
        SpinBoxStyleHelper::applyAngleStyleOn(eulerAxis0);
        connect(eulerAxis0, SIGNAL(valueChanged(double)), this, SLOT(updateModelTransform()));
        eulerAxis1 = new QDoubleSpinBox();
        eulerAxisLayout->addWidget(eulerAxis1);
        SpinBoxStyleHelper::applyAngleStyleOn(eulerAxis1);
        connect(eulerAxis1, SIGNAL(valueChanged(double)), this, SLOT(updateModelTransform()));
        eulerAxis2 = new QDoubleSpinBox();
        eulerAxisLayout->addWidget(eulerAxis2);
        SpinBoxStyleHelper::applyAngleStyleOn(eulerAxis2);
        connect(eulerAxis2, SIGNAL(valueChanged(double)), this, SLOT(updateModelTransform()));
    }

    void ModelPanelWidget::setupScale(QGridLayout* transformLayout) {
        auto* angleLabel = new QLabel("Scale:");
        transformLayout->addWidget(angleLabel, 3, 0);

        scale = new QDoubleSpinBox();
        transformLayout->addWidget(scale, 3, 1);
        SpinBoxStyleHelper::applyDefaultStyleOn(scale);
        scale->setMinimum(0.01);
        connect(scale, SIGNAL(valueChanged(double)), this, SLOT(updateModelScale()));
    }

    void ModelPanelWidget::setupFlagsBox(QVBoxLayout* generalLayout) {
        auto* flagsGroupBox = new QGroupBox("Flags");
        generalLayout->addWidget(flagsGroupBox);
        GroupBoxStyleHelper::applyNormalStyle(flagsGroupBox);
        flagsGroupBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

        auto* flagsLayout = new QGridLayout(flagsGroupBox);

        produceShadowCheckBox = new QCheckBox("Product Shadow");
        flagsLayout->addWidget(produceShadowCheckBox);
        connect(produceShadowCheckBox, SIGNAL(stateChanged(int)), this, SLOT(updateModelFlags()));
    }

    void ModelPanelWidget::setupTagsBox(QVBoxLayout* generalLayout) {
        auto* tagsGroupBox = new QGroupBox("Tags");
        generalLayout->addWidget(tagsGroupBox);
        GroupBoxStyleHelper::applyNormalStyle(tagsGroupBox);
        tagsGroupBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

        auto* tagsLayout = new QGridLayout(tagsGroupBox);

        auto* tagsLabel = new QLabel("Tags:");
        tagsLabel->setToolTip("Comma separated");
        tagsLayout->addWidget(tagsLabel, 0, 0);

        tags = new QLineEdit();
        tagsLayout->addWidget(tags, 0, 1);
        connect(tags, SIGNAL(textChanged(const QString &)), this, SLOT(updateModelTags()));
    }

    void ModelPanelWidget::setupPhysicsBox(QVBoxLayout* physicsLayout) {
        hasRigidBody = new QCheckBox("Rigid Body");
        physicsLayout->addWidget(hasRigidBody);
        connect(hasRigidBody, SIGNAL(stateChanged(int)), this, SLOT(rigidBodyToggled(int)));

        tabPhysicsRigidBody = new QTabWidget();
        physicsLayout->addWidget(tabPhysicsRigidBody);

        auto* tabPhysicsProperties = new QWidget();
        auto* physicsPropertiesLayout = new QVBoxLayout(tabPhysicsProperties);
        physicsPropertiesLayout->setAlignment(Qt::AlignmentFlag::AlignTop);
        physicsPropertiesLayout->setContentsMargins(1, 1, 1, 1);
        setupPhysicsGeneralPropertiesBox(physicsPropertiesLayout);
        setupPhysicsDampingPropertiesBox(physicsPropertiesLayout);
        setupPhysicsFactorPropertiesBox(physicsPropertiesLayout);
        tabPhysicsRigidBody->addTab(tabPhysicsProperties, "Properties");

        auto* tabPhysicsShape = new QWidget();
        physicsShapeLayout = new QVBoxLayout(tabPhysicsShape);
        physicsShapeLayout->setAlignment(Qt::AlignmentFlag::AlignTop);
        physicsShapeLayout->setContentsMargins(1, 1, 1, 1);
        setupPhysicsShapeBox(physicsShapeLayout);
        tabPhysicsRigidBody->addTab(tabPhysicsShape, "Shape");
    }

    void ModelPanelWidget::setupPhysicsGeneralPropertiesBox(QVBoxLayout* physicsPropertiesLayout) {
        auto* rigidBodyGeneralBox = new QGroupBox("General");
        physicsPropertiesLayout->addWidget(rigidBodyGeneralBox);
        GroupBoxStyleHelper::applyNormalStyle(rigidBodyGeneralBox);
        rigidBodyGeneralBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

        auto* rigidBodyGeneralLayout = new QGridLayout(rigidBodyGeneralBox);

        auto* massLabel = new QLabel("Mass:");
        rigidBodyGeneralLayout->addWidget(massLabel, 0, 0);

        mass = new QDoubleSpinBox();
        rigidBodyGeneralLayout->addWidget(mass, 0, 1);
        SpinBoxStyleHelper::applyDefaultStyleOn(mass);
        mass->setMinimum(0.0);
        connect(mass, SIGNAL(valueChanged(double)), this, SLOT(updateModelPhysicsProperties()));

        auto* restitutionLabel = new QLabel("Restitution:");
        rigidBodyGeneralLayout->addWidget(restitutionLabel, 0, 2);

        restitution = new QDoubleSpinBox();
        rigidBodyGeneralLayout->addWidget(restitution, 0, 3);
        SpinBoxStyleHelper::applyDefaultStyleOn(restitution);
        restitution->setMinimum(0.0);
        restitution->setMaximum(1.0);
        connect(restitution, SIGNAL(valueChanged(double)), this, SLOT(updateModelPhysicsProperties()));

        auto* frictionLabel = new QLabel("Friction:");
        rigidBodyGeneralLayout->addWidget(frictionLabel, 1, 0);

        friction = new QDoubleSpinBox();
        rigidBodyGeneralLayout->addWidget(friction, 1, 1);
        SpinBoxStyleHelper::applyDefaultStyleOn(friction);
        friction->setMinimum(0.0);
        friction->setMaximum(1.0);
        connect(friction, SIGNAL(valueChanged(double)), this, SLOT(updateModelPhysicsProperties()));

        auto* rollingFrictionLabel = new QLabel("Rolling Friction:");
        rigidBodyGeneralLayout->addWidget(rollingFrictionLabel, 1, 2);

        rollingFriction = new QDoubleSpinBox();
        rigidBodyGeneralLayout->addWidget(rollingFriction, 1, 3);
        SpinBoxStyleHelper::applyDefaultStyleOn(rollingFriction);
        rollingFriction->setMinimum(0.0);
        rollingFriction->setMaximum(1.0);
        connect(rollingFriction, SIGNAL(valueChanged(double)), this, SLOT(updateModelPhysicsProperties()));
    }

    void ModelPanelWidget::setupPhysicsDampingPropertiesBox(QVBoxLayout* physicsPropertiesLayout) {
        auto* rigidBodyDampingBox = new QGroupBox("Damping");
        physicsPropertiesLayout->addWidget(rigidBodyDampingBox);
        GroupBoxStyleHelper::applyNormalStyle(rigidBodyDampingBox);
        rigidBodyDampingBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

        auto* rigidBodyDampingLayout = new QGridLayout(rigidBodyDampingBox);

        auto* linearDampingLabel = new QLabel("Linear:");
        rigidBodyDampingLayout->addWidget(linearDampingLabel, 0, 0);

        linearDamping = new QDoubleSpinBox();
        rigidBodyDampingLayout->addWidget(linearDamping, 0, 1);
        SpinBoxStyleHelper::applyDefaultStyleOn(linearDamping);
        linearDamping->setMinimum(0.0);
        linearDamping->setMaximum(1.0);
        connect(linearDamping, SIGNAL(valueChanged(double)), this, SLOT(updateModelPhysicsProperties()));

        auto* angularDampingLabel = new QLabel("Angular:");
        rigidBodyDampingLayout->addWidget(angularDampingLabel, 0, 2);

        angularDamping = new QDoubleSpinBox();
        rigidBodyDampingLayout->addWidget(angularDamping, 0, 3);
        SpinBoxStyleHelper::applyDefaultStyleOn(angularDamping);
        angularDamping->setMinimum(0.0);
        angularDamping->setMaximum(1.0);
        connect(angularDamping, SIGNAL(valueChanged(double)), this, SLOT(updateModelPhysicsProperties()));
    }

    void ModelPanelWidget::setupPhysicsFactorPropertiesBox(QVBoxLayout* physicsPropertiesLayout) {
        auto* rigidBodyFactorBox = new QGroupBox("Factor");
        physicsPropertiesLayout->addWidget(rigidBodyFactorBox);
        GroupBoxStyleHelper::applyNormalStyle(rigidBodyFactorBox);
        rigidBodyFactorBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

        auto* rigidBodyFactorLayout = new QGridLayout(rigidBodyFactorBox);

        auto* linearFactorLabel = new QLabel("Linear:");
        rigidBodyFactorLayout->addWidget(linearFactorLabel, 0, 0);

        auto* linearFactorLayout = new QHBoxLayout();
        rigidBodyFactorLayout->addLayout(linearFactorLayout, 0, 1);
        linearFactorX = new QDoubleSpinBox();
        linearFactorLayout->addWidget(linearFactorX);
        SpinBoxStyleHelper::applyDefaultStyleOn(linearFactorX);
        linearFactorX->setMinimum(0.0);
        linearFactorX->setMaximum(1.0);
        connect(linearFactorX, SIGNAL(valueChanged(double)), this, SLOT(updateModelPhysicsProperties()));
        linearFactorY = new QDoubleSpinBox();
        linearFactorLayout->addWidget(linearFactorY);
        SpinBoxStyleHelper::applyDefaultStyleOn(linearFactorY);
        linearFactorY->setMinimum(0.0);
        linearFactorY->setMaximum(1.0);
        connect(linearFactorY, SIGNAL(valueChanged(double)), this, SLOT(updateModelPhysicsProperties()));
        linearFactorZ = new QDoubleSpinBox();
        linearFactorLayout->addWidget(linearFactorZ);
        SpinBoxStyleHelper::applyDefaultStyleOn(linearFactorZ);
        linearFactorZ->setMinimum(0.0);
        linearFactorZ->setMaximum(1.0);
        connect(linearFactorZ, SIGNAL(valueChanged(double)), this, SLOT(updateModelPhysicsProperties()));

        auto* angularFactorLabel = new QLabel("Angular:");
        rigidBodyFactorLayout->addWidget(angularFactorLabel, 1, 0);

        auto* angularFactorLayout = new QHBoxLayout();
        rigidBodyFactorLayout->addLayout(angularFactorLayout, 1, 1);
        angularFactorX = new QDoubleSpinBox();
        angularFactorLayout->addWidget(angularFactorX);
        SpinBoxStyleHelper::applyDefaultStyleOn(angularFactorX);
        angularFactorX->setMinimum(0.0);
        angularFactorX->setMaximum(1.0);
        connect(angularFactorX, SIGNAL(valueChanged(double)), this, SLOT(updateModelPhysicsProperties()));
        angularFactorY = new QDoubleSpinBox();
        angularFactorLayout->addWidget(angularFactorY);
        SpinBoxStyleHelper::applyDefaultStyleOn(angularFactorY);
        angularFactorY->setMinimum(0.0);
        angularFactorY->setMaximum(1.0);
        connect(angularFactorY, SIGNAL(valueChanged(double)), this, SLOT(updateModelPhysicsProperties()));
        angularFactorZ = new QDoubleSpinBox();
        angularFactorLayout->addWidget(angularFactorZ);
        SpinBoxStyleHelper::applyDefaultStyleOn(angularFactorZ);
        angularFactorZ->setMinimum(0.0);
        angularFactorZ->setMaximum(1.0);
        connect(angularFactorZ, SIGNAL(valueChanged(double)), this, SLOT(updateModelPhysicsProperties()));
    }

    void ModelPanelWidget::setupPhysicsShapeBox(QVBoxLayout* physicsShapeLayout) {
        auto* shapeTypeLayout = new QHBoxLayout();
        shapeTypeLayout->setAlignment(Qt::AlignLeft);
        shapeTypeLayout->setSpacing(15);
        physicsShapeLayout->addLayout(shapeTypeLayout);

        auto* shapeTypeLabel = new QLabel("Shape Type:");
        shapeTypeLayout->addWidget(shapeTypeLabel);

        shapeTypeValueLabel = new QLabel();
        shapeTypeLayout->addWidget(shapeTypeValueLabel);

        changeBodyShapeButton = new QPushButton("Change");
        shapeTypeLayout->addWidget(changeBodyShapeButton);
        ButtonStyleHelper::applyNormalStyle(changeBodyShapeButton);
        connect(changeBodyShapeButton, SIGNAL(clicked()), this, SLOT(showChangeBodyShapeDialog()));

        auto* frameLine = new QFrame();
        physicsShapeLayout->addWidget(frameLine);
        FrameStyleHelper::applyLineStyle(frameLine);

        bodyShapeWidget = nullptr;
    }

    void ModelPanelWidget::load(ModelController& modelController) {
        this->modelController = &modelController;

        std::list<const SceneModel*> sceneModels = this->modelController->getSceneModels();
        for (auto& sceneModel : sceneModels) {
            modelTableView->addModel(*sceneModel);
        }
    }

    void ModelPanelWidget::unload() {
        modelTableView->removeAllModels();

        modelController = nullptr;
    }

    void ModelPanelWidget::notify(Observable* observable, int notificationType) {
        if (auto* modelTableView = dynamic_cast<ModelTableView*>(observable)) {
            if (notificationType == ModelTableView::MODEL_SELECTION_CHANGED) {
                if (modelTableView->hasSceneModelSelected()) {
                    const SceneModel& sceneModel = *modelTableView->getSelectedSceneModel();
                    setupModelDataFrom(sceneModel);

                    removeModelButton->setEnabled(true);
                    cloneModelButton->setEnabled(true);
                    tabWidget->show();
                } else {
                    removeModelButton->setEnabled(false);
                    cloneModelButton->setEnabled(false);
                    tabWidget->hide();
                }
            }
        } else if (auto* sceneDisplayerWidget = dynamic_cast<SceneDisplayerWindow*>(observable)) {
            if (notificationType == SceneDisplayerWindow::BODY_PICKED) {
                const std::string& bodyId = sceneDisplayerWidget->getLastPickedBodyId();
                const SceneModel* sceneModel = bodyId.empty() ? nullptr : modelController->findSceneModelByBodyId(bodyId);
                if (sceneModel) {
                    int row = this->modelTableView->getSceneModelRow(sceneModel);
                    if (row >= 0) {
                        this->modelTableView->selectRow(row);
                    }
                } else {
                    this->modelTableView->clearSelection();
                }
            }
        } else if (auto* modelMoveController = dynamic_cast<ModelMoveController*>(observable)) {
            if (notificationType == ModelMoveController::MODEL_MOVED) {
                setupModelDataFrom(*modelMoveController->getSelectedSceneModel());
            }
        }
    }

    void ModelPanelWidget::setupModelDataFrom(const SceneModel& sceneModel) {
        disableModelEvent = true;
        const Model* model = sceneModel.getModel();
        const Transform<float>& modelTransform = model->getTransform();

        this->positionX->setValue(modelTransform.getPosition().X);
        this->positionY->setValue(modelTransform.getPosition().Y);
        this->positionZ->setValue(modelTransform.getPosition().Z);

        Vector3<float> eulerAngle = modelTransform.getOrientation().toEulerAngle(Quaternion<float>::RotationSequence::XYZ);
        this->eulerAxis0->setValue(AngleConverter<double>::toDegree(eulerAngle[0]));
        this->eulerAxis1->setValue(AngleConverter<double>::toDegree(eulerAngle[1]));
        this->eulerAxis2->setValue(AngleConverter<double>::toDegree(eulerAngle[2]));

        this->scale->setValue(modelTransform.getScale());

        this->produceShadowCheckBox->setChecked(model->isProduceShadow());

        std::string tagsValues = StringUtil::merge(model->getTags(), ModelReaderWriter::TAGS_SEPARATOR);
        this->tags->setText(QString::fromStdString(tagsValues));

        setupModelPhysicsDataFrom(sceneModel);
        disableModelEvent = false;
    }

    void ModelPanelWidget::setupModelPhysicsDataFrom(const SceneModel& sceneModel) {
        disableModelEvent = true;
        const RigidBody* rigidBody = sceneModel.getRigidBody();

        if (rigidBody) {
            hasRigidBody->setChecked(true);
            tabPhysicsRigidBody->show();

            mass->setValue(rigidBody->getMass());
            restitution->setValue(rigidBody->getRestitution());
            friction->setValue(rigidBody->getFriction());
            rollingFriction->setValue(rigidBody->getRollingFriction());

            linearDamping->setValue(rigidBody->getLinearDamping());
            angularDamping->setValue(rigidBody->getAngularDamping());

            linearFactorX->setValue(rigidBody->getLinearFactor().X);
            linearFactorY->setValue(rigidBody->getLinearFactor().Y);
            linearFactorZ->setValue(rigidBody->getLinearFactor().Z);
            angularFactorX->setValue(rigidBody->getAngularFactor().X);
            angularFactorY->setValue(rigidBody->getAngularFactor().Y);
            angularFactorZ->setValue(rigidBody->getAngularFactor().Z);

            BodyShapeWidget& bodyShapeWidget = createBodyShapeWidget(rigidBody->getShape(), sceneModel);
            bodyShapeWidget.setupShapePropertiesFrom(rigidBody->getShape());
            shapeTypeValueLabel->setText(QString::fromStdString(bodyShapeWidget.getBodyShapeName()));
        } else {
            hasRigidBody->setChecked(false);
            tabPhysicsRigidBody->hide();

            BodyShapeWidget& bodyShapeWidget = createNoBodyShapeWidget(sceneModel);
            shapeTypeValueLabel->setText(QString::fromStdString(bodyShapeWidget.getBodyShapeName()));
        }

        disableModelEvent = false;
    }

    BodyShapeWidget& ModelPanelWidget::createBodyShapeWidget(const CollisionShape3D& shape, const SceneModel& sceneModel) {
        bodyShapeWidget = BodyShapeWidgetRetriever(&sceneModel).createBodyShapeWidget(shape.getShapeType());
        setupBodyShapeWidget();
        return *bodyShapeWidget;
    }

    BodyShapeWidget& ModelPanelWidget::createNoBodyShapeWidget(const SceneModel& sceneModel) {
        bodyShapeWidget = std::make_unique<NoBodyShapeWidget>(&sceneModel);
        setupBodyShapeWidget();
        return *bodyShapeWidget;
    }

    void ModelPanelWidget::setupBodyShapeWidget() {
        physicsShapeLayout->addWidget(bodyShapeWidget.get());
        bodyShapeWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        bodyShapeWidget->show();
        connect(bodyShapeWidget.get(), SIGNAL(bodyShapeChange(std::unique_ptr<const CollisionShape3D>&)), this, SLOT(bodyShapeChanged(std::unique_ptr<const CollisionShape3D>&)));

        notifyObservers(this, NotificationType::MODEL_BODY_SHAPE_WIDGET_CREATED);
    }

    void ModelPanelWidget::showAddModelDialog() {
        NewModelDialog newSceneModelDialog(this, modelController);
        newSceneModelDialog.exec();

        if (newSceneModelDialog.result() == QDialog::Accepted) {
            std::unique_ptr<SceneModel> sceneModel = newSceneModelDialog.moveSceneModel();
            SceneModel* sceneModelPtr = sceneModel.get();
            modelController->addSceneModel(std::move(sceneModel));
            modelController->createDefaultBody(*sceneModelPtr);

            int row = modelTableView->addModel(*sceneModelPtr);
            modelTableView->selectRow(row);
        }
    }

    void ModelPanelWidget::removeSelectedModel() {
        if (modelTableView->hasSceneModelSelected()) {
            const SceneModel& sceneModel = *modelTableView->getSelectedSceneModel();
            modelController->removeSceneModel(sceneModel);

            modelTableView->removeSelectedModel();
        }
    }

    void ModelPanelWidget::showCloneModelDialog() {
        CloneModelDialog cloneSceneModelDialog(this, modelController);
        cloneSceneModelDialog.exec();

        if (cloneSceneModelDialog.result() == QDialog::Accepted) {
            std::unique_ptr<SceneModel> newSceneModel = cloneSceneModelDialog.moveSceneModel();
            SceneModel* newSceneModelPtr = newSceneModel.get();
            const SceneModel& toCloneSceneModel = *modelTableView->getSelectedSceneModel();
            modelController->cloneSceneModel(std::move(newSceneModel), toCloneSceneModel);

            int row = modelTableView->addModel(*newSceneModelPtr);
            modelTableView->selectRow(row);
        }
    }

    void ModelPanelWidget::updateModelOrientationType() {
        if (!disableModelEvent) {
            const SceneModel* sceneModel = modelTableView->getSelectedSceneModel();

            QVariant variant = orientationType->currentData();
            auto newRotationSequence = static_cast<Quaternion<float>::RotationSequence>(variant.toInt());

            Quaternion<float> orientation = sceneModel->getModel()->getTransform().getOrientation();
            Vector3<float> eulerAngle = orientation.toEulerAngle(newRotationSequence);

            eulerAxis0->setValue(AngleConverter<float>::toDegree(eulerAngle.X));
            eulerAxis1->setValue(AngleConverter<float>::toDegree(eulerAngle.Y));
            eulerAxis2->setValue(AngleConverter<float>::toDegree(eulerAngle.Z));

            updateModelTransform();
        }
    }

    void ModelPanelWidget::updateModelTransform() {
        if (!disableModelEvent) {
            const SceneModel& sceneModel = *modelTableView->getSelectedSceneModel();

            Vector3<float> eulerAngle(
                    AngleConverter<float>::toRadian((float)eulerAxis0->value()),
                    AngleConverter<float>::toRadian((float)eulerAxis1->value()),
                    AngleConverter<float>::toRadian((float)eulerAxis2->value())
            );

            QVariant variant = orientationType->currentData();
            auto rotationSequence = static_cast<Quaternion<float>::RotationSequence>(variant.toInt());

            Transform<float> newSceneModelTransform(
                    Point3<float>((float)positionX->value(),(float)positionY->value(),(float)positionZ->value()),
                    Quaternion<float>(eulerAngle, rotationSequence),
                    (float)scale->value());

            modelController->updateSceneModelTransform(sceneModel, newSceneModelTransform);
        }
    }

    void ModelPanelWidget::updateModelScale() {
        if (!disableModelEvent) {
            updateModelTransform();

            const SceneModel& sceneModel = *modelTableView->getSelectedSceneModel();
            if (sceneModel.getRigidBody()) {
                const CollisionShape3D& updatedCollisionShape = sceneModel.getRigidBody()->getShape();
                auto& bodyShapeWidget = createBodyShapeWidget(updatedCollisionShape, sceneModel);
                bodyShapeWidget.setupShapePropertiesFrom(updatedCollisionShape);
            }
        }
    }

    void ModelPanelWidget::updateModelFlags() {
        if (!disableModelEvent) {
            const SceneModel& sceneModel = *modelTableView->getSelectedSceneModel();

            bool produceShadow = produceShadowCheckBox->checkState() == Qt::Checked;
            modelController->updateSceneModelFlags(sceneModel, produceShadow);
        }
    }

    void ModelPanelWidget::updateModelTags() {
        if (!disableModelEvent) {
            const SceneModel& sceneModel = *modelTableView->getSelectedSceneModel();

            std::string tagsValues = tags->text().toUtf8().constData();
            modelController->updateSceneModelTags(sceneModel, tagsValues);
        }
    }

    void ModelPanelWidget::showChangeBodyShapeDialog() {
        ChangeBodyShapeDialog changeBodyShapeDialog(this, false);
        changeBodyShapeDialog.exec();

        if (changeBodyShapeDialog.result() == QDialog::Accepted) {
            const SceneModel& sceneModel = *modelTableView->getSelectedSceneModel();
            CollisionShape3D::ShapeType shapeType = changeBodyShapeDialog.getShapeType();

            modelController->changeBodyShape(sceneModel, shapeType);
            setupModelPhysicsDataFrom(sceneModel);
        }
    }

    void ModelPanelWidget::rigidBodyToggled(int rigidBodyState) {
        if (!disableModelEvent) {
            const SceneModel& sceneModel = *modelTableView->getSelectedSceneModel();
            if (Qt::CheckState::Checked == rigidBodyState) {
                tabPhysicsRigidBody->show();
                modelController->createDefaultBody(sceneModel);
            } else if (Qt::CheckState::Unchecked == rigidBodyState) {
                modelController->removeBody(sceneModel);
                tabPhysicsRigidBody->hide();
            }

            setupModelPhysicsDataFrom(sceneModel);
        }
    }

    void ModelPanelWidget::updateModelPhysicsProperties() {
        if (!disableModelEvent) {
            const SceneModel& sceneModel = *modelTableView->getSelectedSceneModel();

            Vector3<float> linearFactor((float)linearFactorX->value(), (float)linearFactorY->value(), (float)linearFactorZ->value());
            Vector3<float> angularFactor((float)angularFactorX->value(), (float)angularFactorY->value(), (float)angularFactorZ->value());

            modelController->updateSceneModelPhysicsProperties(sceneModel, (float)mass->value(), (float)restitution->value(),
                    (float)friction->value(), (float)rollingFriction->value(), (float)linearDamping->value(), (float)angularDamping->value(),
                    linearFactor, angularFactor);
        }
    }

    void ModelPanelWidget::bodyShapeChanged(std::unique_ptr<const CollisionShape3D>& shape) {
        if (!disableModelEvent) {
            const SceneModel& sceneModel = *modelTableView->getSelectedSceneModel();
            modelController->updateSceneModelPhysicsShape(sceneModel, std::move(shape));
        }
    }

}
