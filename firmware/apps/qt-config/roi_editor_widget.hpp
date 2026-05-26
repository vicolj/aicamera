#pragma once

#include <QGraphicsView>
#include <QWidget>

#include <utility>
#include <vector>

class RoiEditorWidget : public QWidget {
  Q_OBJECT

 public:
  explicit RoiEditorWidget(QWidget* parent = nullptr);

  void SetPolygon(const std::vector<std::pair<float, float>>& normalized);
  std::vector<std::pair<float, float>> Polygon() const;

 private:
  class HandleItem;

  void EnsureDefaultPolygon();
  void SyncPolygonFromHandles();

  QGraphicsView* view_ = nullptr;
  QGraphicsScene* scene_ = nullptr;
  QGraphicsPolygonItem* polygon_item_ = nullptr;
  std::vector<HandleItem*> handles_;
};
