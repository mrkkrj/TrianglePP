/**
  @file  DrawingArea.cpp
  @brief Implementation of the DrawingArea class

  @author  Marek Krajewski (mrkkrj), www.ib-krajewski.de
*/

#include "DrawingArea.h"

#include <QtWidgets>

#if defined(QT_PRINTSUPPORT_LIB)
#  include <QtPrintSupport/qtprintsupportglobal.h>
#  if QT_CONFIG(printdialog)
#     define TRPP_HAS_PRINTER_SUPPORT true
#     include <QPrinter>
#     include <QPrintDialog>
#  endif
#endif


// public methods

DrawingArea::DrawingArea(QWidget* parent)
   : QWidget(parent),
     mode_(DrawLines),
     penWidth_(1),
     pointSize_(6),
     penColor_(Qt::blue),
     lineStarted_(false),
     imgDirty_(false),
     lineStartPointIdx_(-1)
{
   // get paint events only for parts that are newly visible
   setAttribute(Qt::WA_StaticContents);
}


DrawingArea::~DrawingArea()
{
}


void DrawingArea::setDrawColor(const QColor& color)
{
   penColor_ = color;
}


QColor DrawingArea::getDrawColor() const
{
   return penColor_;
}


void DrawingArea::setDrawMode(DrawMode mode)
{
   mode_ = mode;
}


auto DrawingArea::getDrawMode() const -> DrawMode
{
   return mode_;
}


void DrawingArea::setPointSize(int diameter)
{
   pointSize_ = diameter;
}


int DrawingArea::getPointSize() const
{
   return pointSize_;
}


void DrawingArea::setLineWidth(int width)
{
   penWidth_ = width;
}


int DrawingArea::getLineWidth() const 
{ 
   return penWidth_; 
}


bool DrawingArea::isModified() const 
{ 
   return imgDirty_; 
}


void DrawingArea::drawPoint(const QPoint& pos)
{
   drawPointAt(pos);

   if (mode_ == DrawPoints)
   {
      points_.append(pos);
   }
   else if (mode_ == DrawHoleMarker)
   {
      holeMarkerPoints_.append(pos);
   }
}


void DrawingArea::clearPoint(const QPoint& pos)
{
   if (removePoint(pos, points_))
   {
      emit pointDeleted(pos);
   }

}


void DrawingArea::drawLine(const QPoint& from, const QPoint& to)
{
   startPos_ = from;
   drawLineTo(to);

   Q_ASSERT(startPos_ == to);
}


void DrawingArea::drawText(const QPoint& pos, const QString& txt, const QFont* font)
{
   QPainter painter(&img_);
   auto pen = QPen(penColor_);

   painter.setPen(pen);
   if (font)
   {
      painter.setFont(*font);
   }

   painter.drawText(pos, txt);

#if 0 // OPEN TODO:::
   // update only the changed region
   int rad = (penWidth_ / 2) + 2;
   update(QRect(pos, xxxx).normalized().adjusted(-rad, -rad, +rad, +rad));
#else
   update();
#endif

   imgDirty_ = true;
}


QVector<QPoint> DrawingArea::getPointCoordinates() const
{
   return points_;
}


QVector<QPoint> DrawingArea::getHoleMarkerCoordinates() const
{
   return holeMarkerPoints_;
}


bool DrawingArea::openImage(const QString& fileName)
{
   QImage img;

   if (!img.load(fileName))
   {
      return false;
   }

   QSize sz = img.size().expandedTo(size());
   resizeImage(img, sz);

   img_ = img;
   imgDirty_ = false;

   update();
   return true;
}


bool DrawingArea::saveImage(const QString& fileName, const char* fileFormat)
{
   QImage img = img_;
   resizeImage(img, size());

   if (img.save(fileName, fileFormat))
   {
      imgDirty_ = false;
      return true;
   }
   else
   {
      return false;
   }
}


void DrawingArea::printImage()
{
#ifdef TRPP_HAS_PRINTER_SUPPORT 
   QPrinter printer(QPrinter::HighResolution);
   QPrintDialog dlg(&printer, this);

   if (dlg.exec() == QDialog::Accepted) 
   {
      QPainter painter(&printer);
      QRect rect = painter.viewport();

      QSize sz = img_.size();
      sz.scale(rect.size(), Qt::KeepAspectRatio);

      painter.setViewport(rect.x(), rect.y(), sz.width(), sz.height());
      painter.setWindow(img_.rect());

      painter.drawImage(0, 0, img_);
   }
#endif 
}


