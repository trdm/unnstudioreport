/********************************************************************************
** Form generated from reading ui file 'uorOptions.ui'
**
** Created: Tue 12. Jan 02:14:58 2010
**      by: Qt User Interface Compiler version 4.3.4
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_UOROPTIONS_H
#define UI_UOROPTIONS_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QFrame>
#include <QtGui/QLabel>
#include <QtGui/QTextEdit>

class Ui_uorOptionsDlgUi
{
public:
    QLabel *label;
    QCheckBox *m_saveWithSelection;
    QDialogButtonBox *buttonBox;
    QFrame *line;
    QTextEdit *m_DebugInfoText;
    QLabel *m_DebugInfoLabel;
    QCheckBox *m_saveWithUndoStack;
    QCheckBox *m_directDrawFlag;

    void setupUi(QDialog *uorOptionsDlgUi)
    {
    if (uorOptionsDlgUi->objectName().isEmpty())
        uorOptionsDlgUi->setObjectName(QString::fromUtf8("uorOptionsDlgUi"));
    uorOptionsDlgUi->setWindowModality(Qt::WindowModal);
    uorOptionsDlgUi->resize(400, 252);
    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(uorOptionsDlgUi->sizePolicy().hasHeightForWidth());
    uorOptionsDlgUi->setSizePolicy(sizePolicy);
    uorOptionsDlgUi->setWindowIcon(QIcon(QString::fromUtf8(":/image/report.png")));
    label = new QLabel(uorOptionsDlgUi);
    label->setObjectName(QString::fromUtf8("label"));
    label->setGeometry(QRect(10, 10, 91, 16));
    m_saveWithSelection = new QCheckBox(uorOptionsDlgUi);
    m_saveWithSelection->setObjectName(QString::fromUtf8("m_saveWithSelection"));
    m_saveWithSelection->setGeometry(QRect(10, 40, 151, 19));
    buttonBox = new QDialogButtonBox(uorOptionsDlgUi);
    buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
    buttonBox->setGeometry(QRect(310, 10, 81, 61));
    buttonBox->setOrientation(Qt::Vertical);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::NoButton|QDialogButtonBox::Ok);
    line = new QFrame(uorOptionsDlgUi);
    line->setObjectName(QString::fromUtf8("line"));
    line->setGeometry(QRect(12, 24, 285, 16));
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    m_DebugInfoText = new QTextEdit(uorOptionsDlgUi);
    m_DebugInfoText->setObjectName(QString::fromUtf8("m_DebugInfoText"));
    m_DebugInfoText->setGeometry(QRect(2, 120, 393, 130));
    m_DebugInfoLabel = new QLabel(uorOptionsDlgUi);
    m_DebugInfoLabel->setObjectName(QString::fromUtf8("m_DebugInfoLabel"));
    m_DebugInfoLabel->setGeometry(QRect(8, 100, 154, 16));
    QFont font;
    font.setPointSize(8);
    font.setBold(true);
    font.setWeight(75);
    m_DebugInfoLabel->setFont(font);
    m_DebugInfoLabel->setFrameShape(QFrame::NoFrame);
    m_DebugInfoLabel->setFrameShadow(QFrame::Plain);
    m_saveWithUndoStack = new QCheckBox(uorOptionsDlgUi);
    m_saveWithUndoStack->setObjectName(QString::fromUtf8("m_saveWithUndoStack"));
    m_saveWithUndoStack->setGeometry(QRect(10, 60, 151, 19));
    m_directDrawFlag = new QCheckBox(uorOptionsDlgUi);
    m_directDrawFlag->setObjectName(QString::fromUtf8("m_directDrawFlag"));
    m_directDrawFlag->setGeometry(QRect(12, 80, 151, 19));

    retranslateUi(uorOptionsDlgUi);
    QObject::connect(buttonBox, SIGNAL(accepted()), uorOptionsDlgUi, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), uorOptionsDlgUi, SLOT(reject()));

    QMetaObject::connectSlotsByName(uorOptionsDlgUi);
    } // setupUi

    void retranslateUi(QDialog *uorOptionsDlgUi)
    {
    uorOptionsDlgUi->setWindowTitle(QApplication::translate("uorOptionsDlgUi", "Options...", 0, QApplication::UnicodeUTF8));
    label->setText(QApplication::translate("uorOptionsDlgUi", "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt; font-weight:600;\">Report options</span></p></body></html>", 0, QApplication::UnicodeUTF8));
    m_saveWithSelection->setText(QApplication::translate("uorOptionsDlgUi", "Save with selection", 0, QApplication::UnicodeUTF8));
    m_DebugInfoLabel->setText(QApplication::translate("uorOptionsDlgUi", "Debug info", 0, QApplication::UnicodeUTF8));
    m_saveWithUndoStack->setText(QApplication::translate("uorOptionsDlgUi", "Save with undo stack", 0, QApplication::UnicodeUTF8));
    m_directDrawFlag->setText(QApplication::translate("uorOptionsDlgUi", "direct draw", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(uorOptionsDlgUi);
    } // retranslateUi

};

namespace Ui {
    class uorOptionsDlgUi: public Ui_uorOptionsDlgUi {};
} // namespace Ui

#endif // UI_UOROPTIONS_H
