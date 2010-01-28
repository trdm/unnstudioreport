/********************************************************************************
** Form generated from reading ui file 'uoInputId.ui'
**
** Created: Sun 31. May 22:12:37 2009
**      by: Qt User Interface Compiler version 4.3.4
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_UOINPUTID_H
#define UI_UOINPUTID_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>

class Ui_uoInputId
{
public:
    QGridLayout *gridLayout;
    QVBoxLayout *vboxLayout;
    QLabel *label;
    QLineEdit *m_name;
    QHBoxLayout *hboxLayout;
    QSpacerItem *spacerItem;
    QPushButton *btnOk;
    QPushButton *btnCansel;

    void setupUi(QDialog *uoInputId)
    {
    if (uoInputId->objectName().isEmpty())
        uoInputId->setObjectName(QString::fromUtf8("uoInputId"));
    uoInputId->setWindowModality(Qt::WindowModal);
    uoInputId->resize(342, 82);
    gridLayout = new QGridLayout(uoInputId);
    gridLayout->setSpacing(6);
    gridLayout->setMargin(9);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
#if (QT_VERSION > 0x040201)
    gridLayout->setContentsMargins(6, 6, 6, 6);
#endif
    vboxLayout = new QVBoxLayout();
    vboxLayout->setSpacing(3);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    label = new QLabel(uoInputId);
    label->setObjectName(QString::fromUtf8("label"));

    vboxLayout->addWidget(label);

    m_name = new QLineEdit(uoInputId);
    m_name->setObjectName(QString::fromUtf8("m_name"));

    vboxLayout->addWidget(m_name);

    hboxLayout = new QHBoxLayout();
    hboxLayout->setSpacing(6);
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    spacerItem = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout->addItem(spacerItem);

    btnOk = new QPushButton(uoInputId);
    btnOk->setObjectName(QString::fromUtf8("btnOk"));
    btnOk->setIcon(QIcon(QString::fromUtf8(":/image/apply.png")));
    btnOk->setDefault(true);

    hboxLayout->addWidget(btnOk);

    btnCansel = new QPushButton(uoInputId);
    btnCansel->setObjectName(QString::fromUtf8("btnCansel"));
    btnCansel->setIcon(QIcon(QString::fromUtf8(":/image/cancel_16x16.png")));

    hboxLayout->addWidget(btnCansel);


    vboxLayout->addLayout(hboxLayout);


    gridLayout->addLayout(vboxLayout, 0, 0, 1, 1);


    retranslateUi(uoInputId);
    QObject::connect(btnOk, SIGNAL(clicked()), uoInputId, SLOT(accept()));
    QObject::connect(btnCansel, SIGNAL(clicked()), uoInputId, SLOT(reject()));

    QMetaObject::connectSlotsByName(uoInputId);
    } // setupUi

    void retranslateUi(QDialog *uoInputId)
    {
    uoInputId->setWindowTitle(QApplication::translate("uoInputId", "Input name", 0, QApplication::UnicodeUTF8));
    label->setText(QApplication::translate("uoInputId", "Input section name", 0, QApplication::UnicodeUTF8));
    btnOk->setText(QApplication::translate("uoInputId", "OK", 0, QApplication::UnicodeUTF8));
    btnCansel->setText(QApplication::translate("uoInputId", "Cancel", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(uoInputId);
    } // retranslateUi

};

namespace Ui {
    class uoInputId: public Ui_uoInputId {};
} // namespace Ui

#endif // UI_UOINPUTID_H
