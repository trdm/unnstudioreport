/********************************************************************************
** Form generated from reading ui file 'uorSubstrateDlgDats.ui'
**
** Created: Sun 13. Dec 18:52:40 2009
**      by: Qt User Interface Compiler version 4.3.4
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_UORSUBSTRATEDLGDATS_H
#define UI_UORSUBSTRATEDLGDATS_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QDialog>
#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSpinBox>

class Ui_Dialog
{
public:
    QLabel *label;
    QLineEdit *lineEdit;
    QPushButton *pushButton;
    QGroupBox *groupBox;
    QSpinBox *spinBox;
    QLabel *label_2;
    QSpinBox *spinBox_2;
    QLabel *label_3;
    QGroupBox *groupBox_2;
    QSpinBox *spinBox_3;
    QLabel *label_4;
    QSpinBox *spinBox_4;
    QLabel *label_5;
    QCheckBox *checkBox;

    void setupUi(QDialog *Dialog)
    {
    if (Dialog->objectName().isEmpty())
        Dialog->setObjectName(QString::fromUtf8("Dialog"));
    Dialog->resize(392, 217);
    Dialog->setSizeGripEnabled(false);
    label = new QLabel(Dialog);
    label->setObjectName(QString::fromUtf8("label"));
    label->setGeometry(QRect(10, 10, 201, 16));
    lineEdit = new QLineEdit(Dialog);
    lineEdit->setObjectName(QString::fromUtf8("lineEdit"));
    lineEdit->setGeometry(QRect(10, 30, 291, 20));
    pushButton = new QPushButton(Dialog);
    pushButton->setObjectName(QString::fromUtf8("pushButton"));
    pushButton->setGeometry(QRect(301, 26, 77, 26));
    groupBox = new QGroupBox(Dialog);
    groupBox->setObjectName(QString::fromUtf8("groupBox"));
    groupBox->setGeometry(QRect(10, 60, 371, 51));
    spinBox = new QSpinBox(groupBox);
    spinBox->setObjectName(QString::fromUtf8("spinBox"));
    spinBox->setGeometry(QRect(90, 18, 81, 22));
    label_2 = new QLabel(groupBox);
    label_2->setObjectName(QString::fromUtf8("label_2"));
    label_2->setGeometry(QRect(10, 20, 81, 16));
    spinBox_2 = new QSpinBox(groupBox);
    spinBox_2->setObjectName(QString::fromUtf8("spinBox_2"));
    spinBox_2->setGeometry(QRect(280, 18, 81, 22));
    label_3 = new QLabel(groupBox);
    label_3->setObjectName(QString::fromUtf8("label_3"));
    label_3->setGeometry(QRect(190, 20, 81, 16));
    groupBox_2 = new QGroupBox(Dialog);
    groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
    groupBox_2->setGeometry(QRect(10, 120, 371, 75));
    spinBox_3 = new QSpinBox(groupBox_2);
    spinBox_3->setObjectName(QString::fromUtf8("spinBox_3"));
    spinBox_3->setGeometry(QRect(90, 40, 81, 22));
    label_4 = new QLabel(groupBox_2);
    label_4->setObjectName(QString::fromUtf8("label_4"));
    label_4->setGeometry(QRect(10, 42, 81, 16));
    spinBox_4 = new QSpinBox(groupBox_2);
    spinBox_4->setObjectName(QString::fromUtf8("spinBox_4"));
    spinBox_4->setGeometry(QRect(280, 40, 81, 22));
    label_5 = new QLabel(groupBox_2);
    label_5->setObjectName(QString::fromUtf8("label_5"));
    label_5->setGeometry(QRect(190, 42, 81, 16));
    checkBox = new QCheckBox(groupBox_2);
    checkBox->setObjectName(QString::fromUtf8("checkBox"));
    checkBox->setGeometry(QRect(88, 13, 121, 19));

    retranslateUi(Dialog);

    QMetaObject::connectSlotsByName(Dialog);
    } // setupUi

    void retranslateUi(QDialog *Dialog)
    {
    Dialog->setWindowTitle(QApplication::translate("Dialog", "\320\224\320\260\320\275\320\275\321\213\320\265 \320\277\320\276\320\264\320\273\320\276\320\266\320\272\320\270", 0, QApplication::UnicodeUTF8));
    label->setText(QApplication::translate("Dialog", "\320\222\321\213\320\261\320\265\321\200\320\270\321\202\320\265 \321\204\320\260\320\271\320\273 \320\277\320\276\320\264\320\273\320\276\320\266\320\272\320\270:", 0, QApplication::UnicodeUTF8));
    pushButton->setText(QApplication::translate("Dialog", "...", 0, QApplication::UnicodeUTF8));
    groupBox->setTitle(QApplication::translate("Dialog", "\320\241\320\274\320\265\321\211\320\265\320\275\320\270\320\265 \320\273\320\265\320\262\320\276\320\263\320\276 \320\262\320\265\321\200\321\205\320\275\320\265\320\263\320\276 \321\203\320\263\320\273\320\260:", 0, QApplication::UnicodeUTF8));
    label_2->setText(QApplication::translate("Dialog", "\320\277\320\276 \320\262\320\265\321\200\321\202\320\270\320\272\320\260\320\273\320\270:", 0, QApplication::UnicodeUTF8));
    label_3->setText(QApplication::translate("Dialog", "\320\277\320\276 \320\263\320\276\321\200\320\270\320\267\320\276\320\275\321\202\320\260\320\273\320\270:", 0, QApplication::UnicodeUTF8));
    groupBox_2->setTitle(QApplication::translate("Dialog", "\320\234\320\260\321\201\321\210\321\202\320\260\320\261:", 0, QApplication::UnicodeUTF8));
    label_4->setText(QApplication::translate("Dialog", "\320\277\320\276 \320\262\320\265\321\200\321\202\320\270\320\272\320\260\320\273\320\270:", 0, QApplication::UnicodeUTF8));
    label_5->setText(QApplication::translate("Dialog", "\320\277\320\276 \320\263\320\276\321\200\320\270\320\267\320\276\320\275\321\202\320\260\320\273\320\270:", 0, QApplication::UnicodeUTF8));
    checkBox->setText(QApplication::translate("Dialog", "\320\237\321\200\320\276\320\277\320\276\321\200\321\206\320\270\320\276\320\275\320\260\320\273\321\214\320\275\320\276", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(Dialog);
    } // retranslateUi

};

namespace Ui {
    class Dialog: public Ui_Dialog {};
} // namespace Ui

#endif // UI_UORSUBSTRATEDLGDATS_H
