#include "CallbackSceneUpdateAfterCommand.h"
#include "Curve.h"
#include "CurvesGraphs.h"
#include "CurveStyles.h"
#include "DataKey.h"
#include "EngaugeAssert.h"
#include "EnumsToQt.h"
#include "GraphicsItemType.h"
#include "GraphicsPoint.h"
#include "GraphicsPointFactory.h"
#include "GraphicsScene.h"
#include "Logger.h"
#include "MainWindow.h"
#include "Point.h"
#include "PointStyle.h"
#include <QApplication>
#include <QGraphicsItem>
#include "QtToString.h"
#include "Transformation.h"

GraphicsScene::GraphicsScene(MainWindow *mainWindow) :
  QGraphicsScene(mainWindow)
{
}

GraphicsPoint *GraphicsScene::addPoint (const QString &identifier,
                                        const PointStyle &pointStyle,
                                        const QPointF &posScreen)
{
  double ordinal = maxOrdinal () + 1;

  LOG4CPP_INFO_S ((*mainCat)) << "GraphicsScene::addPoint"
                              << " identifier=" << identifier.toLatin1().data()
                              << " ordinal=" << ordinal;

  // Ordinal value is initially computed as one plus the max ordinal seen so far. This initial ordinal value will be overridden if the
  // cordinates determine the ordinal values.
  //
  // This is an N-squared algorithm and may be worth replacing later
  GraphicsPointFactory pointFactory;
  GraphicsPoint *point = pointFactory.createPoint (*this,
                                                   identifier,
                                                   posScreen,
                                                   pointStyle,
                                                   ordinal);

  point->setToolTip (identifier);
  point->setData (DATA_KEY_GRAPHICS_ITEM_TYPE, GRAPHICS_ITEM_TYPE_POINT);

  // This adds the GraphicsPoint, and also its GraphicsLinesForCurve if that does not already exist
  QString curveName = Point::curveNameFromPointIdentifier (identifier);
  m_graphicsLinesForCurves.savePoint (*this,
                                      curveName,
                                      identifier,
                                      ordinal,
                                      *point);

  return point;
}

QString GraphicsScene::dumpCursors () const
{
  QString cursorOverride = (QApplication::overrideCursor () != 0) ?
                             QtCursorToString (QApplication::overrideCursor ()->shape ()) :
                             "<null>";
  QString cursorImage = QtCursorToString (image()->cursor().shape ());

  QString dump = QString ("overrideCursor=%1 imageCursor=%2")
      .arg (cursorOverride)
      .arg (cursorImage);

  return dump;
}

const QGraphicsPixmapItem *GraphicsScene::image () const
{
  // Loop through items in scene to find the image
  QList<QGraphicsItem*> items = QGraphicsScene::items();
  QList<QGraphicsItem*>::iterator itr;
  for (itr = items.begin(); itr != items.end(); itr++) {

    QGraphicsItem* item = *itr;
    if (item->data (DATA_KEY_GRAPHICS_ITEM_TYPE).toInt () == GRAPHICS_ITEM_TYPE_IMAGE) {

      return (QGraphicsPixmapItem *) item;
    }
  }

  ENGAUGE_ASSERT (false);
  return 0;
}

double GraphicsScene::maxOrdinal () const
{
  // LOG4CPP_INFO_S is on exit

  double maxOrdinal = 0;

  const QList<QGraphicsItem*> &items = QGraphicsScene::items();
  QList<QGraphicsItem*>::const_iterator itr;
  for (itr = items.begin(); itr != items.end(); itr++) {

    const QGraphicsItem *item = *itr;

    // Only look at the Points
    bool isPoint = (item->data (DATA_KEY_GRAPHICS_ITEM_TYPE).toInt () == GRAPHICS_ITEM_TYPE_POINT);
    if (isPoint) {

      // Save if max value so far
      double ordinal = item->data (DATA_KEY_ORDINAL).toDouble ();
      maxOrdinal = qMax (maxOrdinal, ordinal);
    }
  }

  LOG4CPP_INFO_S ((*mainCat)) << "GraphicsScene::maxOrdinal maxOrdinal=" << maxOrdinal;

  return maxOrdinal;
}

QStringList GraphicsScene::positionHasChangedPointIdentifiers () const
{
  LOG4CPP_INFO_S ((*mainCat)) << "GraphicsScene::positionHasChangedPointIdentifiers";

  QStringList movedIds;

  const QList<QGraphicsItem*> &items = QGraphicsScene::items();
  QList<QGraphicsItem*>::const_iterator itr;
  for (itr = items.begin(); itr != items.end(); itr++) {

    const QGraphicsItem *item = *itr;

    // Skip the image and only keep the Points
    bool isPoint = (item->data (DATA_KEY_GRAPHICS_ITEM_TYPE).toInt () == GRAPHICS_ITEM_TYPE_POINT);
    if (isPoint) {

      QString identifier = item->data (DATA_KEY_IDENTIFIER).toString ();
      bool positionHasChanged = item->data (DATA_KEY_POSITION_HAS_CHANGED).toBool ();

      LOG4CPP_DEBUG_S ((*mainCat)) << "GraphicsScene::positionHasChangedPointIdentifiers"
                                   << " identifier=" << identifier.toLatin1().data()
                                   << " positionHasChanged=" << (positionHasChanged ? "yes" : "no");

      if (isPoint && positionHasChanged) {

        // Add Point to the list
        movedIds << item->data(DATA_KEY_IDENTIFIER).toString ();

      }
    }
  }

  return  movedIds;
}

