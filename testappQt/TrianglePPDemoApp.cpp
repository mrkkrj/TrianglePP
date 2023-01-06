

// OPEN TODO:: file header


#include "TrianglePPDemoApp.h"
#include "TrianglePpOptions.h"

#include <tpp_interface.hpp>

#include <QMessageBox>
#include <QFileDialog>
#include <QPixmap>

#include <vector>
#include <random>
#include <cassert>


using namespace tpp;


namespace {

   // impl. constants
   const int c_defaultMinPoints = 3;
   const int c_defaultMaxPoints = 100;
   const int c_defaultMinAngle = 20; // default min. angle used by the original Triangle package!

   const QColor c_TriangleColor = Qt::blue;
   const QColor c_VoronoiColor = Qt::red;
   const QColor c_SegmentColor = "limegreen";


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

TrianglePPDemoApp::TrianglePPDemoApp(QWidget *parent)
    : QMainWindow(parent),
      mode_(ManualMode),
      useConstraints_(false),
      triangulated_(false),
      minAngle_(-1),
      maxArea_(-1),
      minPoints_(-1),
      maxPoints_(-1),
      useConformingDelaunay_(false),
      includeConvexHull_(true)
{
   ui.setupUi(this);
   
   ui.drawAreaWidget->setDrawMode(DrawingArea::DrawPoints);
   ui.optionsToolButton->setText(QChar(0x2630)); // trigram for the heaven (tian)

   setGenerateButtonText();

   // drawing area changes:
   connect(ui.drawAreaWidget, &DrawingArea::pointDeleted, this, &TrianglePPDemoApp::onTriangulationPointDeleted);
   connect(ui.drawAreaWidget, &DrawingArea::linePointsSelected, this, &TrianglePPDemoApp::onSegmentEndpointsSelected);
   connect(ui.drawAreaWidget, &DrawingArea::pointChangedToHoleMarker, this, &TrianglePPDemoApp::onPointChangedToHoleMarker);
}


void TrianglePPDemoApp::on_generatePointsPushButton_clicked()
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
      QMessageBox::critical(this, tr("ERROR"), tr("Not enough points to triangulate!"));
      return;
   }

   std::vector<Delaunay::Point> delaunayInput;
   for (auto& point : drawnPoints)
   {
      if (isHoleMarker(point))
         continue;

      delaunayInput.push_back(Delaunay::Point(point.x(), point.y()));
   }

   Delaunay trGenerator(delaunayInput);
   configDelaunay(trGenerator);
   
#if 0
   auto trace = tpp::Debug; // TEST:::
#else
   auto trace = tpp::None;
#endif

   trGenerator.useConvexHullWithSegments(includeConvexHull_);

   if (useConformingDelaunay_)
   {
      trGenerator.TriangulateConf(useConstraints_, trace);
   }
   else
   {
      trGenerator.Triangulate(useConstraints_, trace);
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

   // and hole markers
   for (auto& point : holePoints_)
   {
      drawHoleMarker(point);
   }

   ui.drawAreaWidget->setDrawColor(c_TriangleColor);

   // finished
   statusBar()->showMessage(QString("Created %1 triangles").arg(trGenerator.ntriangles()));
}


void TrianglePPDemoApp::on_tesselatePointsPushButton_clicked()
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


void TrianglePPDemoApp::on_pointModeComboBox_currentIndexChanged(int index)
{
   Q_ASSERT(ManualMode == PointGenerationMode(0));

   mode_ = PointGenerationMode(index); // 0 == ManualMode!
   setGenerateButtonText();
}


void TrianglePPDemoApp::on_useConstraintsCheckBox_toggled(bool checked)
{
   useConstraints_ = checked;

   // ??? good idea?
#if 0
   // re-triangulate!
   on_triangualtePointsPushButton_clicked();
#endif
}


void TrianglePPDemoApp::on_optionsToolButton_clicked()
{
   QMenu ctxtMenu(tr(""), this);
   
   QAction action01("Save to File", this);
   connect(&action01, &QAction::triggered, this, &TrianglePPDemoApp::writeToFile);
   ctxtMenu.addAction(&action01);

   QAction action02("Read from File", this);
   connect(&action02, &QAction::triggered, this, &TrianglePPDemoApp::readFromFile);
   ctxtMenu.addAction(&action02);

   QAction action1("Options", this);
   connect(&action1, &QAction::triggered, this, &TrianglePPDemoApp::showTrianguationOptions);
   ctxtMenu.addAction(&action1);

   QAction action2("Info", this);
   connect(&action2, &QAction::triggered, this, &TrianglePPDemoApp::showInfo);
   ctxtMenu.addAction(&action2);
   
   QAction action3("Close", this);
   connect(&action3, &QAction::triggered, this, &TrianglePPDemoApp::close);
   ctxtMenu.addAction(&action3);

   ctxtMenu.exec(mapToGlobal(ui.optionsToolButton->geometry().bottomLeft()));
}


