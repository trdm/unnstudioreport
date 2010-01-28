/********************************************************************************
** Form generated from reading ui file 'uorTextLayotProp.ui'
**
** Created: Sun 13. Dec 18:52:40 2009
**      by: Qt User Interface Compiler version 4.3.4
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_UORTEXTLAYOTPROP_H
#define UI_UORTEXTLAYOTPROP_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QGroupBox>
#include <QtGui/QRadioButton>
#include <QtGui/QWidget>

class Ui_uorTextLayotProp
{
public:
    QGroupBox *groupBox;
    QRadioButton *m_TopVTA;
    QRadioButton *m_CenterVTA;
    QRadioButton *m_BottomVTA;
    QGroupBox *groupBox_2;
    QRadioButton *m_LeftHTA;
    QRadioButton *m_CenterHTA;
    QRadioButton *m_RightHTA;
    QCheckBox *chbUseSelectedCell;

    void setupUi(QWidget *uorTextLayotProp)
    {
    if (uorTextLayotProp->objectName().isEmpty())
        uorTextLayotProp->setObjectName(QString::fromUtf8("uorTextLayotProp"));
    uorTextLayotProp->resize(321, 110);
    uorTextLayotProp->setWindowIcon(QIcon());
    groupBox = new QGroupBox(uorTextLayotProp);
    groupBox->setObjectName(QString::fromUtf8("groupBox"));
    groupBox->setGeometry(QRect(10, 10, 86, 91));
    QSizePolicy sizePolicy(static_cast<QSizePolicy::Policy>(0), static_cast<QSizePolicy::Policy>(5));
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(groupBox->sizePolicy().hasHeightForWidth());
    groupBox->setSizePolicy(sizePolicy);
    m_TopVTA = new QRadioButton(groupBox);
    m_TopVTA->setObjectName(QString::fromUtf8("m_TopVTA"));
    m_TopVTA->setGeometry(QRect(10, 20, 39, 19));
    m_CenterVTA = new QRadioButton(groupBox);
    m_CenterVTA->setObjectName(QString::fromUtf8("m_CenterVTA"));
    m_CenterVTA->setGeometry(QRect(10, 40, 56, 19));
    m_BottomVTA = new QRadioButton(groupBox);
    m_BottomVTA->setObjectName(QString::fromUtf8("m_BottomVTA"));
    m_BottomVTA->setGeometry(QRect(10, 60, 56, 19));
    groupBox_2 = new QGroupBox(uorTextLayotProp);
    groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
    groupBox_2->setGeometry(QRect(116, 11, 81, 91));
    QSizePolicy sizePolicy1(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(groupBox_2->sizePolicy().hasHeightForWidth());
    groupBox_2->setSizePolicy(sizePolicy1);
    m_LeftHTA = new QRadioButton(groupBox_2);
    m_LeftHTA->setObjectName(QString::fromUtf8("m_LeftHTA"));
    m_LeftHTA->setGeometry(QRect(10, 20, 41, 19));
    m_CenterHTA = new QRadioButton(groupBox_2);
    m_CenterHTA->setObjectName(QString::fromUtf8("m_CenterHTA"));
    m_CenterHTA->setGeometry(QRect(10, 40, 61, 19));
    m_RightHTA = new QRadioButton(groupBox_2);
    m_RightHTA->setObjectName(QString::fromUtf8("m_RightHTA"));
    m_RightHTA->setGeometry(QRect(10, 60, 51, 19));
    chbUseSelectedCell = new QCheckBox(uorTextLayotProp);
    chbUseSelectedCell->setObjectName(QString::fromUtf8("chbUseSelectedCell"));
    chbUseSelectedCell->setGeometry(QRect(200, 20, 111, 19));

    retranslateUi(uorTextLayotProp);

    QMetaObject::connectSlotsByName(uorTextLayotProp);
    } // setupUi

    void retranslateUi(QWidget *uorTextLayotProp)
    {
    uorTextLayotProp->setWindowTitle(QApplication::translate("uorTextLayotProp", "Form", 0, QApplication::UnicodeUTF8));
    groupBox->setTitle(QApplication::translate("uorTextLayotProp", "Vertical", 0, QApplication::UnicodeUTF8));
    m_TopVTA->setText(QApplication::translate("uorTextLayotProp", "Top", 0, QApplication::UnicodeUTF8));
    m_CenterVTA->setText(QApplication::translate("uorTextLayotProp", "Midle", 0, QApplication::UnicodeUTF8));
    m_BottomVTA->setText(QApplication::translate("uorTextLayotProp", "Bottom", 0, QApplication::UnicodeUTF8));
    groupBox_2->setTitle(QApplication::translate("uorTextLayotProp", "Horizontal", 0, QApplication::UnicodeUTF8));
    m_LeftHTA->setText(QApplication::translate("uorTextLayotProp", "Left", 0, QApplication::UnicodeUTF8));
    m_CenterHTA->setText(QApplication::translate("uorTextLayotProp", "Center", 0, QApplication::UnicodeUTF8));
    m_RightHTA->setText(QApplication::translate("uorTextLayotProp", "Right", 0, QApplication::UnicodeUTF8));
    chbUseSelectedCell->setText(QApplication::translate("uorTextLayotProp", "Use selected cell", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(uorTextLayotProp);
    } // retranslateUi

};

namespace Ui {
    class uorTextLayotProp: public Ui_uorTextLayotProp {};
} // namespace Ui

#endif // UI_UORTEXTLAYOTPROP_H
