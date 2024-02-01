/**
  @file  TrianglePPDemoApp.cpp
  @brief Implementation of the TrianglePPDemoApp class

  @author  Marek Krajewski (mrkkrj), www.ib-krajewski.de
*/

#include "TrianglePPDemoApp.h"
#include "TrianglePpOptions.h"

#include <tpp_interface.hpp>

#include <QMessageBox>
#include <QFileDialog>
#include <QPixmap>

#include <vector>
#include <random>
#include <cassert>

// debug support
//#define TRIANGLE_DETAIL_DEBUG


using namespace tpp;


namespace {

   // impl. constants
   const int c_defaultMinPoints = 3;
   const int c_defaultMaxPoints = 100;
   const int c_defaultMinAngle = 20; // default min. angle used by the original Triangle package!

   const QColor c_TriangleColor = Qt::blue;
   const QColor c_VoronoiColor = Qt::red;
   const QColor c_SegmentColor = "limegreen";
   const QColor c_HoleMarkerColor = "limegreen";
   const QColor c_RegionMarkerColor = Qt::darkMagenta;

   // impl. helpers
   QPoint getDelaunayResultPoint(
         const Delaunay& trGenerator,
         int resultIndex,
         const Delaunay::Point& steinerPt)
   {
      double x;
      double y;

      // new vertices might have been added to enforce constraints!
      // ( aka Steiner points )
      if (resultIndex == -1)
      {
         x = steinerPt[0]; // an added vertex, it's data copied to resultPoint
         y = steinerPt[1];
      }
      else
      {
          // point from original data
          x = trGenerator.pointAtVertexId(resultIndex)[0];
          y = trGenerator.pointAtVertexId(resultIndex)[1];
      }

      return QPoint(x, y);
   }


   std::vector<Delaunay::Point> toTppPointVector(const QVector<QPoint>& qPoints)
   {
      std::vector<Delaunay::Point> tppPoints;

      for (auto& point : qPoints)
      {
         tppPoints.push_back(Delaunay::Point(point.x(), point.y()));
      }

      return tppPoints;
   }

}


// public methods

TrianglePPDemoApp::TrianglePPDemoApp(QWidget *parent)
    : QMainWindow(parent),
      zoomInAct_(nullptr),
      zoomOutAct_(nullptr),
      mode_(ManualMode),
      useConstraints_(false),
      triangulated_(false),
      tesselated_(false),
      minAngle_(-1),
      maxArea_(-1),
      minPoints_(-1),
      maxPoints_(-1),
      useConformingDelaunay_(false),
      includeConvexHull_(true),
      seperateSegmentColor_(true),
      lastFileDir_(".")
{
   ui.setupUi(this);
  
   ui.drawAreaWidget->setDrawMode(DrawingArea::DrawPoints);
   ui.optionsToolButton->setText(QChar(0x2630)); // trigram for the heaven (tian)     
   
   ui.pointModeComboBox->setCurrentIndex(AutomaticMode);
   ui.hideMarkersCheckBox->hide();

   zoomInAct_ = new QAction(this);
   zoomInAct_->setShortcut(QKeySequence::ZoomIn);
   addAction(zoomInAct_);
   zoomOutAct_ = new QAction(this);
   zoomOutAct_->setShortcut(QKeySequence::ZoomOut);
   addAction(zoomOutAct_);

   // drawing area changes:
   connect(ui.drawAreaWidget, &DrawingArea::pointDeleted, this, &TrianglePPDemoApp::onTriangulationPointDeleted);
   connect(ui.drawAreaWidget, &DrawingArea::linePointsSelected, this, &TrianglePPDemoApp::onSegmentEndpointsSelected);
   connect(ui.drawAreaWidget, &DrawingArea::pointChangedToHoleMarker, this, &TrianglePPDemoApp::onPointChangedToHoleMarker);
   connect(ui.drawAreaWidget, &DrawingArea::pointMoved, this, &TrianglePPDemoApp::onTriangulationPointMoved);

   connect(zoomInAct_, &QAction::triggered, this, &TrianglePPDemoApp::zoomIn);
   connect(zoomOutAct_, &QAction::triggered, this, &TrianglePPDemoApp::zoomOut);
}


void TrianglePPDemoApp::on_generatePointsPushButton_clicked()
{
   clearDisplay();

   switch (mode_)
   {
   case ManualMode:
      // reset only
      break;
   case AutomaticMode:
      generateRandomPoints();
      break;
   case FromImageMode:
      // find characteristic points in the image

      // OPEN TODO::: --> image processing, characteristic points
      //Q_ASSERT(false && "FromImageMode ---> NYI!!!");
      QMessageBox::critical(this/*qApp->activeModalWidget()*/, "ERROR", "FromImage mode ---> NYI, sorry !!!");
      // OPEN TODO::: end ---

      break;
   case FromFileMode:
      readFromFile();
      break;
   case Example1Mode:
      showExample1();
      break;
   case Example2Mode:
      showExample2();
      break;
   default:
      Q_ASSERT(false);
   }
}


