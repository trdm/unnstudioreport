/********************************************************************************
** Form generated from reading ui file 'uorTextFontProp.ui'
**
** Created: Sun 13. Dec 18:52:40 2009
**      by: Qt User Interface Compiler version 4.3.4
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_UORTEXTFONTPROP_H
#define UI_UORTEXTFONTPROP_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QListWidget>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

class Ui_uorTextFontProp
{
public:
    QGridLayout *gridLayout;
    QGridLayout *gridLayout1;
    QVBoxLayout *vboxLayout;
    QHBoxLayout *hboxLayout;
    QLabel *label_2;
    QLineEdit *m_fontName;
    QListWidget *m_listFonts;
    QVBoxLayout *vboxLayout1;
    QHBoxLayout *hboxLayout1;
    QLabel *label_3;
    QLineEdit *m_size;
    QListWidget *m_sizeList;
    QVBoxLayout *vboxLayout2;
    QCheckBox *m_chbItalic;
    QCheckBox *m_chbUnderline;
    QCheckBox *m_chbBold;
    QLabel *labelColor;
    QFrame *m_colFrm;
    QSpacerItem *spacerItem;

    void setupUi(QWidget *uorTextFontProp)
    {
    if (uorTextFontProp->objectName().isEmpty())
        uorTextFontProp->setObjectName(QString::fromUtf8("uorTextFontProp"));
    uorTextFontProp->resize(360, 113);
    QSizePolicy sizePolicy(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(1));
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(uorTextFontProp->sizePolicy().hasHeightForWidth());
    uorTextFontProp->setSizePolicy(sizePolicy);
    uorTextFontProp->setMinimumSize(QSize(360, 102));
    uorTextFontProp->setMaximumSize(QSize(16777215, 16777215));
    uorTextFontProp->setSizeIncrement(QSize(0, 0));
    gridLayout = new QGridLayout(uorTextFontProp);
    gridLayout->setSpacing(2);
    gridLayout->setMargin(2);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    gridLayout1 = new QGridLayout();
    gridLayout1->setSpacing(2);
    gridLayout1->setMargin(2);
    gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
    vboxLayout = new QVBoxLayout();
    vboxLayout->setSpacing(2);
    vboxLayout->setMargin(2);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    hboxLayout = new QHBoxLayout();
    hboxLayout->setSpacing(2);
    hboxLayout->setMargin(2);
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    label_2 = new QLabel(uorTextFontProp);
    label_2->setObjectName(QString::fromUtf8("label_2"));
    QSizePolicy sizePolicy1(static_cast<QSizePolicy::Policy>(0), static_cast<QSizePolicy::Policy>(0));
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(label_2->sizePolicy().hasHeightForWidth());
    label_2->setSizePolicy(sizePolicy1);

    hboxLayout->addWidget(label_2);

    m_fontName = new QLineEdit(uorTextFontProp);
    m_fontName->setObjectName(QString::fromUtf8("m_fontName"));
    QSizePolicy sizePolicy2(static_cast<QSizePolicy::Policy>(7), static_cast<QSizePolicy::Policy>(0));
    sizePolicy2.setHorizontalStretch(0);
    sizePolicy2.setVerticalStretch(0);
    sizePolicy2.setHeightForWidth(m_fontName->sizePolicy().hasHeightForWidth());
    m_fontName->setSizePolicy(sizePolicy2);
    m_fontName->setMinimumSize(QSize(130, 20));
    m_fontName->setMaximumSize(QSize(16777215, 20));

    hboxLayout->addWidget(m_fontName);


    vboxLayout->addLayout(hboxLayout);

    m_listFonts = new QListWidget(uorTextFontProp);
    m_listFonts->setObjectName(QString::fromUtf8("m_listFonts"));
    sizePolicy.setHeightForWidth(m_listFonts->sizePolicy().hasHeightForWidth());
    m_listFonts->setSizePolicy(sizePolicy);
    m_listFonts->setMinimumSize(QSize(150, 50));
    m_listFonts->setSizeIncrement(QSize(0, 0));
    m_listFonts->setBaseSize(QSize(160, 60));
    m_listFonts->setSortingEnabled(true);

    vboxLayout->addWidget(m_listFonts);


    gridLayout1->addLayout(vboxLayout, 0, 0, 1, 1);

    vboxLayout1 = new QVBoxLayout();
    vboxLayout1->setSpacing(2);
    vboxLayout1->setMargin(2);
    vboxLayout1->setObjectName(QString::fromUtf8("vboxLayout1"));
    hboxLayout1 = new QHBoxLayout();
#ifndef Q_OS_MAC
    hboxLayout1->setSpacing(6);
#endif
    hboxLayout1->setMargin(2);
    hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
    label_3 = new QLabel(uorTextFontProp);
    label_3->setObjectName(QString::fromUtf8("label_3"));
    sizePolicy1.setHeightForWidth(label_3->sizePolicy().hasHeightForWidth());
    label_3->setSizePolicy(sizePolicy1);
    label_3->setOpenExternalLinks(false);

    hboxLayout1->addWidget(label_3);

    m_size = new QLineEdit(uorTextFontProp);
    m_size->setObjectName(QString::fromUtf8("m_size"));
    sizePolicy1.setHeightForWidth(m_size->sizePolicy().hasHeightForWidth());
    m_size->setSizePolicy(sizePolicy1);
    m_size->setMinimumSize(QSize(40, 0));
    m_size->setMaximumSize(QSize(60, 20));

    hboxLayout1->addWidget(m_size);


    vboxLayout1->addLayout(hboxLayout1);

    m_sizeList = new QListWidget(uorTextFontProp);
    m_sizeList->setObjectName(QString::fromUtf8("m_sizeList"));
    QSizePolicy sizePolicy3(static_cast<QSizePolicy::Policy>(0), static_cast<QSizePolicy::Policy>(1));
    sizePolicy3.setHorizontalStretch(0);
    sizePolicy3.setVerticalStretch(0);
    sizePolicy3.setHeightForWidth(m_sizeList->sizePolicy().hasHeightForWidth());
    m_sizeList->setSizePolicy(sizePolicy3);
    m_sizeList->setMinimumSize(QSize(50, 50));
    m_sizeList->setMaximumSize(QSize(80, 16777215));
    m_sizeList->setSizeIncrement(QSize(0, 1));
    m_sizeList->setBaseSize(QSize(50, 60));

    vboxLayout1->addWidget(m_sizeList);


    gridLayout1->addLayout(vboxLayout1, 0, 1, 1, 1);

    vboxLayout2 = new QVBoxLayout();
    vboxLayout2->setSpacing(2);
    vboxLayout2->setMargin(2);
    vboxLayout2->setObjectName(QString::fromUtf8("vboxLayout2"));
    m_chbItalic = new QCheckBox(uorTextFontProp);
    m_chbItalic->setObjectName(QString::fromUtf8("m_chbItalic"));
    sizePolicy1.setHeightForWidth(m_chbItalic->sizePolicy().hasHeightForWidth());
    m_chbItalic->setSizePolicy(sizePolicy1);

    vboxLayout2->addWidget(m_chbItalic);

    m_chbUnderline = new QCheckBox(uorTextFontProp);
    m_chbUnderline->setObjectName(QString::fromUtf8("m_chbUnderline"));
    sizePolicy1.setHeightForWidth(m_chbUnderline->sizePolicy().hasHeightForWidth());
    m_chbUnderline->setSizePolicy(sizePolicy1);

    vboxLayout2->addWidget(m_chbUnderline);

    m_chbBold = new QCheckBox(uorTextFontProp);
    m_chbBold->setObjectName(QString::fromUtf8("m_chbBold"));
    sizePolicy1.setHeightForWidth(m_chbBold->sizePolicy().hasHeightForWidth());
    m_chbBold->setSizePolicy(sizePolicy1);

    vboxLayout2->addWidget(m_chbBold);

    labelColor = new QLabel(uorTextFontProp);
    labelColor->setObjectName(QString::fromUtf8("labelColor"));
    sizePolicy1.setHeightForWidth(labelColor->sizePolicy().hasHeightForWidth());
    labelColor->setSizePolicy(sizePolicy1);

    vboxLayout2->addWidget(labelColor);

    m_colFrm = new QFrame(uorTextFontProp);
    m_colFrm->setObjectName(QString::fromUtf8("m_colFrm"));
    sizePolicy.setHeightForWidth(m_colFrm->sizePolicy().hasHeightForWidth());
    m_colFrm->setSizePolicy(sizePolicy);
    m_colFrm->setMinimumSize(QSize(70, 20));
    m_colFrm->setFrameShape(QFrame::NoFrame);
    m_colFrm->setFrameShadow(QFrame::Plain);
    m_colFrm->setLineWidth(0);

    vboxLayout2->addWidget(m_colFrm);

    spacerItem = new QSpacerItem(20, 2, QSizePolicy::Minimum, QSizePolicy::Expanding);

    vboxLayout2->addItem(spacerItem);


    gridLayout1->addLayout(vboxLayout2, 0, 2, 1, 1);


    gridLayout->addLayout(gridLayout1, 0, 0, 1, 1);

    QWidget::setTabOrder(m_fontName, m_listFonts);
    QWidget::setTabOrder(m_listFonts, m_size);
    QWidget::setTabOrder(m_size, m_sizeList);
    QWidget::setTabOrder(m_sizeList, m_chbItalic);
    QWidget::setTabOrder(m_chbItalic, m_chbUnderline);
    QWidget::setTabOrder(m_chbUnderline, m_chbBold);

    retranslateUi(uorTextFontProp);

    QMetaObject::connectSlotsByName(uorTextFontProp);
    } // setupUi

    void retranslateUi(QWidget *uorTextFontProp)
    {
    uorTextFontProp->setWindowTitle(QApplication::translate("uorTextFontProp", "Form", 0, QApplication::UnicodeUTF8));
    label_2->setText(QApplication::translate("uorTextFontProp", "Font", 0, QApplication::UnicodeUTF8));
    m_listFonts->setStyleSheet(QApplication::translate("uorTextFontProp", "QListWidget { show-decoration-selected: 1;}", 0, QApplication::UnicodeUTF8));
    label_3->setText(QApplication::translate("uorTextFontProp", "Size", 0, QApplication::UnicodeUTF8));
    m_sizeList->setStyleSheet(QApplication::translate("uorTextFontProp", "QListWidget { show-decoration-selected: 1;}", 0, QApplication::UnicodeUTF8));
    m_chbItalic->setText(QApplication::translate("uorTextFontProp", "Italic", 0, QApplication::UnicodeUTF8));
    m_chbUnderline->setText(QApplication::translate("uorTextFontProp", "Underline", 0, QApplication::UnicodeUTF8));
    m_chbBold->setText(QApplication::translate("uorTextFontProp", "Bold", 0, QApplication::UnicodeUTF8));
    labelColor->setText(QApplication::translate("uorTextFontProp", "Color", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(uorTextFontProp);
    } // retranslateUi

};

namespace Ui {
    class uorTextFontProp: public Ui_uorTextFontProp {};
} // namespace Ui

#endif // UI_UORTEXTFONTPROP_H
