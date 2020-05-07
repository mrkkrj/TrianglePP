

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

   bool minMaxAvailable = ui.constrainedDelaunayCheckBox->isChecked();
   enableMinMaxAngle(minMaxAvailable);
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


bool TrianglePpOptions::useConformingDelaunay() const
{
   bool conforming = ui.conformingDelaunayCheckBox->isChecked();
   Q_ASSERT(ui.constrainedDelaunayCheckBox->isChecked() == !conforming);

   return conforming;
}


void TrianglePpOptions::fillContents(int minAngle, int maxArea, int minPoints, int maxPoints, bool confDelaunay)
{
   ui.minAngleLineEdit->setText(minAngle >= 0 ? QString::number(minAngle) : "");
   ui.maxAreaLineEdit->setText(maxArea >= 0 ? QString::number(maxArea) : "");
   ui.minPointCountLineEdit->setText(minPoints >= 0 ? QString::number(minPoints) : "");
   ui.maxPointCountLineEdit->setText(maxPoints >= 0 ? QString::number(maxPoints) : "");
   ui.conformingDelaunayCheckBox->setChecked(confDelaunay);
   ui.constrainedDelaunayCheckBox->setChecked(!confDelaunay);
   enableMinMaxAngle(!confDelaunay);
}


void TrianglePpOptions::on_constrainedDelaunayCheckBox_clicked(bool checked)
{
   ui.conformingDelaunayCheckBox->setChecked(!checked);
   enableMinMaxAngle(checked);
}


void TrianglePpOptions::on_conformingDelaunayCheckBox_clicked(bool checked)
{
   ui.constrainedDelaunayCheckBox->setChecked(!checked);
   enableMinMaxAngle(!checked);
}


void TrianglePpOptions::enableMinMaxAngle(bool enable)
{
   ui.minAngleLineEdit->setEnabled(enable);
   ui.maxAreaLineEdit->setEnabled(enable);
}
