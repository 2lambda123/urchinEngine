#pragma once

#include <list>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QLabel>

#include <UrchinAggregation.h>
#include <controller/objects/ObjectController.h>

namespace urchin {

    class RenameObjectDialog : public QDialog {
        Q_OBJECT

        public:
            RenameObjectDialog(QWidget*, const ObjectController*);

            std::string getObjectName() const;

        private:
            void setupNameFields(QGridLayout*);

            void updateObjectName();

            void done(int) override;
            bool isObjectEntityExist(const std::string&);

            const ObjectController* objectController;

            QLabel* objectNameLabel;
            QLineEdit* objectNameText;

            std::string objectName;
    };

}
