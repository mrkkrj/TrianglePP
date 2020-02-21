

// OPEN TODO:: file header

#pragma once

#include "ui_TrianglePpOptions.h"


// OPEN TODO::: class header ...

class TrianglePpOptions : public QDialog
{
    Q_OBJECT

public:
    TrianglePpOptions(QWidget *parent = nullptr);

    int getMinAngle() const;
    int getMaxArea() const;
    int getMinPointCount() const;
    int getMaxPointCount() const;

    void fillContents(int minAngle, int maxArea = -1, int minPoints = -1, int maxPoints = -1);

public slots:

private:
    Ui::TrianglePpOptions ui;
};
