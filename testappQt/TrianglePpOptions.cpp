

// OPEN TODO:: file header


#include "TrianglePpOptions.h"


// public methods

TrianglePpOptions::TrianglePpOptions(QWidget *parent)
    : QDialog(parent)
{
   ui.setupUi(this);

   ui.minAngleLineEdit->setInputMask("");
   ui.maxAreaLineEdit->setInputMask("");

   ui.minPointCountLineEdit->setInputMask("");
   ui.maxPointCountLineEdit->setInputMask("");
}


int TrianglePpOptions::getMinAngle() const
{
   return ui.minAngleLineEdit->text().toInt();
}


int TrianglePpOptions::getMaxArea() const
{
   return ui.maxAreaLineEdit->text().toInt();
}


int TrianglePpOptions::getMinPointCount() const
{
   return ui.minPointCountLineEdit->text().toInt();
}


int TrianglePpOptions::getMaxPointCount() const
{
   return ui.maxPointCountLineEdit->text().toInt();
}


void TrianglePpOptions::fillContents(int minAngle, int maxArea, int minPoints, int maxPoints)
{
   ui.minAngleLineEdit->setText(minAngle >= 0 ? QString::number(minAngle) : "");
   ui.maxAreaLineEdit->setText(maxArea >= 0 ? QString::number(maxArea) : "");
   ui.minPointCountLineEdit->setText(minPoints >= 0 ? QString::number(minPoints) : "");
   ui.maxPointCountLineEdit->setText(maxPoints >= 0 ? QString::number(maxPoints) : "");
}
