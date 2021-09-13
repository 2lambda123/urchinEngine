#include <stdexcept>
#include <QMessageBox>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFileDialog>

#include <panel/models/dialog/CloneObjectDialog.h>
#include <widget/style/LabelStyleHelper.h>

namespace urchin {
    CloneObjectDialog::CloneObjectDialog(QWidget* parent, const ObjectController* modelController) :
            QDialog(parent),
            modelController(modelController),
            modelNameLabel(nullptr),
            modelNameText(nullptr),
            sceneModel(nullptr) {
        this->setWindowTitle("Clone Model");
        this->resize(530, 80);
        this->setFixedSize(this->width(), this->height());

        auto* mainLayout = new QGridLayout(this);
        mainLayout->setAlignment(Qt::AlignmentFlag::AlignLeft);

        setupNameFields(mainLayout);

        auto* buttonBox = new QDialogButtonBox();
        mainLayout->addWidget(buttonBox, 2, 0, 1, 3);
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);

        QObject::connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    }

    void CloneObjectDialog::setupNameFields(QGridLayout* mainLayout) {
        modelNameLabel = new QLabel("Model Name:");
        mainLayout->addWidget(modelNameLabel, 0, 0);

        modelNameText = new QLineEdit();
        mainLayout->addWidget(modelNameText, 0, 1);
        modelNameText->setFixedWidth(360);
    }

    void CloneObjectDialog::updateObjectName() {
        QString modelName = modelNameText->text();
        if (!modelName.isEmpty()) {
            this->modelName = modelName.toUtf8().constData();
        }
    }

    int CloneObjectDialog::buildSceneModel(int result) {
        try {
            sceneModel = std::make_unique<SceneModel>();
            sceneModel->setName(modelName);
        } catch (std::exception& e) {
            QMessageBox::critical(this, "Error", e.what());
            return QDialog::Rejected;
        }

        return result;
    }

    std::unique_ptr<SceneModel> CloneObjectDialog::moveSceneModel() {
        return std::move(sceneModel);
    }

    void CloneObjectDialog::done(int r) {
        if (QDialog::Accepted == r) {
            bool hasError = false;

            updateObjectName();
            LabelStyleHelper::applyNormalStyle(modelNameLabel);

            if (modelName.empty()) {
                LabelStyleHelper::applyErrorStyle(modelNameLabel, "Model name is mandatory");
                hasError = true;
            } else if (isSceneModelExist(modelName)) {
                LabelStyleHelper::applyErrorStyle(modelNameLabel, "Model name is already used");
                hasError = true;
            }

            if (!hasError) {
                r = buildSceneModel(r);
                QDialog::done(r);
            }
        } else {
            QDialog::done(r);
        }
    }

    bool CloneObjectDialog::isSceneModelExist(const std::string& name) {
        std::list<const SceneModel*> sceneModels = modelController->getSceneModels();
        return std::any_of(sceneModels.begin(), sceneModels.end(), [&name](const auto& so){return so->getName() == name;});
    }

}
