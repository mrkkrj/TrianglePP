
// OPEN TODO::: file header ...


#pragma once

#include <QWidget>
#include <QColor>
#include <QImage>
#include <QPoint>
#include <QVector>


// OPEN TODO::: class header ...

class DrawingArea : public QWidget
{
   Q_OBJECT

public:
    DrawingArea(QWidget* parent = nullptr);
    ~DrawingArea();

    enum DrawMode {
       DrawPoints, DrawLines
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

    void drawPoint(const QPoint& pos);
    void drawLine(const QPoint& from, const QPoint& to);

    QVector<QPoint> getPointCoordinates() const;

    bool openImage(const QString& fileName);
    bool saveImage(const QString& fileName, const char* fileFormat);
    void clearImage();
    void printImage();

protected:
   void paintEvent(QPaintEvent* ev) override;
   void resizeEvent(QResizeEvent* ev) override;

   void mousePressEvent(QMouseEvent* ev) override;
   void mouseMoveEvent(QMouseEvent* ev) override;
   void mouseReleaseEvent(QMouseEvent* ev) override;

private slots:
   void deletPointAtLastPos();
   void startMovingPoint();

private:
   void drawLineTo(const QPoint& endPos);
   void drawPointAt(const QPoint& pos);
   void resizeImage(QImage& img, const QSize& newSz); 
   bool pointClicked(const QPoint& clickPos, int& pointIndex) const;
   void showPointCtxMenu(const QPoint& pos);

   DrawMode mode_;
   int penWidth_;
   int pointSize_;
   QColor penColor_;

   bool lineStarted_;
   QPoint startPos_;
   QVector<QPoint> points_;

   QImage img_;
   bool imgDirty_;
};