void DrawingArea::clearImage()
{
   img_.fill(qRgb(255, 255, 255));

   if (mode_ == DrawPoints)
   {
      points_.clear();
      holeMarkerPoints_.clear();
   }

   lineStartPointIdx_ = -1;
   imgDirty_ = true;

   update();
}


// protected methods

void DrawingArea::mousePressEvent(QMouseEvent* ev)
{
   if (ev->button() == Qt::LeftButton) 
   {
      startPos_ = ev->pos();
      if (mode_ == DrawLines)
      {
         lineStarted_ = true;
      }
      else
      {
         int idx = -1;

         if (pointClicked(ev->pos(), idx))
         {
            startPos_ = points_[idx];
            lastMovingPos_ = startPos_;
            startPosIndex_ = idx;

            mode_ = MovePoint;
            return;
         }
      }
   }
   else if (ev->button() == Qt::RightButton)
   {
      if (mode_ == DrawPoints)
      {
         int idx = -1;

         if (pointClicked(ev->pos(), idx))
         {
            startPos_ = points_[idx];
            showPointCtxMenu(ev->pos());
            return;
         }
      }
   }
}


void DrawingArea::mouseMoveEvent(QMouseEvent* ev)
{
   if ((ev->buttons() & Qt::LeftButton) && 
       lineStarted_)
   {
      drawLineTo(ev->pos());
   }
   else if ((ev->buttons() & Qt::LeftButton) &&
            mode_ == MovePoint)
   {
      // OPEN TODO::: --> ???? but not over points!!!!!
      //   - restore points and lines....

      // overpaint        
      auto pointColor = penColor_;
      penColor_ = Qt::white;
      drawPointAt(lastMovingPos_);
      penColor_ = pointColor; // OPEN TODO::: ---> to a method???

      drawPointAt(ev->pos());

      Q_ASSERT(startPosIndex_ >= 0);
      points_[startPosIndex_] = ev->pos();

      emit pointMoved(lastMovingPos_, ev->pos());

      lastMovingPos_ = ev->pos();
   }
}


void DrawingArea::mouseReleaseEvent(QMouseEvent* ev)
{
   if (ev->button() == Qt::LeftButton)
   {
      if (mode_ == DrawLines && lineStarted_)
      {
         drawLineTo(ev->pos());
         lineStarted_ = false;
      }
      else if (mode_ == DrawPoints)
      {
         drawPointAt(ev->pos());
         points_.append(ev->pos());
      }
      else if (mode_ == MovePoint)
      {
         Q_ASSERT(startPosIndex_ >= 0);
         points_[startPosIndex_] = ev->pos();

         mode_ = DrawPoints;
         startPosIndex_ = -1;

         emit pointPositionChanged(startPos_, ev->pos());
      }
   }
}


void DrawingArea::paintEvent(QPaintEvent* ev)
{
   QPainter painter(this);

   QRect dirtyRect = ev->rect();
   painter.drawImage(dirtyRect, img_, dirtyRect);
}


void DrawingArea::resizeEvent(QResizeEvent* ev)
{
   if (width() > img_.width() ||
       height() > img_.height()) 
   {

      // OPEN TODO::: 
      // -- resize to be slightly larger then the main window
      //    avoid the need to resize img_ 

      int newWidth = qMax(width() + 128, img_.width());
      int newHeight = qMax(height() + 128, img_.height());

      resizeImage(img_, QSize(newWidth, newHeight));
      update();
   }

   QWidget::resizeEvent(ev);
}


// private slots

void DrawingArea::deletPointAtLastPos()
{
   clearPoint(startPos_);
}


void DrawingArea::deleteHoleMarkerAtLastPos()
{
   // OPEN TODO::: here or outside the class???
   //  --> treat hole points as normal points with diffrent color???

   if (removePoint(startPos_, holeMarkerPoints_))
   {
      emit holeMarkerDeleted(startPos_);
   }
}


void DrawingArea::startMovingPoint()
{
   int idx = points_.indexOf(startPos_);

   if (idx != -1)
   {
      // OPEN TODO::: next

      // ...

   }
}


void DrawingArea::selectLineStartPoint()
{
   lineStartPointIdx_ = -1;
   bool pointFound = pointClicked(startPos_, lineStartPointIdx_);

   Q_ASSERT(pointFound);
   Q_ASSERT(lineStartPointIdx_ != -1);
}


void DrawingArea::selectLineEndPoint()
{
   Q_ASSERT(lineStartPointIdx_ != -1);

   int lineEndPointIdx = -1;
   bool pointFound = pointClicked(startPos_, lineEndPointIdx);

   Q_ASSERT(pointFound);
   Q_ASSERT(lineEndPointIdx != -1);

   emit linePointsSelected(lineStartPointIdx_, lineEndPointIdx);
   lineStartPointIdx_ = -1;
}


