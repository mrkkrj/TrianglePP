/**
  @file  TrianglePpOptions.cpp
  @brief Declaration of the TrianglePpOptions class

  @author  Marek Krajewski (mrkkrj), www.ib-krajewski.de
*/

#pragma once

#include "ui_TrianglePpOptions.h"

/**
   @brief: Class for displaying and changing triangulation options
 */
class TrianglePpOptions : public QDialog
{
    Q_OBJECT

public:
    TrianglePpOptions(QWidget *parent = nullptr);

    int getMinAngle() const;
    int getMaxArea() const;
    int getMinPointCount() const;
    int getMaxPointCount() const;
    
    bool useConformingDelaunay() const;
    bool includeConvexHull() const;
    bool applyQualityConstraints() const;
    bool seperateSegmentColor() const;

    QVector<int> getSegmentPointIndexes() const;
    void getDelaunayColors(QColor& vertexColor, QColor& segmentColor,  QColor& voronoiColor) const;

    void fillContents(int minAngle, int maxArea = -1, int minPoints = -1, int maxPoints = -1, 
                      bool qualityConstr = false, bool confDelaunay = false, bool convexHull = false, 
                      bool diffColorForSegments = true);
    void fillColors(const QColor& vertexColor, const QColor& segmentColor, const QColor& voronoiColor);
  

    void setMinAngleBoundaries(float maxOk, float maxWarning);
    void setSegmentPointIndexes(const QVector<int>& segmentEndpoints);

private slots:
   void on_constrainedDelaunayCheckBox_clicked(bool checked);
   void on_conformingDelaunayCheckBox_clicked(bool checked);
   void on_applyQualityConstraintsCheckBox_clicked(bool checked);      
   void on_minAngleLineEdit_textChanged();
   void on_minAngleLineEdit_editingFinished();
   void on_segmentPointsLineEdit_editingFinished();
   void on_removeConcavitiesCheckBox_clicked(bool checked);
   void on_seperateSegmentColorCheckBox_clicked(bool checked);
   void on_vertexColorButton_clicked();
   void on_segmentColorButton_clicked();
   void on_voronoiColorButton_clicked();
   void on_restoreColorsButton_clicked();
   
private:
   void enableMinMaxAngle(bool enable);
   Ui::TrianglePpOptions ui;
   QColor vertexColor_;
   QColor segmentColor_;
   QColor voronoiColor_;

   float minAngleOk_ = 0;
   float miAngleWarning_ = 0;
   QPalette normalPal_;
};
