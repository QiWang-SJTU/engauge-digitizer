#ifndef GRAPHICS_LINES_FOR_CURVES_H
#define GRAPHICS_LINES_FOR_CURVES_H

#include <QHash>

class CurveStyles;
class GraphicsLinesForCurve;
class GraphicsPoint;
class GraphicsScene;
class Point;
class QGraphicsItem;
class QPointF;
class QTextStream;
class Transformation;

typedef QHash<QString, GraphicsLinesForCurve*> GraphicsLinesContainer;

/// This class stores the GraphicsLinesForCurves objects, one per Curve
class GraphicsLinesForCurves
{
public:
  /// Single constructor
  GraphicsLinesForCurves();

  /// Remove points that are unwanted
  void lineMembershipPurge ();

  /// Mark points as unwanted
  void lineMembershipReset ();

  /// Debugging method for printing directly from symbolic debugger
  void print () const;

  /// Debugging method that supports print method of this class and printStream method of some other class(es)
  void printStream (QTextStream &str) const;

  /// Remove the specified point. The act of deleting it will automatically remove it from the GraphicsScene
  void removePoint (const QString &identifier);

  /// Add new point
  void savePoint (GraphicsScene &scene,
                  const QString &curveName,
                  const QString &pointIdentifier,
                  double ordinal,
                  GraphicsPoint &point);

  /// Update the GraphicsScene with the specified Point from the Document. If it does not exist yet in the scene, we add it
  void updateAfterCommand (GraphicsScene &scene,
                           const CurveStyles &curveStyles,
                           const QString &curveName,
                           const Point &point);

  /// Mark the end of savePoint calls. Remove stale lines, insert missing lines, and draw the graphics lines
  void updateFinish (const CurveStyles &curveStyles);

  /// Calls to moveLinesWithDraggedPoint have finished so update the lines correspondingly
  void updateGraphicsLinesToMatchGraphicsPoints (const CurveStyles &curveStyles);

  /// See GraphicsScene::updateOrdinalsAfterDrag
  void updatePointOrdinalsAfterDrag (const CurveStyles &curveStyles,
                                     const Transformation &transformation);

  /// Mark the start of savePoint calls. Afterwards, updateFinish gets called
  void updateStart ();

private:

  GraphicsLinesContainer m_graphicsLinesForCurve;
};

#endif // GRAPHICS_LINES_FOR_CURVES_H
