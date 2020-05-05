

// OPEN TODO:: file header


#include "TrianglePPTest.h"
#include "TrianglePpOptions.h"

#include <tpp_interface.hpp>

#include <QMessageBox>

#include <vector>
#include <random>


using namespace tpp;


namespace {

   // impl. constants
   const int c_defaultMinPoints = 3;
   const int c_defaultMaxPoints = 100;
   const int c_defaultMinAngle = 20; // default min. angle used by the original Triangle package!

   // impl. helpers
   QPoint getDelaunayResultPoint(
         std::vector<Delaunay::Point>& delaunayInput, 
         int resultIndex, 
         const Delaunay::Point& stPoint)
   {
      double x;
      double y;

      // new vertices might have been added to enforce constraints!
      // ( aka Steiner points )
      if (resultIndex == -1)
      {
         x = stPoint[0]; // an added vertex, it's data copied to resultPoint
         y = stPoint[1];
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
      useConstraints_(false),
      triangulated_(false),
      minAngle_(-1),
      maxArea_(-1),
      minPoints_(-1),
      maxPoints_(-1)
{
   ui.setupUi(this);

   // NYI:
   //ui.tesselatePointsPushButton->hide();
   // ---
   
   ui.drawAreaWidget->setDrawMode(DrawingArea::DrawPoints);
   ui.optionsToolButton->setText(QChar(0x2630)); // trigram for the heaven (tian)

   setGenerateButtonText();

   // drawing area changes:
   connect(ui.drawAreaWidget, &DrawingArea::pointDeleted, this, &TrianglePPTest::onTriangulationPointDeleted);
}


void TrianglePPTest::on_generatePointsPushButton_clicked()
{
   switch (mode_)
   {
   case ManualMode:
      // reset
      clearDisplay();
      break;

   case AutomaticMode:
      clearDisplay();
      generateRandomPoints();
      break;

   case FromImageMode:
      clearDisplay();

      // find characteristic points in the image

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
   // remove Voronoi points
   clearVoronoiPoints();

   // reset triangulation lines
   auto drawnPoints = ui.drawAreaWidget->getPointCoordinates();
   ui.drawAreaWidget->clearImage();
   ui.drawAreaWidget->setDrawColor(Qt::blue);

   for (auto& point : drawnPoints)
   {
      ui.drawAreaWidget->drawPoint(point);
   }   

   if (drawnPoints.size() < 3)
   {
      QMessageBox::critical(this, tr("ERROR"), tr("Not enough points to triangulate!"));
      return;
   }

   // triangulate
   std::vector<Delaunay::Point> delaunayInput;
   for (auto& point : drawnPoints)
   {
      delaunayInput.push_back(Delaunay::Point(point.x(), point.y()));
   }

   Delaunay trGenerator(delaunayInput);

   if (useConstraints_)
   {
      if (minAngle_ > 0)
      {
         trGenerator.setMinAngle(minAngle_);
      }
      if (maxArea_ > 0)
      {
         trGenerator.setMaxArea(maxArea_);
      }
   }
   else
   {
      trGenerator.setMinAngle(-1);
      trGenerator.setMaxArea(-1);
   }

   trGenerator.Triangulate(useConstraints_);
   triangulated_ = true;
   
   // iterate over triangles
   for (Delaunay::fIterator fit = trGenerator.fbegin(); fit != trGenerator.fend(); ++fit)
   {
      // Steiner points?
      Delaunay::Point sp1;
      Delaunay::Point sp2;
      Delaunay::Point sp3;

      int originIdx = trGenerator.Org(fit, &sp1);
      int destIdx = trGenerator.Dest(fit, &sp2);
      int apexIdx = trGenerator.Apex(fit, &sp3);

      auto getResultPoint = [&](int index, const Delaunay::Point& dpoint)
      {
         return getDelaunayResultPoint(delaunayInput, index, dpoint);
      };

      // draw triangle
      ui.drawAreaWidget->drawLine(getResultPoint(originIdx, sp1), getResultPoint(destIdx, sp2));
      ui.drawAreaWidget->drawLine(getResultPoint(destIdx, sp2), getResultPoint(apexIdx, sp3));
      ui.drawAreaWidget->drawLine(getResultPoint(apexIdx, sp3), getResultPoint(originIdx, sp1));
   }
}


void TrianglePPTest::on_tesselatePointsPushButton_clicked()
{
   //QMessageBox::critical(this, tr("TODO"), tr("Not yet implemented!"));

   clearVoronoiPoints();

   // OPEN TODO::: clear Voronoi lines / ALL lines ???

   // get the original points
   auto drawnPoints = ui.drawAreaWidget->getPointCoordinates();

   std::vector<Delaunay::Point> delaunayInput;
   for (auto& point : drawnPoints)
   {
      delaunayInput.push_back(Delaunay::Point(point.x(), point.y()));
   }

   Delaunay trGenerator(delaunayInput);
   trGenerator.Tesselate();

   // draw Voronoi points
   ui.drawAreaWidget->setDrawColor(Qt::red);

   for (Delaunay::vvIterator fit = trGenerator.vvbegin(); fit != trGenerator.vvend(); ++fit)
   {
      // access data
      auto point = *fit;
      double x = point[0];
      double y = point[1];

      ui.drawAreaWidget->drawPoint(QPoint(x, y));
      voronoiPoints_.append(QPoint(x, y));
   }

   // ... and Voronoi edges
   for (Delaunay::veIterator fit = trGenerator.vebegin(); fit != trGenerator.veend(); ++fit)
   {
      bool finiteEdge = false;
      Delaunay::Point p1 = trGenerator.Org(fit);
      Delaunay::Point p2 = trGenerator.Dest(fit, finiteEdge);

      // access data
      double xstart = p1[0];
      double ystart = p1[1];

      if (!finiteEdge)
      {
         // an inifinite ray, thus no endpoint coordinates!
         auto rayNormalXValue = p2[0];
         auto rayNormalYValue = p2[1];
         assert(!(rayNormalXValue == 0.0 && rayNormalYValue == 0.0));

         double xend = rayNormalXValue;
         double yend = rayNormalYValue;

         // move vector to the start point
         xend += xstart;
         yend += ystart;

         // OPEN TODO:: project to the boundaries
         // ...

         ui.drawAreaWidget->drawLine(QPoint(xstart, ystart), QPoint(xend, yend));
      }
      else
      {
         double xend = p2[0];
         double yend = p2[1];

         ui.drawAreaWidget->drawLine(QPoint(xstart, ystart), QPoint(xend, yend));
      }
   }

   ui.drawAreaWidget->setDrawColor(Qt::blue);
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

   // ??? good idea?
#if 0
   // re-triangulate!
   on_triangualtePointsPushButton_clicked();
#endif
}


void TrianglePPTest::on_optionsToolButton_clicked()
{
   QMenu ctxtMenu(tr(""), this);

   QAction action1("Options", this);
   connect(&action1, &QAction::triggered, this, &TrianglePPTest::showTrianguationOptions);
   ctxtMenu.addAction(&action1);

   QAction action2("Close", this);
   connect(&action2, &QAction::triggered, this, &TrianglePPTest::close);
   ctxtMenu.addAction(&action2);
   
   ctxtMenu.exec(mapToGlobal(ui.optionsToolButton->geometry().bottomLeft()));
}


// private methods

void TrianglePPTest::onTriangulationPointDeleted(const QPoint& pos)
{
   if (!triangulated_)
   {
      return;
   }

   // find adjacent vertices, remove them...
   //  -- OR: just re-triangulate!

   on_triangualtePointsPushButton_clicked();
}


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


void TrianglePPTest::generateRandomPoints()
{
   std::random_device r;
   std::default_random_engine reng(r());

   // 1. dice out a number
   int minPoints = minPoints_ <= 0 ? c_defaultMinPoints : minPoints_;
   int maxPoints = maxPoints_ <= 0 ? c_defaultMaxPoints : maxPoints_;

   std::uniform_int_distribution<int> udistr(minPoints, maxPoints);
   int count = udistr(reng);

   statusBar()->showMessage(QString("Generated %1 points").arg(count));

   // 2. dice out points
   auto ptSize = ui.drawAreaWidget->getPointSize();

   std::uniform_int_distribution<int> udistrWidth(0 + ptSize / 2, ui.drawAreaWidget->width() - 1 - (ptSize / 2));
   std::uniform_int_distribution<int> udistrHeight(0 + ptSize / 2, ui.drawAreaWidget->height() - 1 - (ptSize / 2));

   for (int i = 0; i < count; ++i)
   {
      // OPEN TODO:: 
      //  -- check minimum distance to other points!!!

      ui.drawAreaWidget->drawPoint({ udistrWidth(reng), udistrHeight(reng) });
   }     
}


void TrianglePPTest::showTrianguationOptions()
{
   TrianglePpOptions dlg(this);

   dlg.fillContents(
         minAngle_ > 0 ? minAngle_ : c_defaultMinAngle,
         maxArea_ > 0 ? maxArea_ : -1,
         minPoints_ > 0 ? minPoints_ : c_defaultMinPoints,
         maxPoints_ > 0 ? maxPoints_ : c_defaultMaxPoints);

   dlg.exec();

   if (dlg.result() == QDialog::Accepted)
   {
      minAngle_ = (dlg.getMinAngle() == c_defaultMinAngle) ? -1 : dlg.getMinAngle();
      maxArea_ = dlg.getMaxArea();
      minPoints_ = (dlg.getMinPointCount() == c_defaultMinPoints) ? -1 : dlg.getMinPointCount();
      maxPoints_ = (dlg.getMaxPointCount() == c_defaultMaxPoints) ? -1 : dlg.getMaxPointCount();
   }
}


void TrianglePPTest::clearDisplay()
{
   ui.drawAreaWidget->clearImage();
   statusBar()->showMessage("");
   triangulated_ = false;
   ui.drawAreaWidget->setDrawColor(Qt::blue);
}


void TrianglePPTest::clearVoronoiPoints()
{
   for (auto& pt : voronoiPoints_)
   {
      ui.drawAreaWidget->clearPoint(pt);
   }

   voronoiPoints_.clear();
}