void TrianglePPDemoApp::on_triangualtePointsPushButton_clicked()
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
      QMessageBox::critical(this, tr("Triangle++"), tr("Not enough points to triangulate!"));
      return;
   }

   std::vector<Delaunay::Point> delaunayInput;
   for (auto& point : drawnPoints)
   {
      // TEST::: ---> OLD...
      //if (isHoleMarker(point))
      //   continue;

      delaunayInput.push_back(Delaunay::Point(point.x(), point.y()));
   }

#ifdef TRIANGLE_DETAIL_DEBUG
   auto trace = tpp::Debug;
#else
   auto trace = tpp::None;
#endif

   tpp::Delaunay trGenerator(delaunayInput);
   configDelaunay(trGenerator);
   
   try
   {
      if (useConformingDelaunay_)
      {
         trGenerator.TriangulateConf(useConstraints_, trace);
      }
      else
      {
         trGenerator.Triangulate(useConstraints_, trace);
      }
   }
   catch (std::exception& e)
   {
      statusBar()->showMessage(tr("Triangulate threw an exception, txt=\"%1\"").arg(e.what()));
      return;
   }

   // draw
   triangulated_ = true;
   tesselated_ = false;
   
   drawTriangualtion(trGenerator, drawnPoints);
}


void TrianglePPDemoApp::on_tesselatePointsPushButton_clicked()
{
   clearVoronoiPoints();
   
   // get the original points
   auto drawnPoints = ui.drawAreaWidget->getPointCoordinates();

   if (drawnPoints.size() < 3)
   {
      QMessageBox::critical(this, tr("Triangle++"), tr("Not enough points to tesselate!"));
      return;
   }

   std::vector<Delaunay::Point> delaunayInput;
   for (auto& point : drawnPoints)
   {
      delaunayInput.push_back(Delaunay::Point(point.x(), point.y()));
   }

   // tesselate
   tpp::Delaunay trGenerator(delaunayInput);
   trGenerator.Tesselate(useConformingDelaunay_);

   tesselated_ = true;

   // draw
   if (!triangulated_)
   {
      // reset Voronoi lines
      ui.drawAreaWidget->clearImage();

      // ...but retain the points!
      ui.drawAreaWidget->setDrawColor(c_TriangleColor);

      for (auto& point : drawnPoints)
      {
         ui.drawAreaWidget->drawPoint(point);
      }
   }

   drawVoronoiTesselation(trGenerator);
}


void TrianglePPDemoApp::on_pointModeComboBox_currentIndexChanged(int index)
{
   Q_ASSERT(ManualMode == PointGenerationMode(0));

   mode_ = PointGenerationMode(index); // 0 == ManualMode!
   setGenerateButtonText();

   // some immediate action needed?
   switch (mode_)
   {
   case FromImageMode:      
      on_generatePointsPushButton_clicked(); // will just say that's not implemented
      break;

   case FromFileMode:
      clearDisplay();
      readFromFile();
      break;

   case Example1Mode:
       clearDisplay();
       showExample1();
       break;

   case Example2Mode:
       clearDisplay();
       showExample2();
       break;

   default:
       break;
   }
}


void TrianglePPDemoApp::on_useConstraintsCheckBox_toggled(bool checked)
{
   useConstraints_ = checked;

   if (triangulated_)
   {
      on_triangualtePointsPushButton_clicked();
   }
}


void TrianglePPDemoApp::on_hideMarkersCheckBox_toggled(bool checked)
{
   // repaint all holes
   QColor holeColor = checked ? Qt::white : c_HoleMarkerColor; // OPEN TODO::: holeMarkerColor()

   for (auto& hole : holePoints_)
   {
      drawHoleMarker(hole, holeColor);
   }

   // plus region markers
   QColor regionColor = ui.hideMarkersCheckBox->isChecked() ? Qt::white : c_RegionMarkerColor; // OPEN TODO::: regionMarkerColor()

   for (auto& point : regionPoints_)
   {
      drawRegionMarker(point, regionColor);
   }

   ui.drawAreaWidget->setDrawColor(c_TriangleColor);
}


void TrianglePPDemoApp::on_optionsToolButton_clicked()
{
   QMenu ctxtMenu(tr(""), this);
   
   QAction action01("Save to File", this);
   connect(&action01, &QAction::triggered, this, &TrianglePPDemoApp::writeToFile);
   ctxtMenu.addAction(&action01);

   QAction action1("Options", this);
   connect(&action1, &QAction::triggered, this, &TrianglePPDemoApp::showTrianguationOptions);
   ctxtMenu.addAction(&action1);

   QAction action2("Info", this);
   connect(&action2, &QAction::triggered, this, &TrianglePPDemoApp::showInfo);
   ctxtMenu.addAction(&action2);

   QAction action31("Zoom In (Ctrl +)", this);
   action31.setEnabled(ui.drawAreaWidget->hasPoints());
   connect(&action31, &QAction::triggered, this, &TrianglePPDemoApp::zoomIn);
   ctxtMenu.addAction(&action31);

   QAction action32("Zoom Out (Ctrl -)", this);
   action32.setEnabled(ui.drawAreaWidget->hasPoints());
   connect(&action32, &QAction::triggered, this, &TrianglePPDemoApp::zoomOut);
   ctxtMenu.addAction(&action32);

   QAction action4("Close", this);
   connect(&action4, &QAction::triggered, this, &TrianglePPDemoApp::close);
   ctxtMenu.addAction(&action4);

   ctxtMenu.exec(mapToGlobal(ui.optionsToolButton->geometry().bottomLeft()));
}


