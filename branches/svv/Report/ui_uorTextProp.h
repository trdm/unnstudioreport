/********************************************************************************
** Form generated from reading ui file 'uorTextProp.ui'
**
** Created: Sun 13. Dec 18:52:41 2009
**      by: Qt User Interface Compiler version 4.3.4
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_UORTEXTPROP_H
#define UI_UORTEXTPROP_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

class Ui_uorTextProp
{
public:
    QGridLayout *gridLayout;
    QVBoxLayout *vboxLayout;
    QHBoxLayout *hboxLayout;
    QLabel *label;
    QComboBox *m_cbTextType;
    QLabel *label_2;
    QComboBox *m_cbTextBehavior;
    QLabel *label_3;
    QTextEdit *m_textCell;
    QHBoxLayout *hboxLayout1;
    QLabel *label_4;
    QLineEdit *m_textDecoding;

    void setupUi(QWidget *uorTextProp)
    {
    if (uorTextProp->objectName().isEmpty())
        uorTextProp->setObjectName(QString::fromUtf8("uorTextProp"));
    uorTextProp->resize(296, 127);
    uorTextProp->setWindowIcon(QIcon());
    gridLayout = new QGridLayout(uorTextProp);
    gridLayout->setSpacing(1);
    gridLayout->setMargin(1);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    vboxLayout = new QVBoxLayout();
    vboxLayout->setSpacing(2);
    vboxLayout->setMargin(2);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    hboxLayout = new QHBoxLayout();
    hboxLayout->setSpacing(2);
    hboxLayout->setMargin(2);
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    label = new QLabel(uorTextProp);
    label->setObjectName(QString::fromUtf8("label"));
    QSizePolicy sizePolicy(static_cast<QSizePolicy::Policy>(0), static_cast<QSizePolicy::Policy>(0));
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
    label->setSizePolicy(sizePolicy);

    hboxLayout->addWidget(label);

    m_cbTextType = new QComboBox(uorTextProp);
    m_cbTextType->setObjectName(QString::fromUtf8("m_cbTextType"));
    sizePolicy.setHeightForWidth(m_cbTextType->sizePolicy().hasHeightForWidth());
    m_cbTextType->setSizePolicy(sizePolicy);
    m_cbTextType->setMinimumSize(QSize(110, 20));

    hboxLayout->addWidget(m_cbTextType);

    label_2 = new QLabel(uorTextProp);
    label_2->setObjectName(QString::fromUtf8("label_2"));
    sizePolicy.setHeightForWidth(label_2->sizePolicy().hasHeightForWidth());
    label_2->setSizePolicy(sizePolicy);

    hboxLayout->addWidget(label_2);

    m_cbTextBehavior = new QComboBox(uorTextProp);
    m_cbTextBehavior->setObjectName(QString::fromUtf8("m_cbTextBehavior"));
    QSizePolicy sizePolicy1(static_cast<QSizePolicy::Policy>(7), static_cast<QSizePolicy::Policy>(0));
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(m_cbTextBehavior->sizePolicy().hasHeightForWidth());
    m_cbTextBehavior->setSizePolicy(sizePolicy1);
    m_cbTextBehavior->setMinimumSize(QSize(60, 20));

    hboxLayout->addWidget(m_cbTextBehavior);


    vboxLayout->addLayout(hboxLayout);

    label_3 = new QLabel(uorTextProp);
    label_3->setObjectName(QString::fromUtf8("label_3"));
    QSizePolicy sizePolicy2(static_cast<QSizePolicy::Policy>(0), static_cast<QSizePolicy::Policy>(4));
    sizePolicy2.setHorizontalStretch(0);
    sizePolicy2.setVerticalStretch(0);
    sizePolicy2.setHeightForWidth(label_3->sizePolicy().hasHeightForWidth());
    label_3->setSizePolicy(sizePolicy2);

    vboxLayout->addWidget(label_3);

    m_textCell = new QTextEdit(uorTextProp);
    m_textCell->setObjectName(QString::fromUtf8("m_textCell"));
    QSizePolicy sizePolicy3(static_cast<QSizePolicy::Policy>(7), static_cast<QSizePolicy::Policy>(7));
    sizePolicy3.setHorizontalStretch(1);
    sizePolicy3.setVerticalStretch(1);
    sizePolicy3.setHeightForWidth(m_textCell->sizePolicy().hasHeightForWidth());
    m_textCell->setSizePolicy(sizePolicy3);
    m_textCell->setMinimumSize(QSize(270, 50));
    m_textCell->setMaximumSize(QSize(16777215, 16777215));
    m_textCell->setSizeIncrement(QSize(1, 1));

    vboxLayout->addWidget(m_textCell);

    hboxLayout1 = new QHBoxLayout();
    hboxLayout1->setSpacing(2);
    hboxLayout1->setMargin(2);
    hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
    label_4 = new QLabel(uorTextProp);
    label_4->setObjectName(QString::fromUtf8("label_4"));
    sizePolicy.setHeightForWidth(label_4->sizePolicy().hasHeightForWidth());
    label_4->setSizePolicy(sizePolicy);

    hboxLayout1->addWidget(label_4);

    m_textDecoding = new QLineEdit(uorTextProp);
    m_textDecoding->setObjectName(QString::fromUtf8("m_textDecoding"));
    QSizePolicy sizePolicy4(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(7));
    sizePolicy4.setHorizontalStretch(0);
    sizePolicy4.setVerticalStretch(0);
    sizePolicy4.setHeightForWidth(m_textDecoding->sizePolicy().hasHeightForWidth());
    m_textDecoding->setSizePolicy(sizePolicy4);
    m_textDecoding->setMinimumSize(QSize(180, 20));
    m_textDecoding->setMaximumSize(QSize(16777215, 16777215));
    m_textDecoding->setBaseSize(QSize(240, 20));

    hboxLayout1->addWidget(m_textDecoding);


    vboxLayout->addLayout(hboxLayout1);


    gridLayout->addLayout(vboxLayout, 0, 0, 1, 1);


    retranslateUi(uorTextProp);

    QMetaObject::connectSlotsByName(uorTextProp);
    } // setupUi

    void retranslateUi(QWidget *uorTextProp)
    {
    uorTextProp->setWindowTitle(QApplication::translate("uorTextProp", "Form", 0, QApplication::UnicodeUTF8));
    label->setText(QApplication::translate("uorTextProp", "Type", 0, QApplication::UnicodeUTF8));
    label_2->setText(QApplication::translate("uorTextProp", "Text behavior", 0, QApplication::UnicodeUTF8));
    label_3->setText(QApplication::translate("uorTextProp", "Text", 0, QApplication::UnicodeUTF8));
    label_4->setText(QApplication::translate("uorTextProp", "Decoding", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(uorTextProp);
    } // retranslateUi

};

namespace Ui {
    class uorTextProp: public Ui_uorTextProp {};
} // namespace Ui

#endif // UI_UORTEXTPROP_H
