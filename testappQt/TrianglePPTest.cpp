#include "TrianglePPTest.h"

#include <tpp_interface.hpp>
#include <vector>

#include <QMessageBox>


using namespace tpp;


namespace {

   // impl. helpers
   QPoint getDelaunayResultPoint(
         std::vector<Delaunay::Point>& delaunayInput, 
         int resultIndex, 
         const Delaunay::Point& resultPoint)
   {
      double x;
      double y;

      // new vertices might have been added to enforce constraints!
      if (resultIndex == -1)
      {
         x = resultPoint[0]; // an added vertex, it's data copied to resultPoint
         y = resultPoint[1];
      }
      else
      {
         x = delaunayInput[resultIndex][0];
         y = delaunayInput[resultIndex][1];
      }

      return QPoint(x, y);
   }

}


// public methods

TrianglePPTest::TrianglePPTest(QWidget *parent)
    : QMainWindow(parent),
      mode_(ManualMode),
      useConstraints_(false)
{
   ui.setupUi(this);

   ui.drawAreaWidget->setDrawMode(DrawingArea::DrawPoints);
   ui.optionsToolButton->setText(QChar(0x2630)); // trigram for heaven 

   setGenerateButtonText();
}


void TrianglePPTest::on_generatePointsPushButton_clicked()
{
   switch (mode_)
   {
   case ManualMode:
      // reset
      ui.drawAreaWidget->clearImage();
      break;

   case AutomaticMode:
      ui.drawAreaWidget->clearImage();

      // make random points

      // OPEN TODO:::--> random points
      Q_ASSERT(false && "AutomaticMode ---> NYI!!!");
      // OPEN TODO::: end ---

      break;

   case FromImageMode:
      ui.drawAreaWidget->clearImage();

      // make random points

      // OPEN TODO::: --> image processing, characteristic points
      Q_ASSERT(false && "FromImageMode ---> NYI!!!");
      // OPEN TODO::: end ---

      break;

   default:
      Q_ASSERT(false);
   }
}


void TrianglePPTest::on_triangualtePointsPushButton_clicked()
{
   // reset all lines
   auto drawnPoints = ui.drawAreaWidget->getPointCoordinates();
   ui.drawAreaWidget->clearImage();

   for (auto& point : drawnPoints)
   {
      ui.drawAreaWidget->drawPoint(point);
   }   

   if (drawnPoints.size() < 3)
   {
      QMessageBox::critical(this, tr("ERROR"), tr("Not enough points to triangulate!"));
      return;
   }

   // 1. standard triangulation
   std::vector<Delaunay::Point> delaunayInput;
   for (auto& point : drawnPoints)
   {
      delaunayInput.push_back(Delaunay::Point(point.x(), point.y()));
   }

   Delaunay trGenerator(delaunayInput);
   trGenerator.Triangulate(useConstraints_);

   // iterate over triangles
   for (Delaunay::fIterator fit = trGenerator.fbegin(); fit != trGenerator.fend(); ++fit)
   {
      // access data
      Delaunay::Point p1;
      Delaunay::Point p2;
      Delaunay::Point p3;

      int originIdx = trGenerator.Org(fit, &p1);
      int destIdx = trGenerator.Dest(fit, &p2);
      int apexIdx = trGenerator.Apex(fit, &p3);

      auto getResultPoint = [&](int index, const Delaunay::Point& dpoint)
      {
         return getDelaunayResultPoint(delaunayInput, index, dpoint);
      };

      // draw triangle
      ui.drawAreaWidget->drawLine(getResultPoint(originIdx, p1), getResultPoint(destIdx, p2));
      ui.drawAreaWidget->drawLine(getResultPoint(destIdx, p2), getResultPoint(apexIdx, p3));
      ui.drawAreaWidget->drawLine(getResultPoint(apexIdx, p3), getResultPoint(originIdx, p1));
   }
}


void TrianglePPTest::on_pointModeComboBox_currentIndexChanged(int index)
{
   Q_ASSERT(ManualMode == PointGenerationMode(0));

   mode_ = PointGenerationMode(index); // 0 == ManualMode!
   setGenerateButtonText();
}


void TrianglePPTest::on_useConstraintsCheckBox_toggled(bool checked)
{
   useConstraints_ = checked;
}


void TrianglePPTest::on_optionsToolButton_clicked()
{

   // NYI !!!!!!!!!!!!!!!!!
   // -- show options

}


// private methods

void TrianglePPTest::setGenerateButtonText()
{
   switch (mode_)
   {
   case ManualMode:
      ui.generatePointsPushButton->setText(tr("Clear Points"));
      break;
   case AutomaticMode:
      ui.generatePointsPushButton->setText(tr("Generate Points"));
      break;      
   case FromImageMode:
      ui.generatePointsPushButton->setText(tr("Find Points"));
      break;
   default:
      ui.generatePointsPushButton->setText("???");
      Q_ASSERT(false && "unknown draw mode");
   }
}