// private methods

void TrianglePPDemoApp::onTriangulationPointDeleted(const QPoint& pos)
{
   if (holePoints_.contains(pos))
   {
      holePoints_.removeOne(pos);

      if (holePoints_.empty())
      {
         ui.hideMarkersCheckBox->hide();
      }

      if (!triangulated_)
      {
         // overpaint "H" with white!
         //  --> OPEN TODO:: not quite working, some outline still visible!
         drawHoleMarker(pos, Qt::white);
      }
   }

   if (!triangulated_)
   {
      return;
   }

   // find adjacent vertices, remove them...
   //  -- OR: just re-triangulate!
   on_triangualtePointsPushButton_clicked();
}


void TrianglePPDemoApp::onSegmentEndpointsSelected(int startPointIdx, int endPointIdx)
{
   ui.drawAreaWidget->setDrawColor(segmentColor());

   auto drawnPoints = ui.drawAreaWidget->getPointCoordinates();
   ui.drawAreaWidget->drawLine(drawnPoints[startPointIdx], drawnPoints[endPointIdx]);

   Q_ASSERT(startPointIdx >= 0 && endPointIdx >= 0);
   Q_ASSERT(startPointIdx < drawnPoints.size()&& endPointIdx < drawnPoints.size());

   segmentEndpointIndexes_ << startPointIdx << endPointIdx;

   ui.drawAreaWidget->setDrawColor(c_TriangleColor);
}


void TrianglePPDemoApp::onPointChangedToHoleMarker(int pointIdx, const QPoint& pos)
{
   Q_UNUSED(pointIdx);

   drawHoleMarker(pos, c_HoleMarkerColor);
   ui.drawAreaWidget->setDrawColor(c_TriangleColor);

   holePoints_ << pos;

   ui.hideMarkersCheckBox->show();
}


void TrianglePPDemoApp::onPointChangedToRegionMarker(int pointIdx, const QPoint& pos)
{
   Q_UNUSED(pointIdx);

   drawRegionMarker(pos, c_RegionMarkerColor);
   ui.drawAreaWidget->setDrawColor(c_TriangleColor);

   regionPoints_ << pos;

   ui.hideMarkersCheckBox->show(); // OPEN TODO::: hideMarkersCheckBox
}


void TrianglePPDemoApp::onTriangulationPointMoved(const QPoint& pos1, const QPoint& pos2)
{
   Q_UNUSED(pos1);
   Q_UNUSED(pos2);

   bool wasTesselated = tesselated_;

   if (triangulated_)
   {
      on_triangualtePointsPushButton_clicked();
   }

   if (wasTesselated)
   {
      on_tesselatePointsPushButton_clicked();
   }
}


void TrianglePPDemoApp::setGenerateButtonText()
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
   case FromFileMode:
      ui.generatePointsPushButton->setText(tr("Open File"));
      break;
   case Example1Mode:
       ui.generatePointsPushButton->setText(tr("Read Example"));
       break;
   case Example2Mode:
       ui.generatePointsPushButton->setText(tr("Read Example"));
       break;
   default:
      ui.generatePointsPushButton->setText("???");
      Q_ASSERT(false && "unknown draw mode");
   }
}


void TrianglePPDemoApp::generateRandomPoints()
{
   std::random_device r;
   std::default_random_engine reng(r());

   // 1. dice out a number
   int minPoints = minPoints_ <= 0 ? c_defaultMinPoints : minPoints_;
   int maxPoints = maxPoints_ <= 0 ? c_defaultMaxPoints : maxPoints_;

   std::uniform_int_distribution<int> udistr(minPoints, maxPoints);
   int count = udistr(reng);

   statusBar()->showMessage(tr("Generated %1 points").arg(count));

   // 2. dice out points
   auto ptSize = ui.drawAreaWidget->getPointSize();

   std::uniform_int_distribution<int> distrWidth(0 + ptSize / 2, ui.drawAreaWidget->width() - 1 - (ptSize / 2));
   std::uniform_int_distribution<int> distrHeight(0 + ptSize / 2, ui.drawAreaWidget->height() - 1 - (ptSize / 2));

   for (int i = 0; i < count; ++i)
   {
      // OPEN TODO:: 
      //  -- check minimum distance to other points ????

      ui.drawAreaWidget->drawPoint({distrWidth(reng), distrHeight(reng)});
   }     
}


