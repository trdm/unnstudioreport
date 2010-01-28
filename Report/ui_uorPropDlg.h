/********************************************************************************
** Form generated from reading ui file 'uorPropDlg.ui'
**
** Created: Sun 13. Dec 18:52:40 2009
**      by: Qt User Interface Compiler version 4.3.4
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_UORPROPDLG_H
#define UI_UORPROPDLG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QGridLayout>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QTabWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

class Ui_uoPropDlg
{
public:
    QGridLayout *gridLayout;
    QGridLayout *gridLayout1;
    QVBoxLayout *vboxLayout;
    QPushButton *m_btnOK;
    QPushButton *m_btnCancel;
    QPushButton *m_btmUpdate;
    QSpacerItem *spacerItem;
    QTabWidget *propFrame;

    void setupUi(QWidget *uoPropDlg)
    {
    if (uoPropDlg->objectName().isEmpty())
        uoPropDlg->setObjectName(QString::fromUtf8("uoPropDlg"));
    uoPropDlg->resize(466, 150);
    uoPropDlg->setWindowIcon(QIcon());
    gridLayout = new QGridLayout(uoPropDlg);
    gridLayout->setSpacing(1);
    gridLayout->setMargin(1);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    gridLayout1 = new QGridLayout();
#ifndef Q_OS_MAC
    gridLayout1->setSpacing(6);
#endif
#ifndef Q_OS_MAC
    gridLayout1->setMargin(0);
#endif
    gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
    vboxLayout = new QVBoxLayout();
#ifndef Q_OS_MAC
    vboxLayout->setSpacing(6);
#endif
#ifndef Q_OS_MAC
    vboxLayout->setMargin(0);
#endif
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    m_btnOK = new QPushButton(uoPropDlg);
    m_btnOK->setObjectName(QString::fromUtf8("m_btnOK"));
    QSizePolicy sizePolicy(static_cast<QSizePolicy::Policy>(0), static_cast<QSizePolicy::Policy>(0));
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(m_btnOK->sizePolicy().hasHeightForWidth());
    m_btnOK->setSizePolicy(sizePolicy);
    m_btnOK->setMinimumSize(QSize(74, 24));
    m_btnOK->setMaximumSize(QSize(74, 24));
    m_btnOK->setBaseSize(QSize(74, 24));
    m_btnOK->setIcon(QIcon());
    m_btnOK->setDefault(true);
    m_btnOK->setFlat(false);

    vboxLayout->addWidget(m_btnOK);

    m_btnCancel = new QPushButton(uoPropDlg);
    m_btnCancel->setObjectName(QString::fromUtf8("m_btnCancel"));
    sizePolicy.setHeightForWidth(m_btnCancel->sizePolicy().hasHeightForWidth());
    m_btnCancel->setSizePolicy(sizePolicy);
    m_btnCancel->setMinimumSize(QSize(74, 24));
    m_btnCancel->setMaximumSize(QSize(74, 24));

    vboxLayout->addWidget(m_btnCancel);

    m_btmUpdate = new QPushButton(uoPropDlg);
    m_btmUpdate->setObjectName(QString::fromUtf8("m_btmUpdate"));
    sizePolicy.setHeightForWidth(m_btmUpdate->sizePolicy().hasHeightForWidth());
    m_btmUpdate->setSizePolicy(sizePolicy);
    m_btmUpdate->setMinimumSize(QSize(74, 24));
    m_btmUpdate->setMaximumSize(QSize(74, 24));

    vboxLayout->addWidget(m_btmUpdate);

    spacerItem = new QSpacerItem(10, 2, QSizePolicy::Minimum, QSizePolicy::Expanding);

    vboxLayout->addItem(spacerItem);


    gridLayout1->addLayout(vboxLayout, 0, 1, 1, 1);

    propFrame = new QTabWidget(uoPropDlg);
    propFrame->setObjectName(QString::fromUtf8("propFrame"));
    QSizePolicy sizePolicy1(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(1));
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(propFrame->sizePolicy().hasHeightForWidth());
    propFrame->setSizePolicy(sizePolicy1);
    propFrame->setMinimumSize(QSize(380, 147));
    propFrame->setTabShape(QTabWidget::Rounded);

    gridLayout1->addWidget(propFrame, 0, 0, 1, 1);


    gridLayout->addLayout(gridLayout1, 0, 0, 1, 1);


    retranslateUi(uoPropDlg);

    QMetaObject::connectSlotsByName(uoPropDlg);
    } // setupUi

    void retranslateUi(QWidget *uoPropDlg)
    {
    uoPropDlg->setWindowTitle(QApplication::translate("uoPropDlg", "Form", 0, QApplication::UnicodeUTF8));
    m_btnOK->setText(QApplication::translate("uoPropDlg", "OK", 0, QApplication::UnicodeUTF8));
    m_btnCancel->setText(QApplication::translate("uoPropDlg", "Cancel", 0, QApplication::UnicodeUTF8));
    m_btmUpdate->setText(QApplication::translate("uoPropDlg", "Update", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(uoPropDlg);
    } // retranslateUi

};

namespace Ui {
    class uoPropDlg: public Ui_uoPropDlg {};
} // namespace Ui

#endif // UI_UORPROPDLG_H
