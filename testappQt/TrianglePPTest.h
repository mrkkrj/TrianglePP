#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_TrianglePPTest.h"

class TrianglePPTest : public QMainWindow
{
    Q_OBJECT

public:
    TrianglePPTest(QWidget *parent = Q_NULLPTR);

public slots:
   void on_generatePointsPushButton_clicked();
   void on_triangualtePointsPushButton_clicked();
   void on_pointModeComboBox_currentIndexChanged(int index);

private:
   void setGenerateButtonText();

private:
    Ui::TrianglePPTestClass ui;

    enum PointGenerationMode { 
       ManualMode = 0, AutomaticMode 
    } mode_;
};