void TrianglePPDemoApp::showExample1()
{
   // "CDT triangulation" data from trpp_tests.cpp
   //  - see "example constr segments.jpg" for visualisation!

   // draw points: 
   std::vector<Point> constrDelaunayInput;

   constrDelaunayInput.push_back(Point{ 0, 0 });
   constrDelaunayInput.push_back(Point{ 0, 1 });
   constrDelaunayInput.push_back(Point{ 0, 3 });
   constrDelaunayInput.push_back(Point{ 2, 0 });
   constrDelaunayInput.push_back(Point{ 4, 1.25 });
   constrDelaunayInput.push_back(Point{ 4, 3 });
   constrDelaunayInput.push_back(Point{ 6, 0 });
   constrDelaunayInput.push_back(Point{ 8, 1.25 });
   constrDelaunayInput.push_back(Point{ 9, 0 });
   constrDelaunayInput.push_back(Point{ 9, 0.75 });
   constrDelaunayInput.push_back(Point{ 9, 3 });

   float offsetX = 20;
   float offsetY = 20;
   float scaleFactor = 80;

   drawPoints(constrDelaunayInput, offsetX, offsetY, scaleFactor);
   statusBar()->showMessage(tr("Created %1 Example-1 points").arg(constrDelaunayInput.size()));

   // draw constrainig segment 
   std::vector<Point> constrDelaunaySegment;

   constrDelaunaySegment.push_back(Point(0, 1));
   constrDelaunaySegment.push_back(Point(9, 0.75));

   drawSegments(constrDelaunaySegment, constrDelaunayInput);
}


void TrianglePPDemoApp::showExample2()
{
    // "Planar Straight Line Graph (PSLG) triangulation" data from trpp_tests.cpp
    //   - letter A, as in Triangle's documentation but simplified (https://www.cs.cmu.edu/~quake/triangle.defs.html#dt)

    // draw points
    std::vector<Point> pslgDelaunayInput;

    pslgDelaunayInput.push_back(Point(0, 0));
    pslgDelaunayInput.push_back(Point(1, 0));
    pslgDelaunayInput.push_back(Point(3, 0));
    pslgDelaunayInput.push_back(Point(4, 0));
    pslgDelaunayInput.push_back(Point(1.5f, 1));
    pslgDelaunayInput.push_back(Point(2.5f, 1));
    pslgDelaunayInput.push_back(Point(1.6f, 1.5f));
    pslgDelaunayInput.push_back(Point(2.4f, 1.5f));
    pslgDelaunayInput.push_back(Point(2, 2));
    pslgDelaunayInput.push_back(Point(2, 3));

    flipPoints(pslgDelaunayInput);

    float offsetX = 20;
    float offsetY = 20;
    float scaleFactor = 80;       

    drawPoints(pslgDelaunayInput, offsetX, offsetY, scaleFactor);
    statusBar()->showMessage(tr("Created %1 Example-2 points").arg(pslgDelaunayInput.size()));

    // draw segments
    std::vector<int> pslgSegmentEndpoints;

    auto addSegment = [&](int start, int end) {
        pslgSegmentEndpoints.push_back(start);
        pslgSegmentEndpoints.push_back(end);
    };

    // outer outline
    addSegment(0, 9);
    addSegment(9, 3);
    addSegment(3, 2);
    addSegment(2, 5);
    addSegment(5, 4);
    addSegment(4, 1);
    addSegment(1, 0);

    // inner outline
    addSegment(6, 8);
    addSegment(8, 7);
    addSegment(7, 6);

    drawSegments(pslgSegmentEndpoints);

    // OPEN TODO:: draw holes
        // ....
}


void TrianglePPDemoApp::showTrianguationOptions()
{
   TrianglePpOptions dlg(this);

   dlg.fillContents(
         minAngle_ >= 0 ? minAngle_ : c_defaultMinAngle,
         maxArea_ > 0 ? maxArea_ : -1,
         minPoints_ > 0 ? minPoints_ : c_defaultMinPoints,
         maxPoints_ > 0 ? maxPoints_ : c_defaultMaxPoints,
         useConformingDelaunay_,
         includeConvexHull_,
         seperateSegmentColor_);

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
      includeConvexHull_ = dlg.includeConvexHull();
      seperateSegmentColor_ = dlg.seperateSegmentColor();
   }
}


