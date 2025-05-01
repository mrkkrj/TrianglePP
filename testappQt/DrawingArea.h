/**
  @file  DrawingArea.h
  @brief Declaration of the DrawingArea class

  @author  Marek Krajewski (mrkkrj), www.ib-krajewski.de
*/

#pragma once

#include <QWidget>
#include <QColor>
#include <QImage>
#include <QPointF>
#include <QVector>

/**
   @brief: Class for drawing points and lines in a Qt widget

     OPEN TODO::: remove distinction between normal points and hole markers????
     OPEN TODO::: showing ctx menu here is a good thing????
 */
class DrawingArea : public QWidget
{
   Q_OBJECT

public:
    DrawingArea(QWidget* parent = nullptr);
    ~DrawingArea();

    enum DrawMode {
       DrawPoints, DrawLines, DrawHoleMarker, MovePoint
    };
    
    void setDrawMode(DrawMode mode);
    void setDrawColor(const QColor& color);
    void setPointSize(int diameter);
    void setLineWidth(int width);

    DrawMode getDrawMode() const;
    QColor getDrawColor() const;
    int getPointSize() const;
    int getLineWidth() const;
    bool isModified() const;
    bool hasPoints() const;

    void drawPoint(const QPointF& pos);
    void clearPoint(const QPointF& pos);
    void clearLastPoint();
    void drawLine(const QPointF& from, const QPointF& to);
    void drawText(const QPointF& pos, const QString& txt, const QFont* font = nullptr);

    QVector<QPointF> getPointCoordinates() const;
    QVector<QPointF> getHoleMarkerCoordinates() const;

    bool openImage(const QString& fileName);
    bool saveImage(const QString& fileName, const char* fileFormat);
    void clearImage();
    void printImage();

signals:
   void pointAdded(const QPointF& pos);
   void pointDeleted(const QPointF& pos);
   void holeMarkerDeleted(const QPointF& pos);
   void lineAdded(const QPointF& start, const QPointF& end);
   void lineDeleted(const QPointF& start, const QPointF& end);
   void linePointsSelected(int startIdx, int endIdx);
   void pointChangedToHoleMarker(int pointIdx, const QPointF& pos);
   void pointMoved(const QPointF& pos1, const QPointF& pos2);
   void pointPositionChanged(const QPointF& originalPos, const QPointF& currentPos);
   void drawingAreaResized();
   
protected:
   void paintEvent(QPaintEvent* ev) override;
   void resizeEvent(QResizeEvent* ev) override;

   void mousePressEvent(QMouseEvent* ev) override;
   void mouseMoveEvent(QMouseEvent* ev) override;
   void mouseReleaseEvent(QMouseEvent* ev) override;

private slots:
   void deletPointAtLastPos();
   void deleteHoleMarkerAtLastPos();
   void startMovingPoint();
   void selectLineStartPoint();
   void selectLineEndPoint();
   void changePointToHoleMarker();

private:
   void drawLineTo(const QPointF& endPos);
   void drawPointAt(const QPointF& pos);
   bool removePoint(const QPointF& pos, QVector<QPointF>& points);
   void resizeImage(QImage& img, const QSize& newSz); 
   bool pointClicked(const QPointF& clickPos, int& pointIndex) const;
   void showPointCtxMenu(const QPointF& pos);

   DrawMode mode_;
   int penWidth_;
   int pointSize_;
   QColor penColor_;

   bool lineStarted_;
   QPointF startPos_;
   QPointF lastMovingPos_;
   int startPosIndex_;
   QVector<QPointF> points_;
   QVector<QPointF> holeMarkerPoints_;

   int lineStartPointIdx_;

   QImage img_;
   bool imgDirty_;
};
