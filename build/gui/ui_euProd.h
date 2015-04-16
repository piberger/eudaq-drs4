/********************************************************************************
** Form generated from reading UI file 'euProd.ui'
**
** Created: Thu Apr 16 15:43:43 2015
**      by: Qt User Interface Compiler version 4.6.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EUPROD_H
#define UI_EUPROD_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QFrame>
#include <QtGui/QGraphicsView>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QSpinBox>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_wndProd
{
public:
    QWidget *centralwidget;
    QHBoxLayout *hboxLayout;
    QVBoxLayout *vboxLayout;
    QGroupBox *grpData;
    QVBoxLayout *vboxLayout1;
    QHBoxLayout *hboxLayout1;
    QRadioButton *radFile;
    QLineEdit *txtFile;
    QRadioButton *radCustom;
    QGridLayout *gridLayout;
    QLabel *lblWidth;
    QSpinBox *spnWidth;
    QLabel *lblProfile;
    QLabel *lblX;
    QLabel *lblHeight;
    QSpinBox *spnHeight;
    QLabel *lblRadius;
    QLabel *lblY;
    QComboBox *comboBox;
    QDoubleSpinBox *spnY;
    QDoubleSpinBox *spnX;
    QDoubleSpinBox *spnRadius;
    QGridLayout *gridLayout1;
    QDoubleSpinBox *spnNoise;
    QDoubleSpinBox *spnSparsify;
    QCheckBox *chkPed;
    QCheckBox *chkSparsify;
    QDoubleSpinBox *spnPed;
    QCheckBox *chkNoise;
    QCheckBox *chkCMmean;
    QDoubleSpinBox *spnPulse;
    QCheckBox *chkCMstdev;
    QDoubleSpinBox *spnCMmean;
    QDoubleSpinBox *spnCMstdev;
    QCheckBox *chkPulses;
    QGroupBox *grpTriggers;
    QHBoxLayout *hboxLayout2;
    QVBoxLayout *vboxLayout2;
    QRadioButton *radManual;
    QRadioButton *radFixed;
    QRadioButton *radPoisson;
    QVBoxLayout *vboxLayout3;
    QGridLayout *gridLayout2;
    QDoubleSpinBox *spnFreq;
    QDoubleSpinBox *spnPeriod;
    QLabel *lblFreq;
    QLabel *lblPeriod;
    QPushButton *btnTrigger;
    QVBoxLayout *vboxLayout4;
    QLabel *lblPreview;
    QGraphicsView *grphPreview;
    QGroupBox *grpStatus;
    QVBoxLayout *vboxLayout5;
    QHBoxLayout *hboxLayout3;
    QLineEdit *txtStatus;
    QComboBox *cmbStatus;
    QLabel *lblStatus;
    QFrame *line;
    QGridLayout *gridLayout3;
    QLineEdit *txtDataColl;
    QLineEdit *txtState;
    QLabel *lblRun;
    QLabel *lblState;
    QLabel *lblConfig;
    QLabel *lblDataColl;
    QLineEdit *txtConfig;
    QLabel *lblEvent;
    QLineEdit *txtRun;
    QLineEdit *txtEvent;
    QPushButton *btnQuit;
    QMenuBar *menubar;

    void setupUi(QMainWindow *wndProd)
    {
        if (wndProd->objectName().isEmpty())
            wndProd->setObjectName(QString::fromUtf8("wndProd"));
        wndProd->resize(705, 542);
        centralwidget = new QWidget(wndProd);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        hboxLayout = new QHBoxLayout(centralwidget);
#ifndef Q_OS_MAC
        hboxLayout->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        hboxLayout->setContentsMargins(9, 9, 9, 9);
#endif
        hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
        vboxLayout = new QVBoxLayout();
#ifndef Q_OS_MAC
        vboxLayout->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        vboxLayout->setContentsMargins(0, 0, 0, 0);
#endif
        vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
        grpData = new QGroupBox(centralwidget);
        grpData->setObjectName(QString::fromUtf8("grpData"));
        vboxLayout1 = new QVBoxLayout(grpData);
#ifndef Q_OS_MAC
        vboxLayout1->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        vboxLayout1->setContentsMargins(9, 9, 9, 9);
#endif
        vboxLayout1->setObjectName(QString::fromUtf8("vboxLayout1"));
        hboxLayout1 = new QHBoxLayout();
        hboxLayout1->setSpacing(0);
#ifndef Q_OS_MAC
        hboxLayout1->setContentsMargins(0, 0, 0, 0);
#endif
        hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
        radFile = new QRadioButton(grpData);
        radFile->setObjectName(QString::fromUtf8("radFile"));

        hboxLayout1->addWidget(radFile);

        txtFile = new QLineEdit(grpData);
        txtFile->setObjectName(QString::fromUtf8("txtFile"));
        QSizePolicy sizePolicy(static_cast<QSizePolicy::Policy>(3), static_cast<QSizePolicy::Policy>(0));
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(txtFile->sizePolicy().hasHeightForWidth());
        txtFile->setSizePolicy(sizePolicy);
        txtFile->setMinimumSize(QSize(30, 0));

        hboxLayout1->addWidget(txtFile);


        vboxLayout1->addLayout(hboxLayout1);

        radCustom = new QRadioButton(grpData);
        radCustom->setObjectName(QString::fromUtf8("radCustom"));
        radCustom->setChecked(true);

        vboxLayout1->addWidget(radCustom);

        gridLayout = new QGridLayout();
        gridLayout->setSpacing(0);
        gridLayout->setContentsMargins(0, 0, 0, 0);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        lblWidth = new QLabel(grpData);
        lblWidth->setObjectName(QString::fromUtf8("lblWidth"));

        gridLayout->addWidget(lblWidth, 0, 0, 1, 1);

        spnWidth = new QSpinBox(grpData);
        spnWidth->setObjectName(QString::fromUtf8("spnWidth"));
        sizePolicy.setHeightForWidth(spnWidth->sizePolicy().hasHeightForWidth());
        spnWidth->setSizePolicy(sizePolicy);
        spnWidth->setMinimumSize(QSize(50, 0));
        spnWidth->setFrame(false);
        spnWidth->setAlignment(Qt::AlignRight);
        spnWidth->setMaximum(65536);
        spnWidth->setValue(512);

        gridLayout->addWidget(spnWidth, 0, 1, 1, 1);

        lblProfile = new QLabel(grpData);
        lblProfile->setObjectName(QString::fromUtf8("lblProfile"));

        gridLayout->addWidget(lblProfile, 1, 0, 1, 1);

        lblX = new QLabel(grpData);
        lblX->setObjectName(QString::fromUtf8("lblX"));

        gridLayout->addWidget(lblX, 2, 0, 1, 1);

        lblHeight = new QLabel(grpData);
        lblHeight->setObjectName(QString::fromUtf8("lblHeight"));

        gridLayout->addWidget(lblHeight, 0, 2, 1, 1);

        spnHeight = new QSpinBox(grpData);
        spnHeight->setObjectName(QString::fromUtf8("spnHeight"));
        sizePolicy.setHeightForWidth(spnHeight->sizePolicy().hasHeightForWidth());
        spnHeight->setSizePolicy(sizePolicy);
        spnHeight->setMinimumSize(QSize(60, 0));
        spnHeight->setFrame(false);
        spnHeight->setAlignment(Qt::AlignRight);
        spnHeight->setMaximum(65536);
        spnHeight->setValue(512);

        gridLayout->addWidget(spnHeight, 0, 3, 1, 1);

        lblRadius = new QLabel(grpData);
        lblRadius->setObjectName(QString::fromUtf8("lblRadius"));

        gridLayout->addWidget(lblRadius, 1, 2, 1, 1);

        lblY = new QLabel(grpData);
        lblY->setObjectName(QString::fromUtf8("lblY"));

        gridLayout->addWidget(lblY, 2, 2, 1, 1);

        comboBox = new QComboBox(grpData);
        comboBox->setObjectName(QString::fromUtf8("comboBox"));

        gridLayout->addWidget(comboBox, 1, 1, 1, 1);

        spnY = new QDoubleSpinBox(grpData);
        spnY->setObjectName(QString::fromUtf8("spnY"));
        spnY->setFrame(false);
        spnY->setDecimals(1);
        spnY->setMaximum(65536);
        spnY->setValue(256);

        gridLayout->addWidget(spnY, 2, 3, 1, 1);

        spnX = new QDoubleSpinBox(grpData);
        spnX->setObjectName(QString::fromUtf8("spnX"));
        spnX->setFrame(false);
        spnX->setAlignment(Qt::AlignRight);
        spnX->setDecimals(1);
        spnX->setMaximum(65536);
        spnX->setValue(256);

        gridLayout->addWidget(spnX, 2, 1, 1, 1);

        spnRadius = new QDoubleSpinBox(grpData);
        spnRadius->setObjectName(QString::fromUtf8("spnRadius"));
        spnRadius->setFrame(false);
        spnRadius->setDecimals(1);
        spnRadius->setMaximum(65536);
        spnRadius->setValue(100);

        gridLayout->addWidget(spnRadius, 1, 3, 1, 1);


        vboxLayout1->addLayout(gridLayout);

        gridLayout1 = new QGridLayout();
        gridLayout1->setSpacing(0);
        gridLayout1->setContentsMargins(0, 0, 0, 0);
        gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
        spnNoise = new QDoubleSpinBox(grpData);
        spnNoise->setObjectName(QString::fromUtf8("spnNoise"));
        sizePolicy.setHeightForWidth(spnNoise->sizePolicy().hasHeightForWidth());
        spnNoise->setSizePolicy(sizePolicy);
        spnNoise->setMinimumSize(QSize(30, 0));
        spnNoise->setFrame(false);
        spnNoise->setAlignment(Qt::AlignRight);
        spnNoise->setDecimals(1);
        spnNoise->setMaximum(65536);
        spnNoise->setValue(4);

        gridLayout1->addWidget(spnNoise, 3, 1, 1, 1);

        spnSparsify = new QDoubleSpinBox(grpData);
        spnSparsify->setObjectName(QString::fromUtf8("spnSparsify"));
        sizePolicy.setHeightForWidth(spnSparsify->sizePolicy().hasHeightForWidth());
        spnSparsify->setSizePolicy(sizePolicy);
        spnSparsify->setMinimumSize(QSize(30, 0));
        spnSparsify->setFrame(false);
        spnSparsify->setAlignment(Qt::AlignRight);
        spnSparsify->setDecimals(1);
        spnSparsify->setMaximum(65536);
        spnSparsify->setValue(10);

        gridLayout1->addWidget(spnSparsify, 5, 1, 1, 1);

        chkPed = new QCheckBox(grpData);
        chkPed->setObjectName(QString::fromUtf8("chkPed"));
        chkPed->setChecked(true);

        gridLayout1->addWidget(chkPed, 4, 0, 1, 1);

        chkSparsify = new QCheckBox(grpData);
        chkSparsify->setObjectName(QString::fromUtf8("chkSparsify"));
        chkSparsify->setChecked(true);

        gridLayout1->addWidget(chkSparsify, 5, 0, 1, 1);

        spnPed = new QDoubleSpinBox(grpData);
        spnPed->setObjectName(QString::fromUtf8("spnPed"));
        sizePolicy.setHeightForWidth(spnPed->sizePolicy().hasHeightForWidth());
        spnPed->setSizePolicy(sizePolicy);
        spnPed->setMinimumSize(QSize(30, 0));
        spnPed->setFrame(false);
        spnPed->setAlignment(Qt::AlignRight);
        spnPed->setDecimals(1);
        spnPed->setMaximum(65536);
        spnPed->setValue(2);

        gridLayout1->addWidget(spnPed, 4, 1, 1, 1);

        chkNoise = new QCheckBox(grpData);
        chkNoise->setObjectName(QString::fromUtf8("chkNoise"));
        chkNoise->setChecked(true);

        gridLayout1->addWidget(chkNoise, 3, 0, 1, 1);

        chkCMmean = new QCheckBox(grpData);
        chkCMmean->setObjectName(QString::fromUtf8("chkCMmean"));
        chkCMmean->setChecked(true);

        gridLayout1->addWidget(chkCMmean, 1, 0, 1, 1);

        spnPulse = new QDoubleSpinBox(grpData);
        spnPulse->setObjectName(QString::fromUtf8("spnPulse"));
        sizePolicy.setHeightForWidth(spnPulse->sizePolicy().hasHeightForWidth());
        spnPulse->setSizePolicy(sizePolicy);
        spnPulse->setMinimumSize(QSize(30, 0));
        spnPulse->setFrame(false);
        spnPulse->setAlignment(Qt::AlignRight);
        spnPulse->setDecimals(1);
        spnPulse->setMaximum(65536);
        spnPulse->setValue(20);

        gridLayout1->addWidget(spnPulse, 0, 1, 1, 1);

        chkCMstdev = new QCheckBox(grpData);
        chkCMstdev->setObjectName(QString::fromUtf8("chkCMstdev"));
        chkCMstdev->setChecked(true);

        gridLayout1->addWidget(chkCMstdev, 2, 0, 1, 1);

        spnCMmean = new QDoubleSpinBox(grpData);
        spnCMmean->setObjectName(QString::fromUtf8("spnCMmean"));
        spnCMmean->setFrame(false);
        spnCMmean->setAlignment(Qt::AlignRight);
        spnCMmean->setDecimals(1);
        spnCMmean->setMaximum(65536);
        spnCMmean->setValue(20);

        gridLayout1->addWidget(spnCMmean, 1, 1, 1, 1);

        spnCMstdev = new QDoubleSpinBox(grpData);
        spnCMstdev->setObjectName(QString::fromUtf8("spnCMstdev"));
        spnCMstdev->setFrame(false);
        spnCMstdev->setAlignment(Qt::AlignRight);
        spnCMstdev->setDecimals(1);
        spnCMstdev->setMaximum(65536);
        spnCMstdev->setValue(5);

        gridLayout1->addWidget(spnCMstdev, 2, 1, 1, 1);

        chkPulses = new QCheckBox(grpData);
        chkPulses->setObjectName(QString::fromUtf8("chkPulses"));
        chkPulses->setChecked(true);

        gridLayout1->addWidget(chkPulses, 0, 0, 1, 1);


        vboxLayout1->addLayout(gridLayout1);


        vboxLayout->addWidget(grpData);

        grpTriggers = new QGroupBox(centralwidget);
        grpTriggers->setObjectName(QString::fromUtf8("grpTriggers"));
        hboxLayout2 = new QHBoxLayout(grpTriggers);
#ifndef Q_OS_MAC
        hboxLayout2->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        hboxLayout2->setContentsMargins(9, 9, 9, 9);
#endif
        hboxLayout2->setObjectName(QString::fromUtf8("hboxLayout2"));
        vboxLayout2 = new QVBoxLayout();
        vboxLayout2->setSpacing(0);
#ifndef Q_OS_MAC
        vboxLayout2->setContentsMargins(0, 0, 0, 0);
#endif
        vboxLayout2->setObjectName(QString::fromUtf8("vboxLayout2"));
        radManual = new QRadioButton(grpTriggers);
        radManual->setObjectName(QString::fromUtf8("radManual"));
        radManual->setChecked(true);

        vboxLayout2->addWidget(radManual);

        radFixed = new QRadioButton(grpTriggers);
        radFixed->setObjectName(QString::fromUtf8("radFixed"));

        vboxLayout2->addWidget(radFixed);

        radPoisson = new QRadioButton(grpTriggers);
        radPoisson->setObjectName(QString::fromUtf8("radPoisson"));

        vboxLayout2->addWidget(radPoisson);


        hboxLayout2->addLayout(vboxLayout2);

        vboxLayout3 = new QVBoxLayout();
        vboxLayout3->setSpacing(0);
        vboxLayout3->setContentsMargins(0, 0, 0, 0);
        vboxLayout3->setObjectName(QString::fromUtf8("vboxLayout3"));
        gridLayout2 = new QGridLayout();
        gridLayout2->setSpacing(0);
#ifndef Q_OS_MAC
        gridLayout2->setContentsMargins(0, 0, 0, 0);
#endif
        gridLayout2->setObjectName(QString::fromUtf8("gridLayout2"));
        spnFreq = new QDoubleSpinBox(grpTriggers);
        spnFreq->setObjectName(QString::fromUtf8("spnFreq"));
        sizePolicy.setHeightForWidth(spnFreq->sizePolicy().hasHeightForWidth());
        spnFreq->setSizePolicy(sizePolicy);
        spnFreq->setMinimumSize(QSize(30, 0));
        spnFreq->setFrame(false);
        spnFreq->setAlignment(Qt::AlignRight);
        spnFreq->setMaximum(100000);
        spnFreq->setValue(1);

        gridLayout2->addWidget(spnFreq, 0, 1, 1, 1);

        spnPeriod = new QDoubleSpinBox(grpTriggers);
        spnPeriod->setObjectName(QString::fromUtf8("spnPeriod"));
        sizePolicy.setHeightForWidth(spnPeriod->sizePolicy().hasHeightForWidth());
        spnPeriod->setSizePolicy(sizePolicy);
        spnPeriod->setMinimumSize(QSize(30, 0));
        spnPeriod->setFrame(false);
        spnPeriod->setAlignment(Qt::AlignRight);
        spnPeriod->setDecimals(1);
        spnPeriod->setMaximum(1e+09);
        spnPeriod->setValue(1000);

        gridLayout2->addWidget(spnPeriod, 1, 1, 1, 1);

        lblFreq = new QLabel(grpTriggers);
        lblFreq->setObjectName(QString::fromUtf8("lblFreq"));

        gridLayout2->addWidget(lblFreq, 0, 0, 1, 1);

        lblPeriod = new QLabel(grpTriggers);
        lblPeriod->setObjectName(QString::fromUtf8("lblPeriod"));

        gridLayout2->addWidget(lblPeriod, 1, 0, 1, 1);


        vboxLayout3->addLayout(gridLayout2);

        btnTrigger = new QPushButton(grpTriggers);
        btnTrigger->setObjectName(QString::fromUtf8("btnTrigger"));

        vboxLayout3->addWidget(btnTrigger);


        hboxLayout2->addLayout(vboxLayout3);


        vboxLayout->addWidget(grpTriggers);


        hboxLayout->addLayout(vboxLayout);

        vboxLayout4 = new QVBoxLayout();
#ifndef Q_OS_MAC
        vboxLayout4->setSpacing(6);
#endif
        vboxLayout4->setContentsMargins(0, 0, 0, 0);
        vboxLayout4->setObjectName(QString::fromUtf8("vboxLayout4"));
        lblPreview = new QLabel(centralwidget);
        lblPreview->setObjectName(QString::fromUtf8("lblPreview"));

        vboxLayout4->addWidget(lblPreview);

        grphPreview = new QGraphicsView(centralwidget);
        grphPreview->setObjectName(QString::fromUtf8("grphPreview"));

        vboxLayout4->addWidget(grphPreview);

        grpStatus = new QGroupBox(centralwidget);
        grpStatus->setObjectName(QString::fromUtf8("grpStatus"));
        vboxLayout5 = new QVBoxLayout(grpStatus);
#ifndef Q_OS_MAC
        vboxLayout5->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        vboxLayout5->setContentsMargins(9, 9, 9, 9);
#endif
        vboxLayout5->setObjectName(QString::fromUtf8("vboxLayout5"));
        hboxLayout3 = new QHBoxLayout();
        hboxLayout3->setSpacing(0);
#ifndef Q_OS_MAC
        hboxLayout3->setContentsMargins(0, 0, 0, 0);
#endif
        hboxLayout3->setObjectName(QString::fromUtf8("hboxLayout3"));
        txtStatus = new QLineEdit(grpStatus);
        txtStatus->setObjectName(QString::fromUtf8("txtStatus"));

        hboxLayout3->addWidget(txtStatus);

        cmbStatus = new QComboBox(grpStatus);
        cmbStatus->setObjectName(QString::fromUtf8("cmbStatus"));

        hboxLayout3->addWidget(cmbStatus);


        vboxLayout5->addLayout(hboxLayout3);

        lblStatus = new QLabel(grpStatus);
        lblStatus->setObjectName(QString::fromUtf8("lblStatus"));

        vboxLayout5->addWidget(lblStatus);

        line = new QFrame(grpStatus);
        line->setObjectName(QString::fromUtf8("line"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        vboxLayout5->addWidget(line);

        gridLayout3 = new QGridLayout();
        gridLayout3->setSpacing(0);
        gridLayout3->setContentsMargins(0, 0, 0, 0);
        gridLayout3->setObjectName(QString::fromUtf8("gridLayout3"));
        txtDataColl = new QLineEdit(grpStatus);
        txtDataColl->setObjectName(QString::fromUtf8("txtDataColl"));
        txtDataColl->setReadOnly(true);

        gridLayout3->addWidget(txtDataColl, 4, 1, 1, 1);

        txtState = new QLineEdit(grpStatus);
        txtState->setObjectName(QString::fromUtf8("txtState"));
        txtState->setReadOnly(true);

        gridLayout3->addWidget(txtState, 2, 1, 1, 1);

        lblRun = new QLabel(grpStatus);
        lblRun->setObjectName(QString::fromUtf8("lblRun"));

        gridLayout3->addWidget(lblRun, 1, 0, 1, 1);

        lblState = new QLabel(grpStatus);
        lblState->setObjectName(QString::fromUtf8("lblState"));

        gridLayout3->addWidget(lblState, 2, 0, 1, 1);

        lblConfig = new QLabel(grpStatus);
        lblConfig->setObjectName(QString::fromUtf8("lblConfig"));

        gridLayout3->addWidget(lblConfig, 3, 0, 1, 1);

        lblDataColl = new QLabel(grpStatus);
        lblDataColl->setObjectName(QString::fromUtf8("lblDataColl"));

        gridLayout3->addWidget(lblDataColl, 4, 0, 1, 1);

        txtConfig = new QLineEdit(grpStatus);
        txtConfig->setObjectName(QString::fromUtf8("txtConfig"));
        txtConfig->setReadOnly(true);

        gridLayout3->addWidget(txtConfig, 3, 1, 1, 1);

        lblEvent = new QLabel(grpStatus);
        lblEvent->setObjectName(QString::fromUtf8("lblEvent"));

        gridLayout3->addWidget(lblEvent, 0, 0, 1, 1);

        txtRun = new QLineEdit(grpStatus);
        txtRun->setObjectName(QString::fromUtf8("txtRun"));
        txtRun->setReadOnly(true);

        gridLayout3->addWidget(txtRun, 1, 1, 1, 1);

        txtEvent = new QLineEdit(grpStatus);
        txtEvent->setObjectName(QString::fromUtf8("txtEvent"));
        txtEvent->setReadOnly(true);

        gridLayout3->addWidget(txtEvent, 0, 1, 1, 1);


        vboxLayout5->addLayout(gridLayout3);


        vboxLayout4->addWidget(grpStatus);

        btnQuit = new QPushButton(centralwidget);
        btnQuit->setObjectName(QString::fromUtf8("btnQuit"));

        vboxLayout4->addWidget(btnQuit);


        hboxLayout->addLayout(vboxLayout4);

        wndProd->setCentralWidget(centralwidget);
        menubar = new QMenuBar(wndProd);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 705, 22));
        wndProd->setMenuBar(menubar);
#ifndef QT_NO_SHORTCUT
        lblWidth->setBuddy(spnWidth);
        lblProfile->setBuddy(comboBox);
        lblX->setBuddy(spnX);
        lblHeight->setBuddy(spnHeight);
        lblRadius->setBuddy(spnRadius);
        lblY->setBuddy(spnY);
        lblFreq->setBuddy(spnFreq);
        lblPeriod->setBuddy(spnPeriod);
        lblPreview->setBuddy(grphPreview);
        lblStatus->setBuddy(txtStatus);
        lblRun->setBuddy(txtRun);
        lblState->setBuddy(txtState);
        lblConfig->setBuddy(txtConfig);
        lblDataColl->setBuddy(txtDataColl);
        lblEvent->setBuddy(txtEvent);
#endif // QT_NO_SHORTCUT
        QWidget::setTabOrder(radFile, txtFile);
        QWidget::setTabOrder(txtFile, radCustom);
        QWidget::setTabOrder(radCustom, spnWidth);
        QWidget::setTabOrder(spnWidth, spnHeight);
        QWidget::setTabOrder(spnHeight, comboBox);
        QWidget::setTabOrder(comboBox, spnRadius);
        QWidget::setTabOrder(spnRadius, spnX);
        QWidget::setTabOrder(spnX, spnY);
        QWidget::setTabOrder(spnY, chkPulses);
        QWidget::setTabOrder(chkPulses, spnPulse);
        QWidget::setTabOrder(spnPulse, chkCMmean);
        QWidget::setTabOrder(chkCMmean, spnCMmean);
        QWidget::setTabOrder(spnCMmean, chkCMstdev);
        QWidget::setTabOrder(chkCMstdev, spnCMstdev);
        QWidget::setTabOrder(spnCMstdev, chkNoise);
        QWidget::setTabOrder(chkNoise, spnNoise);
        QWidget::setTabOrder(spnNoise, chkPed);
        QWidget::setTabOrder(chkPed, spnPed);
        QWidget::setTabOrder(spnPed, chkSparsify);
        QWidget::setTabOrder(chkSparsify, spnSparsify);
        QWidget::setTabOrder(spnSparsify, radManual);
        QWidget::setTabOrder(radManual, radFixed);
        QWidget::setTabOrder(radFixed, radPoisson);
        QWidget::setTabOrder(radPoisson, spnFreq);
        QWidget::setTabOrder(spnFreq, spnPeriod);
        QWidget::setTabOrder(spnPeriod, btnTrigger);
        QWidget::setTabOrder(btnTrigger, grphPreview);
        QWidget::setTabOrder(grphPreview, txtStatus);
        QWidget::setTabOrder(txtStatus, cmbStatus);
        QWidget::setTabOrder(cmbStatus, txtEvent);
        QWidget::setTabOrder(txtEvent, txtRun);
        QWidget::setTabOrder(txtRun, txtState);
        QWidget::setTabOrder(txtState, txtConfig);
        QWidget::setTabOrder(txtConfig, txtDataColl);
        QWidget::setTabOrder(txtDataColl, btnQuit);

        retranslateUi(wndProd);
        QObject::connect(btnQuit, SIGNAL(clicked()), wndProd, SLOT(close()));

        QMetaObject::connectSlotsByName(wndProd);
    } // setupUi

    void retranslateUi(QMainWindow *wndProd)
    {
        wndProd->setWindowTitle(QApplication::translate("wndProd", "eudaq Dummy Producer", 0, QApplication::UnicodeUTF8));
        grpData->setTitle(QApplication::translate("wndProd", "Data", 0, QApplication::UnicodeUTF8));
        radFile->setText(QApplication::translate("wndProd", "File:", 0, QApplication::UnicodeUTF8));
        radCustom->setText(QApplication::translate("wndProd", "Custom:", 0, QApplication::UnicodeUTF8));
        lblWidth->setText(QApplication::translate("wndProd", "Width:", 0, QApplication::UnicodeUTF8));
        lblProfile->setText(QApplication::translate("wndProd", "Profile:", 0, QApplication::UnicodeUTF8));
        lblX->setText(QApplication::translate("wndProd", "X:", 0, QApplication::UnicodeUTF8));
        lblHeight->setText(QApplication::translate("wndProd", "Height:", 0, QApplication::UnicodeUTF8));
        lblRadius->setText(QApplication::translate("wndProd", "Radius:", 0, QApplication::UnicodeUTF8));
        lblY->setText(QApplication::translate("wndProd", "Y:", 0, QApplication::UnicodeUTF8));
        comboBox->clear();
        comboBox->insertItems(0, QStringList()
         << QApplication::translate("wndProd", "Circle", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("wndProd", "Gaussian", 0, QApplication::UnicodeUTF8)
        );
        spnNoise->setSuffix(QApplication::translate("wndProd", " ADC", 0, QApplication::UnicodeUTF8));
        spnSparsify->setSuffix(QApplication::translate("wndProd", " ADC", 0, QApplication::UnicodeUTF8));
        chkPed->setText(QApplication::translate("wndProd", "Pedestal:", 0, QApplication::UnicodeUTF8));
        chkSparsify->setText(QApplication::translate("wndProd", "Sparsify:", 0, QApplication::UnicodeUTF8));
        spnPed->setSuffix(QApplication::translate("wndProd", " ADC", 0, QApplication::UnicodeUTF8));
        chkNoise->setText(QApplication::translate("wndProd", "Noise:", 0, QApplication::UnicodeUTF8));
        chkCMmean->setText(QApplication::translate("wndProd", "CM mean:", 0, QApplication::UnicodeUTF8));
        spnPulse->setSuffix(QApplication::translate("wndProd", " ADC", 0, QApplication::UnicodeUTF8));
        chkCMstdev->setText(QApplication::translate("wndProd", "CM st.dev:", 0, QApplication::UnicodeUTF8));
        spnCMmean->setSuffix(QApplication::translate("wndProd", " ADC", 0, QApplication::UnicodeUTF8));
        spnCMstdev->setSuffix(QApplication::translate("wndProd", " ADC", 0, QApplication::UnicodeUTF8));
        chkPulses->setText(QApplication::translate("wndProd", "Pulses:", 0, QApplication::UnicodeUTF8));
        grpTriggers->setTitle(QApplication::translate("wndProd", "Triggers", 0, QApplication::UnicodeUTF8));
        radManual->setText(QApplication::translate("wndProd", "Manual", 0, QApplication::UnicodeUTF8));
        radFixed->setText(QApplication::translate("wndProd", "Fixed", 0, QApplication::UnicodeUTF8));
        radPoisson->setText(QApplication::translate("wndProd", "Poisson", 0, QApplication::UnicodeUTF8));
        spnFreq->setSuffix(QApplication::translate("wndProd", " Hz", 0, QApplication::UnicodeUTF8));
        spnPeriod->setSuffix(QApplication::translate("wndProd", " ms", 0, QApplication::UnicodeUTF8));
        lblFreq->setText(QApplication::translate("wndProd", "Freq:", 0, QApplication::UnicodeUTF8));
        lblPeriod->setText(QApplication::translate("wndProd", "Period:", 0, QApplication::UnicodeUTF8));
        btnTrigger->setText(QApplication::translate("wndProd", "Trigger!", 0, QApplication::UnicodeUTF8));
        lblPreview->setText(QApplication::translate("wndProd", "Preview:", 0, QApplication::UnicodeUTF8));
        grpStatus->setTitle(QApplication::translate("wndProd", "Status", 0, QApplication::UnicodeUTF8));
        cmbStatus->clear();
        cmbStatus->insertItems(0, QStringList()
         << QApplication::translate("wndProd", "Select:", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("wndProd", "OK", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("wndProd", "Warn", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("wndProd", "Error", 0, QApplication::UnicodeUTF8)
        );
        lblStatus->setText(QApplication::translate("wndProd", "OK", 0, QApplication::UnicodeUTF8));
        lblRun->setText(QApplication::translate("wndProd", "Run Num:", 0, QApplication::UnicodeUTF8));
        lblState->setText(QApplication::translate("wndProd", "State:", 0, QApplication::UnicodeUTF8));
        lblConfig->setText(QApplication::translate("wndProd", "Config:", 0, QApplication::UnicodeUTF8));
        lblDataColl->setText(QApplication::translate("wndProd", "DataCollector:", 0, QApplication::UnicodeUTF8));
        lblEvent->setText(QApplication::translate("wndProd", "Event Num:", 0, QApplication::UnicodeUTF8));
        btnQuit->setText(QApplication::translate("wndProd", "Quit", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class wndProd: public Ui_wndProd {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EUPROD_H