void GraphicsScene::removePoint (const QString &identifier)
{
  LOG4CPP_INFO_S ((*mainCat)) << "GraphicsScene::removePoint identifier=" << identifier.toLatin1().data();

  m_graphicsLinesForCurves.removePoint (identifier);
}

void GraphicsScene::resetPositionHasChangedFlags()
{
  LOG4CPP_INFO_S ((*mainCat)) << "GraphicsScene::resetPositionHasChangedFlags";

  QList<QGraphicsItem*> itms = items ();
  QList<QGraphicsItem*>::const_iterator itr;
  for (itr = itms.begin (); itr != itms.end (); itr++) {

    QGraphicsItem *item = *itr;
    item->setData (DATA_KEY_POSITION_HAS_CHANGED, false);
  }
}

QStringList GraphicsScene::selectedPointIdentifiers () const
{
  QStringList selectedIds;

  const QList<QGraphicsItem*> &items = QGraphicsScene::items();
  QList<QGraphicsItem*>::const_iterator itr;
  for (itr = items.begin(); itr != items.end(); itr++) {

    const QGraphicsItem* item = *itr;

    // Skip the image and only keep the Points
    bool isPoint = (item->data (DATA_KEY_GRAPHICS_ITEM_TYPE).toInt () == GRAPHICS_ITEM_TYPE_POINT);
    bool isSelected = item->isSelected ();
    if (isPoint && isSelected) {

      // Add Point to the list
      selectedIds << item->data(DATA_KEY_IDENTIFIER).toString ();

    }
  }

  return  selectedIds;
}

void GraphicsScene::showPoints (bool show,
                                bool showAll,
                                const QString &curveNameWanted)
{
  const QList<QGraphicsItem*> &items = QGraphicsScene::items();
  QList<QGraphicsItem*>::const_iterator itr;
  for (itr = items.begin(); itr != items.end(); itr++) {

    QGraphicsItem* item = *itr;

    // Skip the image and only process the Points
    bool isPoint = (item->data (DATA_KEY_GRAPHICS_ITEM_TYPE).toInt () == GRAPHICS_ITEM_TYPE_POINT);
    if (isPoint) {

      bool showThisPoint = show;
      if (show && !showAll) {
        QString identifier = item->data (DATA_KEY_IDENTIFIER).toString ();
        QString curveNameGot = Point::curveNameFromPointIdentifier (identifier);

        showThisPoint = (curveNameWanted == curveNameGot);
      }

      item->setVisible (showThisPoint);

    }
  }
}

void GraphicsScene::updateAfterCommand (CmdMediator &cmdMediator,
                                        bool linesAreAlreadyUpdated)
{
  LOG4CPP_INFO_S ((*mainCat)) << "GraphicsScene::updateAfterCommand";

  // Update the points
  updatePointMembership (cmdMediator);

  if (!linesAreAlreadyUpdated) {

    // Update the lines between the points
    updateLineMembershipForPoints (cmdMediator);

  }
}

void GraphicsScene::updateCurveStyles (const CurveStyles &modelCurveStyles)
{
  LOG4CPP_INFO_S ((*mainCat)) << "GraphicsScene::updateCurveStyles";
}

void GraphicsScene::updateGraphicsLinesToMatchGraphicsPoints (const CurveStyles &curveStyles,
                                                              const Transformation &transformation)
{
  LOG4CPP_INFO_S ((*mainCat)) << "GraphicsScene::updateGraphicsLinesToMatchGraphicsPoints";

  if (transformation.transformIsDefined()) {

    // Ordinals must be updated to reflect reordering that may have resulted from dragging points
    m_graphicsLinesForCurves.updatePointOrdinalsAfterDrag (curveStyles,
                                                           transformation);

    // Recompute the lines one time for efficiency
    m_graphicsLinesForCurves.updateGraphicsLinesToMatchGraphicsPoints (curveStyles);
  }
}

void GraphicsScene::updateLineMembershipForPoints (CmdMediator &cmdMediator)
{
  LOG4CPP_INFO_S ((*mainCat)) << "GraphicsScene::updateLineMembershipForPoints";
}

void GraphicsScene::updatePointMembership (CmdMediator &cmdMediator)
{
  LOG4CPP_INFO_S ((*mainCat)) << "GraphicsScene::updatePointMembership";

  CallbackSceneUpdateAfterCommand ftor (m_graphicsLinesForCurves,
                                        *this,
                                        cmdMediator.document ());
  Functor2wRet<const QString &, const Point &, CallbackSearchReturn> ftorWithCallback = functor_ret (ftor,
                                                                                                     &CallbackSceneUpdateAfterCommand::callback);

  // First pass:
  // 1) Mark all points as Not Wanted (this is done while creating the map)
  m_graphicsLinesForCurves.lineMembershipReset ();

  // Next pass:
  // 1) Existing points that are found in the map are marked as Wanted
  // 2) Add new points that were just created in the Document. The new points are marked as Wanted
  cmdMediator.iterateThroughCurvePointsAxes (ftorWithCallback);
  cmdMediator.iterateThroughCurvesPointsGraphs (ftorWithCallback);

  // Next pass:
  // 1) Remove points that were just removed from the Document
  m_graphicsLinesForCurves.lineMembershipPurge ();
}