void DrawingArea::changePointToHoleMarker()
{
   Q_ASSERT(lineStartPointIdx_ == -1);

   int pointIdx = -1;
   bool pointFound = pointClicked(startPos_, pointIdx);

   Q_ASSERT(pointFound);
   holeMarkerPoints_.append(startPos_);
   
   emit pointChangedToHoleMarker(pointIdx, startPos_);
}


// private methods

void DrawingArea::drawLineTo(const QPoint& endPos)
{
   QPainter painter(&img_);
   auto pen = QPen(penColor_, penWidth_, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

   painter.setPen(pen);
   painter.drawLine(startPos_, endPos);

   // update only the changed region
   int rad = (penWidth_ / 2) + 2;
   update(QRect(startPos_, endPos).normalized().adjusted(-rad, -rad, +rad, +rad));

   startPos_ = endPos;
   imgDirty_ = true;
}


void DrawingArea::drawPointAt(const QPoint& pos)
{
   QPainter painter(&img_);

   painter.setPen(QPen(penColor_, penWidth_));

   if (pointSize_ <= 1)
   {
      painter.drawPoint(pos);
   }
   else
   {
      painter.setBrush(QBrush(penColor_));
      painter.drawEllipse(pos, pointSize_ / 2, pointSize_ / 2);
      
      // OPEN TODO:::optimize ...
#if 0
      int rad = (pointSize_ / 2) + 2;
      update(QRect(pos, pos).normalized().adjusted(-rad, -rad, +rad, +rad));
#endif
      update();
   }

   imgDirty_ = true;
}


bool DrawingArea::removePoint(const QPoint& pos, QVector<QPoint>& pointsList)
{
   int idx = pointsList.indexOf(pos);

   if (idx == -1)
   {
      return false;
   }

   pointsList.remove(idx);

   // overpaint
   auto pointColor = penColor_;
   penColor_ = Qt::white;

   drawPointAt(pos);

   // restore color
   penColor_ = pointColor;

   imgDirty_ = true;
   return true;      
}


void DrawingArea::resizeImage(QImage& img, const QSize& newSize)
{
   if (img.size() == newSize)
   {
      return;
   }

   QImage resized(newSize, QImage::Format_RGB32);
   resized.fill(qRgb(255, 255, 255));

   QPainter painter(&resized);
   painter.drawImage(QPoint(0, 0), img);

   img = resized;
}


bool DrawingArea::pointClicked(const QPoint& clickPos, int& pointIndex) const
{
   for (auto& pt : points_)
   {
      if (qAbs(clickPos.x() - pt.x()) <= pointSize_ / 2 &&
          qAbs(clickPos.y() - pt.y()) <= pointSize_ / 2)
      {
         pointIndex = points_.indexOf(pt);
         return true;
      }
   }

   return false;
}


void DrawingArea::showPointCtxMenu(const QPoint& pos)
{
   QMenu ctxtMenu(tr(""), this);

   bool isHoleMarker = holeMarkerPoints_.contains(startPos_);

   QAction action1(isHoleMarker ? "Delete HoleMarker" : "Delete Point", this);
   connect(&action1, &QAction::triggered, this, &DrawingArea::deletPointAtLastPos);

   ctxtMenu.addAction(&action1);

   // OPEN TODO::: next...
#if 0 
   QAction action2("Move Point", this);
   connect(&action2, &QAction::triggered, this, &DrawingArea::startMovingPoint);
   ctxtMenu.addAction(&action2);
#endif

   // OPEN TODO:: not in the general-purpsose drawing widget ?????? 

   QAction action3("Start Segment", this);
   connect(&action3, &QAction::triggered, this, &DrawingArea::selectLineStartPoint);
   ctxtMenu.addAction(&action3);
   action3.setEnabled(lineStartPointIdx_ == -1 && !isHoleMarker);

   QAction action4("End Segment", this);
   connect(&action4, &QAction::triggered, this, &DrawingArea::selectLineEndPoint);
   ctxtMenu.addAction(&action4);
   action4.setEnabled(lineStartPointIdx_ != -1 && !isHoleMarker);

   QAction action5("Change to Hole Marker", this);
   connect(&action5, &QAction::triggered, this, &DrawingArea::changePointToHoleMarker);
   ctxtMenu.addAction(&action5);
   action5.setEnabled(lineStartPointIdx_ == -1 && !isHoleMarker);

   ctxtMenu.exec(mapToGlobal(startPos_));
}
