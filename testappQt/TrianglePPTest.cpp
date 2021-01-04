

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

   const QColor c_TriangleColor = Qt::blue;
   const QColor c_VoronoiColor = Qt::red;
   const QColor c_SegmentColor = Qt::green;


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
      maxPoints_(-1),
      useConformingDelaunay_(false)
{
   ui.setupUi(this);
   
   ui.drawAreaWidget->setDrawMode(DrawingArea::DrawPoints);
   ui.optionsToolButton->setText(QChar(0x2630)); // trigram for the heaven (tian)

   setGenerateButtonText();

   // drawing area changes:
   connect(ui.drawAreaWidget, &DrawingArea::pointDeleted, this, &TrianglePPTest::onTriangulationPointDeleted);
   connect(ui.drawAreaWidget, &DrawingArea::linePointsSelected, this, &TrianglePPTest::onSegmentEndpointsSelected);
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
      //Q_ASSERT(false && "FromImageMode ---> NYI!!!");
      QMessageBox::critical(this/*qApp->activeModalWidget()*/, "ERROR", "FromImage mode ---> NYI, sorry !!!");
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

   // ...but retain the points!
   ui.drawAreaWidget->setDrawColor(c_TriangleColor);

   for (auto& point : drawnPoints)
   {
      ui.drawAreaWidget->drawPoint(point);
   }   

   // triangulate
   if (drawnPoints.size() < 3)
   {
      QMessageBox::critical(this, tr("ERROR"), tr("Not enough points to triangulate!"));
      return;
   }

   std::vector<Delaunay::Point> delaunayInput;
   for (auto& point : drawnPoints)
   {
      delaunayInput.push_back(Delaunay::Point(point.x(), point.y()));
   }

   Delaunay trGenerator(delaunayInput);
   configDelaunay(trGenerator);

   if (useConformingDelaunay_)
   {
      trGenerator.TriangulateConf();
   }
   else
   {
      trGenerator.Triangulate(useConstraints_);
   }

   triangulated_ = true;
   
   // draw
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

   // draw used constraint segments
   ui.drawAreaWidget->setDrawColor(c_SegmentColor);

   for (int i = 0; i < segmentEndpointIndexes_.size(); ++i)
   {
      auto start = segmentEndpointIndexes_[i++];
      auto end = segmentEndpointIndexes_[i];

      ui.drawAreaWidget->drawLine(drawnPoints[start], drawnPoints[end]);
   }

   ui.drawAreaWidget->setDrawColor(c_TriangleColor);
}


void TrianglePPTest::on_tesselatePointsPushButton_clicked()
{
   //QMessageBox::critical(this, tr("TODO"), tr("Not yet implemented!"));

   clearVoronoiPoints();

   // OPEN TODO::: 
   //  -- clear Voronoi lines / ALL lines ???

   // get the original points
   auto drawnPoints = ui.drawAreaWidget->getPointCoordinates();

   if (drawnPoints.size() < 3)
   {
      QMessageBox::critical(this, tr("ERROR"), tr("Not enough points to tesselate!"));
      return;
   }

   std::vector<Delaunay::Point> delaunayInput;
   for (auto& point : drawnPoints)
   {
      delaunayInput.push_back(Delaunay::Point(point.x(), point.y()));
   }

   // tesselate
   Delaunay trGenerator(delaunayInput);
   trGenerator.Tesselate(useConformingDelaunay_);

   // draw
   drawVoronoiTesselation(trGenerator);
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

   QAction action2("Info", this);
   connect(&action2, &QAction::triggered, this, &TrianglePPTest::showInfo);
   ctxtMenu.addAction(&action2);
   
   QAction action3("Close", this);
   connect(&action3, &QAction::triggered, this, &TrianglePPTest::close);
   ctxtMenu.addAction(&action3);

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


void TrianglePPTest::onSegmentEndpointsSelected(int startPointIdx, int endPointIdx)
{
   ui.drawAreaWidget->setDrawColor(c_SegmentColor);

   auto drawnPoints = ui.drawAreaWidget->getPointCoordinates();
   ui.drawAreaWidget->drawLine(drawnPoints[startPointIdx], drawnPoints[endPointIdx]);

   segmentEndpointIndexes_ << startPointIdx << endPointIdx;

   ui.drawAreaWidget->setDrawColor(c_TriangleColor);
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
         maxPoints_ > 0 ? maxPoints_ : c_defaultMaxPoints,
         useConformingDelaunay_);

   float guaranteed = 0, possible = 0;
   Delaunay::getMinAngleBoundaries(guaranteed, possible);
   dlg.setMinAngleBoundaries(guaranteed, possible);
   dlg.setSegmentPointIndexes(segmentEndpointIndexes_);

   dlg.exec();

   if (dlg.result() == QDialog::Accepted)
   {
      minAngle_ = (dlg.getMinAngle() == c_defaultMinAngle) ? -1 : dlg.getMinAngle();
      maxArea_ = dlg.getMaxArea();
      minPoints_ = (dlg.getMinPointCount() == c_defaultMinPoints) ? -1 : dlg.getMinPointCount();
      maxPoints_ = (dlg.getMaxPointCount() == c_defaultMaxPoints) ? -1 : dlg.getMaxPointCount();
      useConformingDelaunay_ = dlg.useConformingDelaunay();

      ui.useConstraintsCheckBox->setEnabled(!useConformingDelaunay_);
      if (useConformingDelaunay_)
      {
         ui.useConstraintsCheckBox->setChecked(false);
      }

      segmentEndpointIndexes_ = dlg.getSegmentPointIndexes();
   }
}


void TrianglePPTest::showInfo()
{
   QMessageBox::information(this, tr("INFO"), "A Qt-based demo for the TrianglePP library!");
}


void TrianglePPTest::clearDisplay()
{
   ui.drawAreaWidget->clearImage();
   statusBar()->showMessage("");
   
   triangulated_ = false;
   segmentEndpointIndexes_.clear(); // forget them, bound to old points!

   ui.drawAreaWidget->setDrawColor(c_TriangleColor);
}


void TrianglePPTest::clearVoronoiPoints()
{
   // do not need to re-triangulate here!
   const QSignalBlocker blocker(ui.drawAreaWidget);

   for (auto& pt : voronoiPoints_)
   {
      ui.drawAreaWidget->clearPoint(pt);
   }

   voronoiPoints_.clear();
}


void TrianglePPTest::drawVoronoiTesselation(tpp::Delaunay& trGenerator)
{
   // draw Voronoi points
   ui.drawAreaWidget->setDrawColor(c_VoronoiColor);

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
          
         // extend to the boundaries
         double slope = (yend - ystart) / (xend - xstart);
         double intercept = yend - (slope * xend);

         double xinfininty = (xend > xstart) ? 10000 : 0;
         double yinfininty = xinfininty * slope + intercept;
                  
         ui.drawAreaWidget->drawLine(QPoint(xstart, ystart), QPoint(xinfininty, yinfininty));
      }
      else
      {
         double xend = p2[0];
         double yend = p2[1];

         ui.drawAreaWidget->drawLine(QPoint(xstart, ystart), QPoint(xend, yend));
      }
   }

   ui.drawAreaWidget->setDrawColor(c_TriangleColor);
}


void TrianglePPTest::configDelaunay(tpp::Delaunay& trGenerator)
{
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

   if (!trGenerator.setSegmentConstraint(segmentEndpointIndexes_.toStdVector()))
   {
      QMessageBox::critical(this, tr("ERROR"), tr("Incorrect segment constraints, ignoring!"));
   }
}

