/********************************************************************************
** Form generated from reading ui file 'uorBorderProp.ui'
**
** Created: Sun 13. Dec 18:52:40 2009
**      by: Qt User Interface Compiler version 4.3.4
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_UORBORDERPROP_H
#define UI_UORBORDERPROP_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QSpacerItem>
#include <QtGui/QWidget>

class Ui_uorBorderProp
{
public:
    QGridLayout *gridLayout;
    QGridLayout *gridLayout1;
    QLabel *m_labelTB;
    QLabel *m_labelOutline;
    QFrame *m_leftBorderFrame;
    QLabel *m_labelBB;
    QFrame *m_outlineBorderFrame;
    QLabel *m_labelRB;
    QFrame *m_bottomBorderFrame;
    QFrame *m_topBorderFrame;
    QFrame *m_rightBorderFrame;
    QLabel *m_labelLB;
    QSpacerItem *spacerItem;
    QFrame *frame;

    void setupUi(QWidget *uorBorderProp)
    {
    if (uorBorderProp->objectName().isEmpty())
        uorBorderProp->setObjectName(QString::fromUtf8("uorBorderProp"));
    uorBorderProp->resize(392, 114);
    gridLayout = new QGridLayout(uorBorderProp);
    gridLayout->setSpacing(1);
    gridLayout->setMargin(1);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    gridLayout1 = new QGridLayout();
    gridLayout1->setSpacing(2);
    gridLayout1->setMargin(2);
    gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
    m_labelTB = new QLabel(uorBorderProp);
    m_labelTB->setObjectName(QString::fromUtf8("m_labelTB"));
    QSizePolicy sizePolicy(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(1));
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(m_labelTB->sizePolicy().hasHeightForWidth());
    m_labelTB->setSizePolicy(sizePolicy);

    gridLayout1->addWidget(m_labelTB, 1, 0, 1, 1);

    m_labelOutline = new QLabel(uorBorderProp);
    m_labelOutline->setObjectName(QString::fromUtf8("m_labelOutline"));

    gridLayout1->addWidget(m_labelOutline, 4, 0, 1, 1);

    m_leftBorderFrame = new QFrame(uorBorderProp);
    m_leftBorderFrame->setObjectName(QString::fromUtf8("m_leftBorderFrame"));
    m_leftBorderFrame->setFrameShape(QFrame::NoFrame);
    m_leftBorderFrame->setFrameShadow(QFrame::Plain);
    m_leftBorderFrame->setLineWidth(1);

    gridLayout1->addWidget(m_leftBorderFrame, 0, 1, 1, 1);

    m_labelBB = new QLabel(uorBorderProp);
    m_labelBB->setObjectName(QString::fromUtf8("m_labelBB"));

    gridLayout1->addWidget(m_labelBB, 3, 0, 1, 1);

    m_outlineBorderFrame = new QFrame(uorBorderProp);
    m_outlineBorderFrame->setObjectName(QString::fromUtf8("m_outlineBorderFrame"));
    m_outlineBorderFrame->setFrameShape(QFrame::NoFrame);
    m_outlineBorderFrame->setFrameShadow(QFrame::Raised);

    gridLayout1->addWidget(m_outlineBorderFrame, 4, 1, 1, 1);

    m_labelRB = new QLabel(uorBorderProp);
    m_labelRB->setObjectName(QString::fromUtf8("m_labelRB"));
    sizePolicy.setHeightForWidth(m_labelRB->sizePolicy().hasHeightForWidth());
    m_labelRB->setSizePolicy(sizePolicy);

    gridLayout1->addWidget(m_labelRB, 2, 0, 1, 1);

    m_bottomBorderFrame = new QFrame(uorBorderProp);
    m_bottomBorderFrame->setObjectName(QString::fromUtf8("m_bottomBorderFrame"));
    m_bottomBorderFrame->setFrameShape(QFrame::NoFrame);
    m_bottomBorderFrame->setFrameShadow(QFrame::Raised);

    gridLayout1->addWidget(m_bottomBorderFrame, 3, 1, 1, 1);

    m_topBorderFrame = new QFrame(uorBorderProp);
    m_topBorderFrame->setObjectName(QString::fromUtf8("m_topBorderFrame"));
    m_topBorderFrame->setFrameShape(QFrame::NoFrame);
    m_topBorderFrame->setFrameShadow(QFrame::Raised);

    gridLayout1->addWidget(m_topBorderFrame, 1, 1, 1, 1);

    m_rightBorderFrame = new QFrame(uorBorderProp);
    m_rightBorderFrame->setObjectName(QString::fromUtf8("m_rightBorderFrame"));
    m_rightBorderFrame->setFrameShape(QFrame::NoFrame);
    m_rightBorderFrame->setFrameShadow(QFrame::Raised);

    gridLayout1->addWidget(m_rightBorderFrame, 2, 1, 1, 1);

    m_labelLB = new QLabel(uorBorderProp);
    m_labelLB->setObjectName(QString::fromUtf8("m_labelLB"));
    QSizePolicy sizePolicy1(static_cast<QSizePolicy::Policy>(0), static_cast<QSizePolicy::Policy>(0));
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(m_labelLB->sizePolicy().hasHeightForWidth());
    m_labelLB->setSizePolicy(sizePolicy1);

    gridLayout1->addWidget(m_labelLB, 0, 0, 1, 1);

    spacerItem = new QSpacerItem(20, 1, QSizePolicy::Minimum, QSizePolicy::Expanding);

    gridLayout1->addItem(spacerItem, 5, 0, 1, 2);

    frame = new QFrame(uorBorderProp);
    frame->setObjectName(QString::fromUtf8("frame"));
    frame->setFrameShape(QFrame::StyledPanel);
    frame->setFrameShadow(QFrame::Raised);

    gridLayout1->addWidget(frame, 0, 2, 6, 1);


    gridLayout->addLayout(gridLayout1, 0, 0, 2, 1);


    retranslateUi(uorBorderProp);

    QMetaObject::connectSlotsByName(uorBorderProp);
    } // setupUi

    void retranslateUi(QWidget *uorBorderProp)
    {
    uorBorderProp->setWindowTitle(QApplication::translate("uorBorderProp", "Form", 0, QApplication::UnicodeUTF8));
    m_labelTB->setText(QApplication::translate("uorBorderProp", "Top border", 0, QApplication::UnicodeUTF8));
    m_labelOutline->setText(QApplication::translate("uorBorderProp", "Outline", 0, QApplication::UnicodeUTF8));
    m_labelBB->setText(QApplication::translate("uorBorderProp", "Bottom border", 0, QApplication::UnicodeUTF8));
    m_labelRB->setText(QApplication::translate("uorBorderProp", "Right border", 0, QApplication::UnicodeUTF8));
    m_labelLB->setText(QApplication::translate("uorBorderProp", "Left border", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(uorBorderProp);
    } // retranslateUi

};

namespace Ui {
    class uorBorderProp: public Ui_uorBorderProp {};
} // namespace Ui

#endif // UI_UORBORDERPROP_H
