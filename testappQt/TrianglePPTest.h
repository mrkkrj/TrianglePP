
// OPEN TODO:: file header


#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_TrianglePPTest.h"


// OPEN TODO::: class header ...

class TrianglePPTest : public QMainWindow
{
    Q_OBJECT

public:
    TrianglePPTest(QWidget *parent = nullptr);

public slots:
   void on_generatePointsPushButton_clicked();
   void on_triangualtePointsPushButton_clicked();
   void on_tesselatePointsPushButton_clicked();
   void on_pointModeComboBox_currentIndexChanged(int index);
   void on_useConstraintsCheckBox_toggled(bool checked);
   void on_optionsToolButton_clicked();

private slots:
   void onTriangulationPointDeleted(const QPoint& pos);

private:
   void setGenerateButtonText();
   void generateRandomPoints();
   void showTrianguationOptions();
   void clearDisplay();

private:
    Ui::TrianglePPTestClass ui;

    enum PointGenerationMode { 
       ManualMode = 0, AutomaticMode, FromImageMode 
    } mode_;

    bool useConstraints_;
    bool triangulated_;

    int minAngle_;
    int maxArea_;
    int minPoints_;
    int maxPoints_;
};