void TrianglePPDemoApp::showInfo()
{
   QMessageBox about;

   about.setText("TrianglePPDemoApp is a Qt-based demo for the TrianglePP library by @mrkkrj.");
   about.setInformativeText(
               "It can create:\n"
               " - Delaunay triangulations\n"
               " - quality Delaunay triangulations\n"
               " - constrained Delaunay triangulations\n"
               " - Voronoi tesselations");

   about.setDetailedText(
               "The Triangle++ library (aka TrianglePP) is an updated version of Piyush Kumar's C++/OO wrapper "
               "for the original 2005 J.P. Shevchuk's Triangle package, which was a **winner**"
               " of the 2003 James Hardy Wilkinson Prize in Numerical Software (sic!).\n\n"
               " - The wrapped Triangle library was written in plain old C, its documentation can be found at:\n"
               "    http://www.cs.cmu.edu/~quake/triangle.html.\n\n"
               " - For backgroud info on the original implementation see:\n"
               "    \"Triangle: Engineering a 2D Quality Mesh Generator and Delaunay Triangulator\" by J.P. Shewchuk: http://www.cs.cmu.edu/~quake-papers/triangle.ps.\n\n"
               " - Algorithm used for DCT construction:\n"
               "    Shewchuk, J.R., Brown, B.C, \"Fast segment insertion and incremental construction of constrained Delaunay triangulations\", Computational Geometry,"
               " Volume 48, Issue 8, September 2015, Pages 554-574 - https://doi.org/10.1016/j.comgeo.2015.04.006");

   about.setWindowTitle("About Triangle++ Demo");
   about.setIconPixmap(QPixmap(":/TrianglePPDemo/triangle-PP-sm.jpg"));

#ifdef Q_OS_WINDOWS
   about.setWindowIcon(QIcon(":/TrianglePPDemo/triangle-PP-ico.ico"));
#else
   about.setWindowIcon(QIcon(":/TrianglePPDemo/triangle-PP-ico.jpg"));
#endif
   
   about.setStandardButtons(QMessageBox::Ok);
   about.setDefaultButton(QMessageBox::Ok);   
   about.show();
   about.exec();
}


void TrianglePPDemoApp::clearDisplay()
{
   ui.drawAreaWidget->clearImage();
   statusBar()->showMessage("");
   
   segmentEndpointIndexes_.clear();  // forget them, bound to old points!
   holePoints_.clear();
   regionPoints_.clear();
   regionMaxAreas_.clear();

   triangulated_ = false;
   tesselated_ = false;

   ui.drawAreaWidget->setDrawColor(c_TriangleColor);
}


void TrianglePPDemoApp::clearVoronoiPoints()
{
   // do not need to re-triangulate here!
   const QSignalBlocker blocker(ui.drawAreaWidget);

   for (auto& pt : voronoiPoints_)
   {
      ui.drawAreaWidget->clearPoint(pt);
   }

   voronoiPoints_.clear();
}


void TrianglePPDemoApp::drawTriangualtion(tpp::Delaunay& trGenerator, QVector<QPoint>& pointsOnScreen)
{
    // draw triangles
    for (tpp::FaceIterator fit = trGenerator.fbegin(); fit != trGenerator.fend(); ++fit)
    {
       // Steiner points?
       Delaunay::Point sp1;
       Delaunay::Point sp2;
       Delaunay::Point sp3;

       int originIdx = fit.Org(&sp1);
       int destIdx = fit.Dest(&sp2);
       int apexIdx = fit.Apex(&sp3);

       auto getResultPoint = [&](int index, const Delaunay::Point& dpoint)
       {
           return getDelaunayResultPoint(trGenerator, index, dpoint);
       };

       // draw triangle
       ui.drawAreaWidget->drawLine(getResultPoint(originIdx, sp1), getResultPoint(destIdx, sp2));
       ui.drawAreaWidget->drawLine(getResultPoint(destIdx, sp2), getResultPoint(apexIdx, sp3));
       ui.drawAreaWidget->drawLine(getResultPoint(apexIdx, sp3), getResultPoint(originIdx, sp1));
    }

    // draw used constraint segments
    ui.drawAreaWidget->setDrawColor(segmentColor());

    for (int i = 0; i < segmentEndpointIndexes_.size(); ++i)
    {
       auto start = segmentEndpointIndexes_[i++];
       auto end = segmentEndpointIndexes_[i];
       ui.drawAreaWidget->drawLine(pointsOnScreen[start], pointsOnScreen[end]);
    }

    // ... and hole+region markers
    on_hideMarkersCheckBox_toggled(ui.hideMarkersCheckBox->isChecked());

    // ready
    statusBar()->showMessage(tr("Created %1 triangles").arg(trGenerator.triangleCount()));
}


void TrianglePPDemoApp::drawVoronoiTesselation(tpp::Delaunay& trGenerator)
{
   // draw Voronoi points
   ui.drawAreaWidget->setDrawColor(c_VoronoiColor);

   for (tpp::VoronoiVertexIterator iter = trGenerator.vvbegin(); iter != trGenerator.vvend(); ++iter)
   {
      // access data
      auto point = *iter;
      double x = point[0];
      double y = point[1];

      ui.drawAreaWidget->drawPoint(QPoint(x, y));
      voronoiPoints_.append(QPoint(x, y));
   }

   // ... and Voronoi edges
   for (tpp::VoronoiEdgeIterator iter = trGenerator.vebegin(); iter != trGenerator.veend(); ++iter)
   {
      bool finiteEdge = false;
      Delaunay::Point p1 = iter.Org();
      Delaunay::Point p2 = iter.Dest(finiteEdge);

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


void TrianglePPDemoApp::configDelaunay(tpp::Delaunay& trGenerator)
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

      if (!regionPoints_.empty())
      {
         trGenerator.setRegionsConstraint(toTppPointVector(regionPoints_), regionMaxAreas_.toStdVector());         
      }
   }
   else
   {
      trGenerator.setMinAngle(-1);
      trGenerator.setMaxArea(-1);
   }

   trGenerator.useConvexHullWithSegments(includeConvexHull_);

   if (!trGenerator.setSegmentConstraint(segmentEndpointIndexes_.toStdVector()))
   {
      QMessageBox::critical(this, tr("Triangle++"), tr("Incorrect segment constraints, ignoring!"));
   }

   trGenerator.setHolesConstraint(toTppPointVector(holePoints_));   
}


