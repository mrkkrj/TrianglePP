

// OPEN TODO:: file header


#include "TrianglePpOptions.h"

#include <QDoubleValidator>
#include <QPushButton>
#include <QMessageBox>


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

   normalPal_ = palette();
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


void TrianglePpOptions::setMinAngleBoundaries(float maxOk, float maxWarning)
{
   minAngleOk_ = maxOk;
   miAngleWaning_ = maxWarning;

   // allow up to a warning!!!

   auto validator = new QDoubleValidator(0.0, maxWarning, 2, this);
   validator->setLocale(QLocale::C); // always use point!
   ui.minAngleLineEdit->setValidator(nullptr);
   ui.minAngleLineEdit->setValidator(validator);
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


void TrianglePpOptions::on_minAngleLineEdit_textChanged()
{
   bool ok = ui.minAngleLineEdit->hasAcceptableInput();
   if (!ok)
   {
      QPalette errorPal;
      errorPal.setColor(QPalette::Base, Qt::red);
      errorPal.setColor(QPalette::Text, Qt::white);

      ui.minAngleLineEdit->setPalette(errorPal);
   }
   else
   {
      ui.minAngleLineEdit->setPalette(normalPal_);
   }

   auto okBttn = ui.buttonBox->button(QDialogButtonBox::Ok);
   okBttn->setEnabled(ok);
}


void TrianglePpOptions::on_minAngleLineEdit_editingFinished()
{
   if (minAngleOk_ <= 0.0)
   {
      return;
   }

   double val = ui.minAngleLineEdit->text().toDouble();

   if (val > minAngleOk_)
   {
      QMessageBox::warning(this, "", 
               QString("Caution!\n\nThe triangualtion will very probably terminate, "
                       "but mathematically this is only guaranteed up to: \n\n  minAngle=%1 deg\n")
                  .arg(minAngleOk_));
   }
}


void TrianglePpOptions::enableMinMaxAngle(bool enable)
{
   ui.minAngleLineEdit->setEnabled(enable);
   ui.maxAreaLineEdit->setEnabled(enable);
}
