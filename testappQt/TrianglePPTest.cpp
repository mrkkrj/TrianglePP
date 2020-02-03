#include "TrianglePPTest.h"

#include <tpp_interface.hpp>
#include <vector>

#include <QMessageBox>


using namespace tpp;


// public methods

TrianglePPTest::TrianglePPTest(QWidget *parent)
    : QMainWindow(parent),
      mode_(ManualMode)
{
   ui.setupUi(this);

   ui.drawAreaWidget->setDrawMode(DrawingArea::DrawPoints);

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
      // OPEN TODO:::
      Q_ASSERT(false && "NYI!!!");

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
   trGenerator.Triangulate();

   // iterate over triangles
   for (Delaunay::fIterator fit = trGenerator.fbegin(); fit != trGenerator.fend(); ++fit)
   {
      // access data
      int originIdx = trGenerator.Org(fit);
      int destIdx = trGenerator.Dest(fit);
      int apexIdx = trGenerator.Apex(fit);
            
      double x1 = delaunayInput[originIdx][0]; // example
      double y1 = delaunayInput[originIdx][1];

      // draw triangle
      ui.drawAreaWidget->drawLine(
         QPoint(delaunayInput[originIdx][0], delaunayInput[originIdx][1]),
         QPoint(delaunayInput[destIdx][0], delaunayInput[destIdx][1]));

      ui.drawAreaWidget->drawLine(
         QPoint(delaunayInput[destIdx][0], delaunayInput[destIdx][1]),
         QPoint(delaunayInput[apexIdx][0], delaunayInput[apexIdx][1]));

      ui.drawAreaWidget->drawLine(
         QPoint(delaunayInput[apexIdx][0], delaunayInput[apexIdx][1]),
         QPoint(delaunayInput[originIdx][0], delaunayInput[originIdx][1]));
   }


   // 2. use costraints
   
   // OPEN TODO:::::

   
}


void TrianglePPTest::on_pointModeComboBox_currentIndexChanged(int index)
{
   Q_ASSERT(ManualMode == PointGenerationMode(0));

   mode_ = PointGenerationMode(index); // 0 == ManualMode!
   setGenerateButtonText();
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
   default:
      ui.generatePointsPushButton->setText("???");
      Q_ASSERT(false && "unknown draw mode");
   }
}