bool TrianglePPDemoApp::isHoleMarker(const QPoint& point) const
{
   return holePoints_.contains(point);
}


void TrianglePPDemoApp::drawHoleMarker(const QPoint& pos, const QColor& color)
{
   auto currMode = ui.drawAreaWidget->getDrawMode();

   ui.drawAreaWidget->setDrawColor(color);
   QFont f;
   f.setPixelSize(22); // OPEN TODO:: adapt to the size of viewport!!!!

   ui.drawAreaWidget->drawText(pos * 0.96, "H", &f);
      
   ui.drawAreaWidget->setDrawMode(DrawingArea::DrawHoleMarker);
   ui.drawAreaWidget->drawPoint(pos);
   ui.drawAreaWidget->setDrawMode(currMode);
}


// OPEN TODO::: drawMarker() generic methos?????

void TrianglePPDemoApp::drawRegionMarker(const QPoint& pos, const QColor& color)
{
   auto currMode = ui.drawAreaWidget->getDrawMode();

   ui.drawAreaWidget->setDrawColor(color);
   QFont f;
   f.setPixelSize(22);

   ui.drawAreaWidget->drawText(pos * 0.96, "R", &f); 

   ui.drawAreaWidget->setDrawMode(DrawingArea::DrawHoleMarker);
   ui.drawAreaWidget->drawPoint(pos);
   ui.drawAreaWidget->setDrawMode(currMode);
}


void TrianglePPDemoApp::findScalingForDrawArea(
        const tpp::Delaunay& trGenerator,
        double& offsetX,
        double& offsetY,
        double& scaleFactor) const
{
    double minX = 0;
    double minY = 0;
    double maxX = 0;
    double maxY = 0;

    trGenerator.getMinMaxPoints(minX, minY, maxX, maxY);

    double width = maxX - minX;
    double height = maxY - minY;

    if (minX < 0)
    {
       offsetX = -minX;
    }

    if (minY < 0)
    {
       offsetY = -minY;
    }

    if (width < ui.drawAreaWidget->width())
    {
       scaleFactor = ui.drawAreaWidget->width() / width;
    }

    if (height < ui.drawAreaWidget->height())
    {
       double scaleFactor2 = ui.drawAreaWidget->height() / height;

       if (scaleFactor2  < scaleFactor)
       {
           scaleFactor = scaleFactor2;
       }
    }

    // TEST::
    offsetX = -minX;
    offsetY = -minY;
}


void TrianglePPDemoApp::flipPoints(std::vector<Point>& points, float* middlePtr) const
{
    float maxY = 0;
    float minY = ui.drawAreaWidget->height();

    for (const auto& pt : points)
    {
        // OPEN TODO:: after rescaling a slightly off-zero coordinates possible!!!!
        //  - correct floating point arithmetic there!
#if 1
        if (pt.x >= 0 && pt.y >= 0)
            ;
        else
        {
            //Q_ASSERT(pt.x >= 0 && pt.y >= 0);
            std::cout << " WARNING: flipPoints() - negative coordinate: point.x=" << pt.x << ", point.y=" << pt.y << std::endl;
        }
#else
        Q_ASSERT(pt.x >= 0 && pt.y >= 0);
#endif

        if (pt.y < minY)
        {
            minY = pt.y;
        }

        if (pt.y > maxY)
        {
            maxY = pt.y;
        }
    }

    if (maxY == minY)
    {
        return;
    }

    auto middle = (maxY - minY)/2 + minY;

    flipAround(points, middle);

    if (middlePtr)
    {
       *middlePtr = middle;
    }
}


void TrianglePPDemoApp::flipAround(std::vector<Point>& points, float middle) const
{
   for (auto& pt : points)
   {
      pt.y = pt.y < middle
         ? middle + (middle - pt.y)
         : middle - (pt.y - middle);
   }
}


void TrianglePPDemoApp::rescalePoints(std::vector<Point>& points, double offsetX, double offsetY, double scaleFactor) const
{
    for (auto& pt: points)
    {
        pt.x = (pt.x + offsetX) * scaleFactor;
        pt.y = (pt.y + offsetY) * scaleFactor;
    }
}


