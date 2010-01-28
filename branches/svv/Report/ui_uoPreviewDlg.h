/********************************************************************************
** Form generated from reading ui file 'uoPreviewDlg.ui'
**
** Created: Sat 16. Jan 19:13:23 2010
**      by: Qt User Interface Compiler version 4.3.4
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_UOPREVIEWDLG_H
#define UI_UOPREVIEWDLG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>

class Ui_uoReportPrevDlg
{
public:
    QGridLayout *gridLayout;
    QGridLayout *gridLayout1;
    QPushButton *m_pbPrint;
    QPushButton *m_pbPage;
    QPushButton *m_pbZoomIn;
    QPushButton *m_pbZoomOut;
    QPushButton *m_pbClose;
    QSpacerItem *spacerItem;
    QPushButton *m_pbNext;
    QPushButton *m_pbPrevious;
    QFrame *m_frame;
    QLineEdit *m_leCurentPage;
    QLabel *m_labelTotalPg;

    void setupUi(QDialog *uoReportPrevDlg)
    {
    if (uoReportPrevDlg->objectName().isEmpty())
        uoReportPrevDlg->setObjectName(QString::fromUtf8("uoReportPrevDlg"));
    uoReportPrevDlg->resize(673, 433);
    gridLayout = new QGridLayout(uoReportPrevDlg);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    gridLayout->setHorizontalSpacing(1);
    gridLayout->setVerticalSpacing(1);
    gridLayout->setContentsMargins(1, 1, 1, 1);
    gridLayout1 = new QGridLayout();
    gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
    gridLayout1->setHorizontalSpacing(2);
    gridLayout1->setVerticalSpacing(2);
    m_pbPrint = new QPushButton(uoReportPrevDlg);
    m_pbPrint->setObjectName(QString::fromUtf8("m_pbPrint"));
    m_pbPrint->setAutoDefault(false);

    gridLayout1->addWidget(m_pbPrint, 0, 1, 1, 1);

    m_pbPage = new QPushButton(uoReportPrevDlg);
    m_pbPage->setObjectName(QString::fromUtf8("m_pbPage"));
    m_pbPage->setAutoDefault(false);

    gridLayout1->addWidget(m_pbPage, 0, 2, 1, 1);

    m_pbZoomIn = new QPushButton(uoReportPrevDlg);
    m_pbZoomIn->setObjectName(QString::fromUtf8("m_pbZoomIn"));
    m_pbZoomIn->setEnabled(false);
    m_pbZoomIn->setAutoDefault(false);

    gridLayout1->addWidget(m_pbZoomIn, 0, 7, 1, 1);

    m_pbZoomOut = new QPushButton(uoReportPrevDlg);
    m_pbZoomOut->setObjectName(QString::fromUtf8("m_pbZoomOut"));
    m_pbZoomOut->setEnabled(false);
    m_pbZoomOut->setAutoDefault(false);

    gridLayout1->addWidget(m_pbZoomOut, 0, 8, 1, 1);

    m_pbClose = new QPushButton(uoReportPrevDlg);
    m_pbClose->setObjectName(QString::fromUtf8("m_pbClose"));
    m_pbClose->setAutoDefault(false);

    gridLayout1->addWidget(m_pbClose, 0, 9, 1, 1);

    spacerItem = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    gridLayout1->addItem(spacerItem, 0, 10, 1, 1);

    m_pbNext = new QPushButton(uoReportPrevDlg);
    m_pbNext->setObjectName(QString::fromUtf8("m_pbNext"));
    m_pbNext->setIcon(QIcon(QString::fromUtf8(":/image/go-next.PNG")));
    m_pbNext->setAutoDefault(false);

    gridLayout1->addWidget(m_pbNext, 0, 6, 1, 1);

    m_pbPrevious = new QPushButton(uoReportPrevDlg);
    m_pbPrevious->setObjectName(QString::fromUtf8("m_pbPrevious"));
    m_pbPrevious->setIcon(QIcon(QString::fromUtf8(":/image/go-previous.png")));
    m_pbPrevious->setAutoDefault(false);

    gridLayout1->addWidget(m_pbPrevious, 0, 3, 1, 1);

    m_frame = new QFrame(uoReportPrevDlg);
    m_frame->setObjectName(QString::fromUtf8("m_frame"));
    m_frame->setFrameShape(QFrame::StyledPanel);
    m_frame->setFrameShadow(QFrame::Sunken);

    gridLayout1->addWidget(m_frame, 1, 0, 2, 11);

    m_leCurentPage = new QLineEdit(uoReportPrevDlg);
    m_leCurentPage->setObjectName(QString::fromUtf8("m_leCurentPage"));
    m_leCurentPage->setMinimumSize(QSize(50, 20));
    m_leCurentPage->setMaximumSize(QSize(50, 20));
    m_leCurentPage->setMaxLength(5);
    m_leCurentPage->setCursorPosition(0);

    gridLayout1->addWidget(m_leCurentPage, 0, 4, 1, 1);

    m_labelTotalPg = new QLabel(uoReportPrevDlg);
    m_labelTotalPg->setObjectName(QString::fromUtf8("m_labelTotalPg"));
    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(m_labelTotalPg->sizePolicy().hasHeightForWidth());
    m_labelTotalPg->setSizePolicy(sizePolicy);
    m_labelTotalPg->setMinimumSize(QSize(30, 20));
    m_labelTotalPg->setMaximumSize(QSize(30, 20));

    gridLayout1->addWidget(m_labelTotalPg, 0, 5, 1, 1);


    gridLayout->addLayout(gridLayout1, 0, 0, 4, 1);

    QWidget::setTabOrder(m_pbPrint, m_pbPage);
    QWidget::setTabOrder(m_pbPage, m_pbNext);
    QWidget::setTabOrder(m_pbNext, m_pbPrevious);
    QWidget::setTabOrder(m_pbPrevious, m_pbZoomIn);
    QWidget::setTabOrder(m_pbZoomIn, m_pbZoomOut);
    QWidget::setTabOrder(m_pbZoomOut, m_pbClose);

    retranslateUi(uoReportPrevDlg);

    QMetaObject::connectSlotsByName(uoReportPrevDlg);
    } // setupUi

    void retranslateUi(QDialog *uoReportPrevDlg)
    {
    uoReportPrevDlg->setWindowTitle(QApplication::translate("uoReportPrevDlg", "Dialog", 0, QApplication::UnicodeUTF8));
    m_pbPrint->setText(QApplication::translate("uoReportPrevDlg", "Print...", 0, QApplication::UnicodeUTF8));
    m_pbPage->setText(QApplication::translate("uoReportPrevDlg", "Page...", 0, QApplication::UnicodeUTF8));
    m_pbZoomIn->setText(QApplication::translate("uoReportPrevDlg", "ZoomIn", 0, QApplication::UnicodeUTF8));
    m_pbZoomOut->setText(QApplication::translate("uoReportPrevDlg", "ZoomOut", 0, QApplication::UnicodeUTF8));
    m_pbClose->setText(QApplication::translate("uoReportPrevDlg", "Close", 0, QApplication::UnicodeUTF8));
    m_pbNext->setText(QApplication::translate("uoReportPrevDlg", "Next", 0, QApplication::UnicodeUTF8));
    m_pbPrevious->setText(QApplication::translate("uoReportPrevDlg", "Prev.", 0, QApplication::UnicodeUTF8));
    m_leCurentPage->setInputMask(QApplication::translate("uoReportPrevDlg", "99999; ", 0, QApplication::UnicodeUTF8));
    m_leCurentPage->setText(QString());
    m_labelTotalPg->setText(QApplication::translate("uoReportPrevDlg", "/ 0", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(uoReportPrevDlg);
    } // retranslateUi

};

namespace Ui {
    class uoReportPrevDlg: public Ui_uoReportPrevDlg {};
} // namespace Ui

#endif // UI_UOPREVIEWDLG_H
