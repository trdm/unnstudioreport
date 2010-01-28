/********************************************************************************
** Form generated from reading ui file 'uorPageSetupDlg.ui'
**
** Created: Sun 13. Dec 18:52:40 2009
**      by: Qt User Interface Compiler version 4.3.4
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_UORPAGESETUPDLG_H
#define UI_UORPAGESETUPDLG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QRadioButton>

class Ui_uorPageSetup
{
public:
    QDialogButtonBox *buttonBox;
    QGroupBox *groupBox;
    QComboBox *m_cbPaperSize;
    QLabel *m_llPaperSize;
    QGroupBox *groupBox_2;
    QRadioButton *m_rbPortrait;
    QRadioButton *m_rbLandscape;
    QGroupBox *groupBox_3;
    QLabel *m_llFieldsLeft;
    QLineEdit *m_leFieldsLeft;
    QLineEdit *m_leFieldsTop;
    QLabel *m_llFieldsTop;
    QLineEdit *m_leFieldsBottom;
    QLabel *m_llFieldsRight;
    QLabel *m_llFieldsBottom;
    QLineEdit *m_leFieldsRight;
    QGroupBox *groupBox_4;
    QLabel *m_llPageViewP;
    QLabel *m_llPageViewL;
    QGroupBox *groupBox_5;
    QLineEdit *m_leRTitleBottom;
    QLineEdit *m_leRTitleTop;
    QLabel *m_llColontitleBottom;
    QLabel *m_llColontitleTop;
    QGroupBox *groupBox_6;
    QRadioButton *m_rbScalePunctual;
    QLineEdit *m_leScalePunctual;
    QRadioButton *m_rbScalePunctual2;

    void setupUi(QDialog *uorPageSetup)
    {
    if (uorPageSetup->objectName().isEmpty())
        uorPageSetup->setObjectName(QString::fromUtf8("uorPageSetup"));
    uorPageSetup->resize(385, 279);
    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(uorPageSetup->sizePolicy().hasHeightForWidth());
    uorPageSetup->setSizePolicy(sizePolicy);
    uorPageSetup->setMinimumSize(QSize(385, 279));
    uorPageSetup->setMaximumSize(QSize(385, 279));
    buttonBox = new QDialogButtonBox(uorPageSetup);
    buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
    buttonBox->setGeometry(QRect(212, 245, 167, 32));
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::NoButton|QDialogButtonBox::Ok);
    groupBox = new QGroupBox(uorPageSetup);
    groupBox->setObjectName(QString::fromUtf8("groupBox"));
    groupBox->setGeometry(QRect(10, 10, 230, 50));
    m_cbPaperSize = new QComboBox(groupBox);
    m_cbPaperSize->setObjectName(QString::fromUtf8("m_cbPaperSize"));
    m_cbPaperSize->setGeometry(QRect(72, 20, 150, 20));
    m_llPaperSize = new QLabel(groupBox);
    m_llPaperSize->setObjectName(QString::fromUtf8("m_llPaperSize"));
    m_llPaperSize->setGeometry(QRect(10, 20, 60, 16));
    groupBox_2 = new QGroupBox(uorPageSetup);
    groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
    groupBox_2->setGeometry(QRect(10, 60, 230, 50));
    m_rbPortrait = new QRadioButton(groupBox_2);
    m_rbPortrait->setObjectName(QString::fromUtf8("m_rbPortrait"));
    m_rbPortrait->setGeometry(QRect(10, 20, 81, 19));
    m_rbLandscape = new QRadioButton(groupBox_2);
    m_rbLandscape->setObjectName(QString::fromUtf8("m_rbLandscape"));
    m_rbLandscape->setGeometry(QRect(124, 20, 81, 19));
    groupBox_3 = new QGroupBox(uorPageSetup);
    groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
    groupBox_3->setGeometry(QRect(10, 110, 230, 80));
    m_llFieldsLeft = new QLabel(groupBox_3);
    m_llFieldsLeft->setObjectName(QString::fromUtf8("m_llFieldsLeft"));
    m_llFieldsLeft->setGeometry(QRect(10, 20, 46, 16));
    m_llFieldsLeft->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    m_leFieldsLeft = new QLineEdit(groupBox_3);
    m_leFieldsLeft->setObjectName(QString::fromUtf8("m_leFieldsLeft"));
    m_leFieldsLeft->setGeometry(QRect(60, 19, 40, 20));
    m_leFieldsLeft->setCursorPosition(0);
    m_leFieldsLeft->setAlignment(Qt::AlignCenter);
    m_leFieldsTop = new QLineEdit(groupBox_3);
    m_leFieldsTop->setObjectName(QString::fromUtf8("m_leFieldsTop"));
    m_leFieldsTop->setGeometry(QRect(60, 49, 40, 20));
    m_leFieldsTop->setFrame(true);
    m_leFieldsTop->setCursorPosition(0);
    m_leFieldsTop->setAlignment(Qt::AlignCenter);
    m_llFieldsTop = new QLabel(groupBox_3);
    m_llFieldsTop->setObjectName(QString::fromUtf8("m_llFieldsTop"));
    m_llFieldsTop->setGeometry(QRect(11, 50, 46, 16));
    m_llFieldsTop->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    m_leFieldsBottom = new QLineEdit(groupBox_3);
    m_leFieldsBottom->setObjectName(QString::fromUtf8("m_leFieldsBottom"));
    m_leFieldsBottom->setGeometry(QRect(169, 49, 40, 20));
    m_leFieldsBottom->setFrame(true);
    m_leFieldsBottom->setCursorPosition(0);
    m_leFieldsBottom->setAlignment(Qt::AlignCenter);
    m_llFieldsRight = new QLabel(groupBox_3);
    m_llFieldsRight->setObjectName(QString::fromUtf8("m_llFieldsRight"));
    m_llFieldsRight->setGeometry(QRect(110, 20, 50, 16));
    m_llFieldsRight->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    m_llFieldsBottom = new QLabel(groupBox_3);
    m_llFieldsBottom->setObjectName(QString::fromUtf8("m_llFieldsBottom"));
    m_llFieldsBottom->setGeometry(QRect(110, 50, 50, 16));
    m_llFieldsBottom->setTextFormat(Qt::RichText);
    m_llFieldsBottom->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    m_leFieldsRight = new QLineEdit(groupBox_3);
    m_leFieldsRight->setObjectName(QString::fromUtf8("m_leFieldsRight"));
    m_leFieldsRight->setGeometry(QRect(170, 19, 40, 20));
    m_leFieldsRight->setCursorPosition(0);
    m_leFieldsRight->setAlignment(Qt::AlignCenter);
    groupBox_4 = new QGroupBox(uorPageSetup);
    groupBox_4->setObjectName(QString::fromUtf8("groupBox_4"));
    groupBox_4->setGeometry(QRect(250, 11, 130, 138));
    m_llPageViewP = new QLabel(groupBox_4);
    m_llPageViewP->setObjectName(QString::fromUtf8("m_llPageViewP"));
    m_llPageViewP->setGeometry(QRect(10, 20, 110, 110));
    m_llPageViewP->setFrameShape(QFrame::NoFrame);
    m_llPageViewP->setFrameShadow(QFrame::Plain);
    m_llPageViewP->setPixmap(QPixmap(QString::fromUtf8(":/image/portrait.png")));
    m_llPageViewP->setAlignment(Qt::AlignHCenter|Qt::AlignTop);
    m_llPageViewL = new QLabel(groupBox_4);
    m_llPageViewL->setObjectName(QString::fromUtf8("m_llPageViewL"));
    m_llPageViewL->setGeometry(QRect(4, 30, 120, 90));
    m_llPageViewL->setFrameShape(QFrame::NoFrame);
    m_llPageViewL->setFrameShadow(QFrame::Plain);
    m_llPageViewL->setPixmap(QPixmap(QString::fromUtf8(":/image/landscape.png")));
    m_llPageViewL->setAlignment(Qt::AlignHCenter|Qt::AlignTop);
    groupBox_5 = new QGroupBox(uorPageSetup);
    groupBox_5->setObjectName(QString::fromUtf8("groupBox_5"));
    groupBox_5->setGeometry(QRect(10, 190, 230, 50));
    m_leRTitleBottom = new QLineEdit(groupBox_5);
    m_leRTitleBottom->setObjectName(QString::fromUtf8("m_leRTitleBottom"));
    m_leRTitleBottom->setEnabled(false);
    m_leRTitleBottom->setGeometry(QRect(170, 18, 40, 20));
    m_leRTitleBottom->setFrame(true);
    m_leRTitleBottom->setCursorPosition(0);
    m_leRTitleBottom->setAlignment(Qt::AlignCenter);
    m_leRTitleTop = new QLineEdit(groupBox_5);
    m_leRTitleTop->setObjectName(QString::fromUtf8("m_leRTitleTop"));
    m_leRTitleTop->setEnabled(false);
    m_leRTitleTop->setGeometry(QRect(61, 18, 40, 20));
    m_leRTitleTop->setFrame(true);
    m_leRTitleTop->setCursorPosition(0);
    m_leRTitleTop->setAlignment(Qt::AlignCenter);
    m_llColontitleBottom = new QLabel(groupBox_5);
    m_llColontitleBottom->setObjectName(QString::fromUtf8("m_llColontitleBottom"));
    m_llColontitleBottom->setGeometry(QRect(111, 19, 50, 16));
    m_llColontitleBottom->setTextFormat(Qt::RichText);
    m_llColontitleBottom->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    m_llColontitleTop = new QLabel(groupBox_5);
    m_llColontitleTop->setObjectName(QString::fromUtf8("m_llColontitleTop"));
    m_llColontitleTop->setGeometry(QRect(11, 20, 46, 16));
    m_llColontitleTop->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    groupBox_6 = new QGroupBox(uorPageSetup);
    groupBox_6->setObjectName(QString::fromUtf8("groupBox_6"));
    groupBox_6->setGeometry(QRect(250, 160, 130, 80));
    m_rbScalePunctual = new QRadioButton(groupBox_6);
    m_rbScalePunctual->setObjectName(QString::fromUtf8("m_rbScalePunctual"));
    m_rbScalePunctual->setEnabled(true);
    m_rbScalePunctual->setGeometry(QRect(10, 20, 60, 19));
    m_leScalePunctual = new QLineEdit(groupBox_6);
    m_leScalePunctual->setObjectName(QString::fromUtf8("m_leScalePunctual"));
    m_leScalePunctual->setEnabled(false);
    m_leScalePunctual->setGeometry(QRect(80, 20, 40, 20));
    m_leScalePunctual->setEchoMode(QLineEdit::Normal);
    m_leScalePunctual->setCursorPosition(0);
    m_leScalePunctual->setAlignment(Qt::AlignCenter);
    m_rbScalePunctual2 = new QRadioButton(groupBox_6);
    m_rbScalePunctual2->setObjectName(QString::fromUtf8("m_rbScalePunctual2"));
    m_rbScalePunctual2->setEnabled(true);
    m_rbScalePunctual2->setGeometry(QRect(10, 40, 114, 19));
    QWidget::setTabOrder(m_cbPaperSize, m_rbPortrait);
    QWidget::setTabOrder(m_rbPortrait, m_rbLandscape);
    QWidget::setTabOrder(m_rbLandscape, m_leFieldsLeft);
    QWidget::setTabOrder(m_leFieldsLeft, m_leFieldsTop);
    QWidget::setTabOrder(m_leFieldsTop, m_leFieldsRight);
    QWidget::setTabOrder(m_leFieldsRight, m_leFieldsBottom);
    QWidget::setTabOrder(m_leFieldsBottom, m_leRTitleTop);
    QWidget::setTabOrder(m_leRTitleTop, m_leRTitleBottom);
    QWidget::setTabOrder(m_leRTitleBottom, m_rbScalePunctual);
    QWidget::setTabOrder(m_rbScalePunctual, m_leScalePunctual);
    QWidget::setTabOrder(m_leScalePunctual, m_rbScalePunctual2);
    QWidget::setTabOrder(m_rbScalePunctual2, buttonBox);

    retranslateUi(uorPageSetup);
    QObject::connect(buttonBox, SIGNAL(accepted()), uorPageSetup, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), uorPageSetup, SLOT(reject()));

    QMetaObject::connectSlotsByName(uorPageSetup);
    } // setupUi

    void retranslateUi(QDialog *uorPageSetup)
    {
    uorPageSetup->setWindowTitle(QApplication::translate("uorPageSetup", "Dialog", 0, QApplication::UnicodeUTF8));
    groupBox->setTitle(QApplication::translate("uorPageSetup", "Paper", 0, QApplication::UnicodeUTF8));
    m_llPaperSize->setText(QApplication::translate("uorPageSetup", "Paper size", 0, QApplication::UnicodeUTF8));
    groupBox_2->setTitle(QApplication::translate("uorPageSetup", "Orientation", 0, QApplication::UnicodeUTF8));
    m_rbPortrait->setText(QApplication::translate("uorPageSetup", "Portrait", 0, QApplication::UnicodeUTF8));
    m_rbLandscape->setText(QApplication::translate("uorPageSetup", "landscape", 0, QApplication::UnicodeUTF8));
    groupBox_3->setTitle(QApplication::translate("uorPageSetup", "Fields (mm.)", 0, QApplication::UnicodeUTF8));
    m_llFieldsLeft->setText(QApplication::translate("uorPageSetup", "Left", 0, QApplication::UnicodeUTF8));
    m_leFieldsLeft->setInputMask(QApplication::translate("uorPageSetup", "99; ", 0, QApplication::UnicodeUTF8));
    m_leFieldsLeft->setText(QApplication::translate("uorPageSetup", "10", 0, QApplication::UnicodeUTF8));
    m_leFieldsTop->setInputMask(QApplication::translate("uorPageSetup", "99; ", 0, QApplication::UnicodeUTF8));
    m_leFieldsTop->setText(QApplication::translate("uorPageSetup", "10", 0, QApplication::UnicodeUTF8));
    m_llFieldsTop->setText(QApplication::translate("uorPageSetup", "Top", 0, QApplication::UnicodeUTF8));
    m_leFieldsBottom->setInputMask(QApplication::translate("uorPageSetup", "99; ", 0, QApplication::UnicodeUTF8));
    m_leFieldsBottom->setText(QApplication::translate("uorPageSetup", "10", 0, QApplication::UnicodeUTF8));
    m_llFieldsRight->setText(QApplication::translate("uorPageSetup", "Right", 0, QApplication::UnicodeUTF8));
    m_llFieldsBottom->setText(QApplication::translate("uorPageSetup", "Bottom", 0, QApplication::UnicodeUTF8));
    m_leFieldsRight->setInputMask(QApplication::translate("uorPageSetup", "99; ", 0, QApplication::UnicodeUTF8));
    m_leFieldsRight->setText(QApplication::translate("uorPageSetup", "10", 0, QApplication::UnicodeUTF8));
    groupBox_4->setTitle(QApplication::translate("uorPageSetup", "View", 0, QApplication::UnicodeUTF8));
    m_llPageViewP->setText(QString());
    m_llPageViewL->setText(QString());
    groupBox_5->setTitle(QApplication::translate("uorPageSetup", "Running title (mm.)", 0, QApplication::UnicodeUTF8));
    m_leRTitleBottom->setInputMask(QApplication::translate("uorPageSetup", "99; ", 0, QApplication::UnicodeUTF8));
    m_leRTitleBottom->setText(QApplication::translate("uorPageSetup", "10", 0, QApplication::UnicodeUTF8));
    m_leRTitleTop->setInputMask(QApplication::translate("uorPageSetup", "99; ", 0, QApplication::UnicodeUTF8));
    m_leRTitleTop->setText(QApplication::translate("uorPageSetup", "10", 0, QApplication::UnicodeUTF8));
    m_llColontitleBottom->setText(QApplication::translate("uorPageSetup", "Bottom", 0, QApplication::UnicodeUTF8));
    m_llColontitleTop->setText(QApplication::translate("uorPageSetup", "Top", 0, QApplication::UnicodeUTF8));
    groupBox_6->setTitle(QApplication::translate("uorPageSetup", "Scale", 0, QApplication::UnicodeUTF8));
    m_rbScalePunctual->setText(QApplication::translate("uorPageSetup", "Punctual", 0, QApplication::UnicodeUTF8));
    m_leScalePunctual->setInputMask(QApplication::translate("uorPageSetup", "999; ", 0, QApplication::UnicodeUTF8));
    m_leScalePunctual->setText(QApplication::translate("uorPageSetup", "100", 0, QApplication::UnicodeUTF8));
    m_rbScalePunctual2->setText(QApplication::translate("uorPageSetup", "width of the sheet", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(uorPageSetup);
    } // retranslateUi

};

namespace Ui {
    class uorPageSetup: public Ui_uorPageSetup {};
} // namespace Ui

#endif // UI_UORPAGESETUPDLG_H