void TrianglePPDemoApp::zoomIn()
{
    zoomPoints(1.25);
}


void TrianglePPDemoApp::zoomOut()
{
    zoomPoints(0.75);
}


void TrianglePPDemoApp::zoomPoints(float zoomFactor)
{
    auto drawnPoints = ui.drawAreaWidget->getPointCoordinates();
    auto holeMarkers = holePoints_;
    auto regionMarkers = regionPoints_;
    auto regionMaxAreas = regionMaxAreas_;

    ui.drawAreaWidget->clearImage();
    holePoints_.clear();
    regionPoints_.clear();
    regionMaxAreas_.clear();

    // OPEN TODO::: region markers & region constraints!!!

    for (auto& point : drawnPoints)
    {
       QPoint scaledPt(point.x() * zoomFactor, point.y() * zoomFactor);
       ui.drawAreaWidget->drawPoint(scaledPt);
    }

    for (auto& point : holeMarkers)
    {
       QPoint scaledPt(point.x() * zoomFactor, point.y() * zoomFactor);
       ui.drawAreaWidget->drawPoint(scaledPt);

       if (!ui.hideMarkersCheckBox->isChecked())
       {
          onPointChangedToHoleMarker(-1, // hole point index not used at the moment!
                                     scaledPt);
       }
       else
       {
          holePoints_ << scaledPt;
       }
    }

    for (auto& point : regionMarkers)
    {
        QPoint scaledPt(point.x() * zoomFactor, point.y() * zoomFactor);
        ui.drawAreaWidget->drawPoint(scaledPt);

        if (!ui.hideMarkersCheckBox->isChecked())
        {
           onPointChangedToRegionMarker(-1, // hole point index not used at the moment!
                                        scaledPt);
        }
        else
        {
           regionPoints_ << scaledPt;
        }
    }

    for (auto& area : regionMaxAreas)
    {
       regionMaxAreas_.push_back(area * (zoomFactor * zoomFactor));
    }


    // redraw
    if (triangulated_)
    {
       on_triangualtePointsPushButton_clicked();
    }
    else
    {
        auto segmentEndpointIndexes = segmentEndpointIndexes_.toStdVector();
        segmentEndpointIndexes_.clear();

        drawSegments(segmentEndpointIndexes);
    }


    // OPEN TODO:: tesselated_ --> TESSELATE
}


QColor TrianglePPDemoApp::segmentColor() const
{         
   return seperateSegmentColor_ ? c_SegmentColor : c_TriangleColor;
}


void TrianglePPDemoApp::writeToFile()
{
    // fill the points
    auto drawnPoints = ui.drawAreaWidget->getPointCoordinates();
    std::vector<Delaunay::Point> delaunayInput;

    for (auto& point : drawnPoints)
    {
        if (isHoleMarker(point))
            continue;

        delaunayInput.push_back(Delaunay::Point(point.x(), point.y()));

    }

    tpp::Delaunay trGenerator(delaunayInput);

    // write out
    bool ok = false;

    if (!segmentEndpointIndexes_.empty())
    {
       if (!trGenerator.setSegmentConstraint(segmentEndpointIndexes_.toStdVector()))
       {
          QMessageBox::critical(this, tr("Triangle++"), tr("Incorrect segment constraints, ignoring!"));
          return;
       }

       if (!holePoints_.empty())
       {
           trGenerator.setHolesConstraint(toTppPointVector(holePoints_));
       }

       // OPEN TODO:::
       QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                                                       lastFileDir_ + "/Trpp_Segments.poly", 
                                                       tr("Segment File (*.poly)"));
       lastFileDir_ = QFileInfo(fileName).absolutePath();

       ok = trGenerator.saveSegments(fileName.toStdString());
    }
    else
    {
       QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                                                       lastFileDir_ + "/Trpp_Points.node", 
                                                       tr("Vertex File (*.node)"));
       lastFileDir_ = QFileInfo(fileName).absolutePath();

       ok = trGenerator.savePoints(fileName.toStdString());
    }

    if (!ok)
    {
       QMessageBox::critical(this, tr("Triangle++"), tr("File couldn't be written!"));
    }   
}