// private methods

void TrianglePPDemoApp::onTriangulationPointDeleted(const QPoint& pos)
{
   if (holePoints_.contains(pos))
   {
      holePoints_.removeOne(pos);
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
   ui.drawAreaWidget->setDrawColor(c_SegmentColor);

   auto drawnPoints = ui.drawAreaWidget->getPointCoordinates();
   ui.drawAreaWidget->drawLine(drawnPoints[startPointIdx], drawnPoints[endPointIdx]);

   Q_ASSERT(startPointIdx >= 0 && endPointIdx >= 0);
   Q_ASSERT(startPointIdx < drawnPoints.size()&& endPointIdx < drawnPoints.size());

   segmentEndpointIndexes_ << startPointIdx << endPointIdx;

   ui.drawAreaWidget->setDrawColor(c_TriangleColor);
}


void TrianglePPDemoApp::onPointChangedToHoleMarker(int pointIdx, const QPoint& pos)
{
   drawHoleMarker(pos);
   ui.drawAreaWidget->setDrawColor(c_TriangleColor);

   holePointIndexes_ << pointIdx;
   holePoints_ << pos;
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

   statusBar()->showMessage(QString("Generated %1 points").arg(count));

   // 2. dice out points
   auto ptSize = ui.drawAreaWidget->getPointSize();

   std::uniform_int_distribution<int> udistrWidth(0 + ptSize / 2, ui.drawAreaWidget->width() - 1 - (ptSize / 2));
   std::uniform_int_distribution<int> udistrHeight(0 + ptSize / 2, ui.drawAreaWidget->height() - 1 - (ptSize / 2));

   for (int i = 0; i < count; ++i)
   {
      // OPEN TODO:: 
      //  -- check minimum distance to other points ????

      ui.drawAreaWidget->drawPoint({ udistrWidth(reng), udistrHeight(reng) });
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
   statusBar()->showMessage(QString("Created %1 Example-1 points").arg(constrDelaunayInput.size()));

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
    pslgDelaunayInput.push_back(Point(1.5, 1));
    pslgDelaunayInput.push_back(Point(2.5, 1));
    pslgDelaunayInput.push_back(Point(1.6, 1.5));
    pslgDelaunayInput.push_back(Point(2.4, 1.5));
    pslgDelaunayInput.push_back(Point(2, 2));
    pslgDelaunayInput.push_back(Point(2, 3));

    float offsetX = 20;
    float offsetY = 20;
    float scaleFactor = 80;

    drawPoints(pslgDelaunayInput, offsetX, offsetY, scaleFactor);
    statusBar()->showMessage(QString("Created %1 Example-2 points").arg(pslgDelaunayInput.size()));

    // draw segments 
    std::vector<Point> pslgDelaunaySegments;

    // outer outline
    pslgDelaunaySegments.push_back(Point(1, 0));
    pslgDelaunaySegments.push_back(Point(0, 0));
    pslgDelaunaySegments.push_back(Point(0, 0));
    pslgDelaunaySegments.push_back(Point(2, 3));
    pslgDelaunaySegments.push_back(Point(2, 3));
    pslgDelaunaySegments.push_back(Point(4, 0));
    pslgDelaunaySegments.push_back(Point(4, 0));
    pslgDelaunaySegments.push_back(Point(3, 0));
    pslgDelaunaySegments.push_back(Point(3, 0));
    pslgDelaunaySegments.push_back(Point(2.5, 1));
    pslgDelaunaySegments.push_back(Point(2.5, 1));
    pslgDelaunaySegments.push_back(Point(1.5, 1));
    pslgDelaunaySegments.push_back(Point(1.5, 1));
    pslgDelaunaySegments.push_back(Point(1, 0));

    // inner outline
    pslgDelaunaySegments.push_back(Point(1.6, 1.5));
    pslgDelaunaySegments.push_back(Point(2, 2));
    pslgDelaunaySegments.push_back(Point(2, 2));
    pslgDelaunaySegments.push_back(Point(2.4, 1.5));
    pslgDelaunaySegments.push_back(Point(2.4, 1.5));
    pslgDelaunaySegments.push_back(Point(1.6, 1.5));

    drawSegments(pslgDelaunaySegments, pslgDelaunayInput);
}


void TrianglePPDemoApp::showTrianguationOptions()
{
   TrianglePpOptions dlg(this);

   dlg.fillContents(
         minAngle_ > 0 ? minAngle_ : c_defaultMinAngle,
         maxArea_ > 0 ? maxArea_ : -1,
         minPoints_ > 0 ? minPoints_ : c_defaultMinPoints,
         maxPoints_ > 0 ? maxPoints_ : c_defaultMaxPoints,
         useConformingDelaunay_,
         includeConvexHull_);

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
               " - Voronoi tesselations"
               "\n\nWARNING: not yet completely correct, more work will be done...");

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

   about.setIconPixmap(QPixmap(":/TrianglePPDemo/triangle-PP-sm.jpg"));
   about.setWindowIcon(QIcon(":/TrianglePPDemo/triangle-PP-ico.jpg"));
   about.setWindowTitle("About Triangle++ Demo");

   about.setStandardButtons(QMessageBox::Ok);
   about.setDefaultButton(QMessageBox::Ok);   
   about.show();
   about.exec();
}


void TrianglePPDemoApp::clearDisplay()
{
   ui.drawAreaWidget->clearImage();
   statusBar()->showMessage("");
   
   segmentEndpointIndexes_.clear(); // forget them, bound to old points!
   holePointIndexes_.clear();
   holePoints_.clear();

   triangulated_ = false;
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


void TrianglePPDemoApp::drawVoronoiTesselation(tpp::Delaunay& trGenerator)
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

   std::vector<Delaunay::Point> constrDelaunayHoles;
   for (auto& point : holePoints_)
   {
      constrDelaunayHoles.push_back(Delaunay::Point(point.x(), point.y()));
   }

   trGenerator.setHolesConstraint(constrDelaunayHoles);
}


bool TrianglePPDemoApp::isHoleMarker(const QPoint& point) const
{
   return holePoints_.contains(point);
}


void TrianglePPDemoApp::drawHoleMarker(const QPoint& pos)
{
   ui.drawAreaWidget->setDrawColor(c_SegmentColor);
   QFont f;
   f.setPixelSize(22);

   ui.drawAreaWidget->drawText(pos * 0.96, "H", &f);
   ui.drawAreaWidget->drawPoint(pos);
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

    Delaunay trGenerator(delaunayInput);

    // write out
    bool ok = false;

    if (!segmentEndpointIndexes_.empty())
    {
#if 1
       QMessageBox::warning(this, tr("WARNING"), tr("Exporting of segments not yet working!!!"));
       return;
#else
       // TEST::: 

       if (!trGenerator.setSegmentConstraint(segmentEndpointIndexes_.toStdVector()))
       {
          QMessageBox::critical(this, tr("ERROR"), tr("Incorrect segment constraints, ignoring!"));
          return;
       }

       QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                                                       "./Trpp_Segments.poly", tr("Segment File (*.poly)"));

       ok = trGenerator.saveSegments(fileName.toStdString());
#endif
    }
    else
    {
       QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                                                       "./Trpp_Points.nodes", tr("Vertex File (*.nodes)"));

       ok = trGenerator.savePoints(fileName.toStdString());
    }

    if (!ok)
    {
       QMessageBox::critical(this, tr("ERROR"), tr("File couldn't be written!"));
    }   
}


void TrianglePPDemoApp::readFromFile()
{
   QString fileName = QFileDialog::getOpenFileName(this, tr("Read File"),
                                                   "./", tr("Vertex File (*.node)")); // OPEN TODO::: .nodes?

    std::vector<Delaunay::Point> points;
    Delaunay trGenerator(points);
    
    bool ok = trGenerator.readPoints(fileName.toStdString(), points);
    
    if (!ok)
    {
       QMessageBox::critical(this, tr("ERROR"), tr("File couldn't be opened!"));
    } 

    // TEST:: scalings work for spiral.nodes & box.nodes!!!!

    // OPEN TODO::: add automatic scaling ???

    double offsetX = 200;
    double offsetY = 200;
    double scaleFactor = 50;

    for (size_t i = 0; i < points.size(); ++i)
    {       
       double x = points[i][0];
       double y = points[i][1];

       ui.drawAreaWidget->drawPoint({ int(x * scaleFactor + offsetX), int(y * scaleFactor + offsetY) });
    }

    statusBar()->showMessage(QString("Read %1 points from %2").arg(points.size()).arg(fileName));
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

