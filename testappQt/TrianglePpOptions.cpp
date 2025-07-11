/**
  @file  TrianglePpOptions.cpp
  @brief Implementation of the TrianglePpOptions class

  @author  Marek Krajewski (mrkkrj), www.ib-krajewski.de
*/

#include "TrianglePpOptions.h"

#include <QDoubleValidator>
#include <QPushButton>
#include <QMessageBox>
#include <QColorDialog>


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


bool TrianglePpOptions::applyQualityConstraints() const 
{
  return ui.applyQualityConstraintsCheckBox->isChecked();
}


bool TrianglePpOptions::includeConvexHull() const 
{
  return !ui.removeConcavitiesCheckBox->isChecked();
}


bool TrianglePpOptions::seperateSegmentColor() const
{
   return ui.seperateSegmentColorCheckBox->isChecked();
}


void TrianglePpOptions::getDelaunayColors(QColor& vertexColor, QColor& segmentColor,  QColor& voronoiColor) const
{
   vertexColor = vertexColor_;
   segmentColor = segmentColor_;
   voronoiColor =  voronoiColor_;
}


QVector<int> TrianglePpOptions::getSegmentPointIndexes() const
{
   if (ui.segmentPointsLineEdit->text().isEmpty())
   {
      return QVector<int>();
   }

   auto values = ui.segmentPointsLineEdit->text().split(",");
   QVector<int> ret;

   for (auto& v : values)
   {
      ret << v.toInt();
   }

   return ret;
}


void TrianglePpOptions::fillContents(
    int minAngle, int maxArea, int minPoints, int maxPoints, 
    bool qualityConstr, bool confDelaunay, bool convexHull, bool diffColorForSegments)
{
   ui.minAngleLineEdit->setText(minAngle >= 0 ? QString::number(minAngle) : "");
   ui.maxAreaLineEdit->setText(maxArea >= 0 ? QString::number(maxArea) : "");
   ui.minPointCountLineEdit->setText(minPoints >= 0 ? QString::number(minPoints) : "");
   ui.maxPointCountLineEdit->setText(maxPoints >= 0 ? QString::number(maxPoints) : "");

   ui.applyQualityConstraintsCheckBox->setChecked(qualityConstr);
   ui.conformingDelaunayCheckBox->setChecked(confDelaunay);
   ui.constrainedDelaunayCheckBox->setChecked(!confDelaunay);
   enableMinMaxAngle(!confDelaunay);

   ui.removeConcavitiesCheckBox->setChecked(!convexHull);
   ui.seperateSegmentColorCheckBox->setChecked(diffColorForSegments);
}


void TrianglePpOptions::fillColors(const QColor& vertexColor, const QColor& segmentColor, const QColor& voronoiColor)
{
   ui.vertexColorButton->setPalette(QPalette(vertexColor));
   vertexColor_ = vertexColor;

   auto segmentColorShown = ui.seperateSegmentColorCheckBox->isChecked() ? segmentColor : vertexColor;
   segmentColor_ = segmentColor;

   ui.segmentColorButton->setPalette(QPalette(segmentColorShown));
   ui.segmentColorButton->setEnabled(ui.seperateSegmentColorCheckBox->isChecked());

   ui.voronoiColorButton->setPalette(QPalette(voronoiColor));
   voronoiColor_ = voronoiColor;   
}


void TrianglePpOptions::setMinAngleBoundaries(float maxOk, float maxWarning)
{
   minAngleOk_ = maxOk;
   miAngleWarning_ = maxWarning;

   // allow up to a warning!!!

   auto validator = new QDoubleValidator(0.0, maxWarning, 2, this);
   validator->setLocale(QLocale::C); // always use point as separator!
   ui.minAngleLineEdit->setValidator(nullptr);
   ui.minAngleLineEdit->setValidator(validator);
}


void TrianglePpOptions::setSegmentPointIndexes(const QVector<int>& segmentEndpoints)
{
   QList<QString> values;

   for (auto& pt : segmentEndpoints)
   {
      values << QString::number(pt);
   }

   ui.segmentPointsLineEdit->setText(values.join(", "));
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


void TrianglePpOptions::on_applyQualityConstraintsCheckBox_clicked(bool checked) 
{
   ui.applyQualityConstraintsCheckBox->setChecked(checked);
   if (checked)
   {
      ui.conformingDelaunayCheckBox->setChecked(false);
   }

   enableMinMaxAngle(!checked);
   ui.conformingDelaunayCheckBox->setEnabled(!checked);
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
   if (minAngleOk_ < 0.0)
   {
      return;
   }

   double val = ui.minAngleLineEdit->text().toDouble();

   if (val > minAngleOk_)
   {
      QMessageBox::warning(this, "Triangle++", 
               QString("Caution!\n\nThe triangualtion will very probably terminate, "
                       "but mathematically this is only guaranteed up to: \n\n  minAngle=%1 deg\n")
                  .arg(minAngleOk_));
   }
}


void TrianglePpOptions::on_segmentPointsLineEdit_editingFinished()
{
   QString input = ui.segmentPointsLineEdit->text();

   if (input.isEmpty())
   {
      return; // OK, resetted
   }

   // OPEN TODO::: better regex!!!

   QRegularExpression r("{[0-9]|,| }*");
   auto match = r.match(input);

   if (!match.hasMatch())
   {
      QMessageBox::critical(this, "Triangle++",
         QString("Error!\n\n The required format is: number, number, number "));
   }
}


void TrianglePpOptions::on_removeConcavitiesCheckBox_clicked(bool checked) 
{
   ui.removeConcavitiesCheckBox->setChecked(checked);
}


void TrianglePpOptions::on_seperateSegmentColorCheckBox_clicked(bool checked)
{
   ui.seperateSegmentColorCheckBox->setChecked(checked);
   fillColors(vertexColor_, segmentColor_, voronoiColor_);
}


void TrianglePpOptions::on_vertexColorButton_clicked()
{
   QColorDialog dlg;

   if(dlg.exec() == QDialog::Accepted)
   {
      vertexColor_ = dlg.currentColor();
      ui.vertexColorButton->setPalette(QPalette(vertexColor_));
   }
}


void TrianglePpOptions::on_segmentColorButton_clicked()
{
   QColorDialog dlg;

   if(dlg.exec() == QDialog::Accepted)
   {
      segmentColor_ = dlg.currentColor();
      ui.segmentColorButton->setPalette(QPalette(segmentColor_));
   }
}


void TrianglePpOptions::on_voronoiColorButton_clicked()
{
   QColorDialog dlg;

   if(dlg.exec() == QDialog::Accepted)
   {
      voronoiColor_ = dlg.currentColor();
      ui.voronoiColorButton->setPalette(QPalette(voronoiColor_));
   }
}


void TrianglePpOptions::on_restoreColorsButton_clicked()
{
   // OPEN TODO:: getDefaultColors from settings ??? !!!!
   
   vertexColor_ = Qt::blue;
   ui.vertexColorButton->setPalette(QPalette(vertexColor_));

   segmentColor_ = "limegreen";
   ui.segmentColorButton->setPalette(QPalette(segmentColor_));

   voronoiColor_ = Qt::red;
   ui.voronoiColorButton->setPalette(QPalette(voronoiColor_));

   ui.seperateSegmentColorCheckBox->setChecked(true);
}


void TrianglePpOptions::enableMinMaxAngle(bool enable)
{
   ui.minAngleLineEdit->setEnabled(enable);
   ui.maxAreaLineEdit->setEnabled(enable);
}