void TrianglePPDemoApp::readFromFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Read File"),
                                                    lastFileDir_, 
                                                    tr("Triangle Files (*.node *.poly)"));
    lastFileDir_ = QFileInfo(fileName).absolutePath();

    std::vector<Delaunay::Point> points;
    std::vector<int> segmentEndpoints;
    std::vector<Delaunay::Point> holeMarkers;
    std::vector<Delaunay::Point4> regionConstraints; 

    tpp::Delaunay trGenerator(points);
    bool ok = false;
    int duplicatePointsCount = 0;
    
    try 
    {
        if (fileName.endsWith(".node"))
        {
            ok = trGenerator.readPoints(fileName.toStdString(), points);
        }
        else
        {
            ok = trGenerator.readSegments(fileName.toStdString(), points, segmentEndpoints, holeMarkers, regionConstraints, &duplicatePointsCount);
        }
    }
    catch (std::exception& e)
    {
        QMessageBox::critical(this, tr("Triangle++"), tr("Exception thrown while reading the file!") + "\n\n \"" + e.what() + "\"");
        return;
    }
    
    if (!ok)
    {
       QMessageBox::critical(this, tr("Triangle++"), tr("File couldn't be opened!"));
       return;
    } 

    // convert points
    std::vector<Point> dempAppPoints;

    auto convertPoints = [](std::vector<Delaunay::Point>& trppPoints, std::vector<Point>& xyPoints)
    {
        for (size_t i = 0; i < trppPoints.size(); ++i)
        {
           double x = trppPoints[i][0];
           double y = trppPoints[i][1];

           xyPoints.emplace_back(x, y);
        }
    };

    convertPoints(points, dempAppPoints);

    // draw points
    double offsetX = 0;
    double offsetY = 0;
    double scaleFactor = 1;
    float middle;

    findScalingForDrawArea(trGenerator, offsetX, offsetY, scaleFactor);
    rescalePoints(dempAppPoints, offsetX, offsetY, scaleFactor);    
    flipPoints(dempAppPoints, &middle);

    drawPoints(dempAppPoints);

    QString msg = 
       (duplicatePointsCount == 0) 
            ? tr("Read %1 points, %2 segments, and %4 holes from %3")
            : tr("Read %1 unique points, %2 sanitized segments, and %4 holes from %3");

    statusBar()->showMessage(msg.arg(points.size()).arg(segmentEndpoints.size() / 2).arg(fileName).arg(holeMarkers.size()));

    // ...and segments
    drawSegments(segmentEndpoints);

    // draw holes
    std::vector<Point> demoAppHoles;
    convertPoints(holeMarkers, demoAppHoles);

    rescalePoints(demoAppHoles, offsetX, offsetY, scaleFactor);
    flipAround(demoAppHoles, middle);

    for (auto& point : demoAppHoles)
    {
       // TEST:::
       //QPoint pos{ (int)(point.x), (int)(point.y) };
       QPoint pos{ (int)(point.x + 0.5), (int)(point.y + 0.5) };       

       if (!ui.hideMarkersCheckBox->isChecked())
       {
          onPointChangedToHoleMarker(-1, // hole point index not used at the moment!
                                     pos);
       }
       else
       {
          holePoints_ << pos;
       }
    }
    
    // ...and region markers        
    std::vector<Delaunay::Point> regionMarkers;

    for (auto& rc : regionConstraints)
    {
       regionMarkers.emplace_back(rc[0], rc[1]);
    }    

    std::vector<Point> demoAppRegions;
    convertPoints(regionMarkers, demoAppRegions);

    rescalePoints(demoAppRegions, offsetX, offsetY, scaleFactor);
    flipAround(demoAppRegions, middle);

    scaleFactor_ = scaleFactor;

    for (auto& point : demoAppRegions)
    {
       // TEST:::
       //QPoint pos{ (int)(point.x), (int)(point.y) };
       QPoint pos{ (int)(point.x + 0.5), (int)(point.y + 0.5) };

       if (!ui.hideMarkersCheckBox->isChecked())
       {
          onPointChangedToRegionMarker(-1, // region point index not used at the moment!
                                       pos);
       }
       else
       {
          regionPoints_ << pos;
       }
    }

    // rescale also the areas
    for (auto& rc : regionConstraints)
    {     
       regionMaxAreas_.push_back(rc[3] * (scaleFactor_ * scaleFactor_));
    }
}


void TrianglePPDemoApp::drawPoints(const std::vector<Point>& points, float offsetX, float offsetY, float scaleFactor)
{
    for (size_t i = 0; i < points.size(); ++i)
    {
        auto& pt = points[i];
        ui.drawAreaWidget->drawPoint({ (int)(pt.x * scaleFactor + offsetX), (int)(pt.y * scaleFactor + offsetY) });
    }
}


void TrianglePPDemoApp::drawSegments(const std::vector<Point>& segmentEndpoints, const std::vector<Point>& points)
{
    for (size_t i = 0; i < segmentEndpoints.size(); i += 2)
    {
        auto& pt1 = segmentEndpoints[i];
        auto& pt2 = segmentEndpoints[i + 1];

        auto idx1 = std::distance(points.begin(), std::find(points.begin(), points.end(), pt1));
        auto idx2 = std::distance(points.begin(), std::find(points.begin(), points.end(), pt2));

        onSegmentEndpointsSelected(idx1, idx2);
    }
}


void TrianglePPDemoApp::drawSegments(const std::vector<int>& segmentEndpointsIndexes)
{
    Q_ASSERT(segmentEndpointsIndexes.size() % 2 == 0);

    for (size_t i = 0; i < segmentEndpointsIndexes.size(); i += 2)
    {
        onSegmentEndpointsSelected(segmentEndpointsIndexes[i], segmentEndpointsIndexes[i + 1]);
    }
}
