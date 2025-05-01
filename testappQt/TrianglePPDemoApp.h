/**
  @file  TrianglePPDemoApp.cpp
  @brief Declaration of the TrianglePPDemoApp class

  @author  Marek Krajewski (mrkkrj), www.ib-krajewski.de
*/

#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_TrianglePPDemoApp.h"


namespace tpp 
{
   class Delaunay;
}


/**
   @brief: Class for creating, drawing and displaying triangulations in a Qt application window
 */
class TrianglePPDemoApp : public QMainWindow
{
    Q_OBJECT

public:
    TrianglePPDemoApp(QWidget *parent = nullptr);

public slots:
   void on_generatePointsPushButton_clicked();
   void on_triangualtePointsPushButton_clicked();
   void on_tesselatePointsPushButton_clicked();
   void on_pointModeComboBox_currentIndexChanged(int index);
   void on_useConstraintsCheckBox_toggled(bool checked);
   void on_hideMarkersCheckBox_toggled(bool checked);
   void on_optionsToolButton_clicked();

protected:
   void resizeEvent(QResizeEvent *event) override;

private slots:
   void onTriangulationPointDeleted(const QPointF& pos);
   void onSegmentEndpointsSelected(int startPointIdx, int endPointIdx);
   void onPointChangedToHoleMarker(int pointIdx, const QPointF& pos);
   void onPointChangedToRegionMarker(int pointIdx, const QPointF& pos);
   void onTriangulationPointMoved(const QPointF& pos1, const QPointF& pos2);

private:
   void addUiShortcuts();
   void setGenerateButtonText();
   void generateRandomPoints();
   void showExample1();
   void showExample2();
   void showTrianguationOptions();
   void showInfo();
   void clearDisplay();
   void clearVoronoiPoints();
   void drawTriangualtion(tpp::Delaunay& trGenerator, QVector<QPointF>& pointsOnScreen);
   void drawVoronoiTesselation(tpp::Delaunay& trGenerator);
   void configDelaunay(tpp::Delaunay& trGenerator);
   bool isHoleMarker(const QPointF& point) const;
   void drawMarkerPoint(const QPointF& pos, const QColor& color, const QString& text);
   void findScalingForDrawArea(const tpp::Delaunay& trGenerator, double& offsetX, double& offsetY, double& scaleFactor) const;

   void writeToFile();
   void readFromFile();

   struct Point {
       float x; float y;
       Point(float x_, float y_) : x(x_), y(y_) {}
       bool operator==(const Point& other) const { return x == other.x && y == other.y; }
   };

   void drawPoints(const std::vector<Point>& points, float offsetX = 0, float offsetY = 0, float scaleFactor = 1);
   void drawSegments(const std::vector<Point>& segmentEndpoints, const std::vector<Point>& points);
   void drawSegments(const std::vector<int>& segmentEndpointsIndexes);
   void flipPoints(std::vector<Point>& points, float* middle = nullptr) const;
   void flipAround(std::vector<Point>& points, float middle) const;
   void rescalePoints(std::vector<Point>& points, double offsetX, double offsetY, double scaleFactor) const;
   void zoomIn();
   void zoomOut();
   void zoomPoints(float zoomFactor);
   void resetZoom();
   void undoPointCreation();

   QPointF rescaleReadPoint(const QPointF& point) const;
   QColor segmentColor() const;

private:
    Ui::TrianglePPDemoAppClass ui;

    QAction* zoomInAct_;
    QAction* zoomOutAct_;
    QAction* showOptionsAct_;
    QAction* saveFileAct_;
    QAction* undoAct_;

    enum PointGenerationMode { 
       ManualMode = 0, AutomaticMode, FromImageMode, FromFileMode, Example1Mode, Example2Mode
    } mode_;

    bool useConstraints_;
    bool triangulated_;
    bool tesselated_;

    int minAngle_;
    int maxArea_;
    int minPoints_;
    int maxPoints_;
    bool useConformingDelaunay_;
    bool includeConvexHull_;
    bool seperateSegmentColor_;

    QString lastFileDir_;

    QVector<QPointF> voronoiPoints_;
    QVector<int> segmentEndpointIndexes_;
    QVector<QPointF> holePoints_;
    QVector<QPointF> regionPoints_;
    QVector<float> regionMaxAreas_;

    // support for precise triangulation when points/segments are read form file:
    bool readFromFile_;

    float scaleFactor_;
    float offsetX_;
    float offsetY_;
    float middle_;
    float zoomFactor_;
    QSize originalSize_;

    QVector<QPointF> vertexPointsOrig_;
    QVector<QPointF> holePointsOrig_;
    QVector<QPointF> regionPointsOrig_;
    QVector<float> regionMaxAreasOrig_;
};
