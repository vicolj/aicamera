#include "roi_editor_widget.hpp"

#include <QGraphicsEllipseItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsScene>
#include <QPen>
#include <QVBoxLayout>

namespace {

constexpr int kSceneWidth = 640;
constexpr int kSceneHeight = 480;
constexpr qreal kHandleRadius = 8.0;

}  // namespace

class RoiEditorWidget::HandleItem : public QGraphicsEllipseItem {
 public:
  HandleItem(int index, RoiEditorWidget* editor)
      : QGraphicsEllipseItem(-kHandleRadius, -kHandleRadius, kHandleRadius * 2,
                             kHandleRadius * 2),
        index_(index),
        editor_(editor) {
    setBrush(Qt::yellow);
    setPen(QPen(Qt::black, 1));
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setZValue(10);
  }

 protected:
  QVariant itemChange(GraphicsItemChange change, const QVariant& value) override {
    if (change == ItemPositionChange && scene() != nullptr) {
      QPointF pos = value.toPointF();
      pos.setX(qBound(0.0, pos.x(), static_cast<qreal>(kSceneWidth)));
      pos.setY(qBound(0.0, pos.y(), static_cast<qreal>(kSceneHeight)));
      return pos;
    }
    if (change == ItemPositionHasChanged && editor_ != nullptr) {
      editor_->SyncPolygonFromHandles();
    }
    return QGraphicsEllipseItem::itemChange(change, value);
  }

 private:
  int index_;
  RoiEditorWidget* editor_;
};

RoiEditorWidget::RoiEditorWidget(QWidget* parent) : QWidget(parent) {
  scene_ = new QGraphicsScene(0, 0, kSceneWidth, kSceneHeight, this);
  scene_->setBackgroundBrush(QBrush(QColor(30, 30, 30)));

  polygon_item_ = new QGraphicsPolygonItem();
  polygon_item_->setPen(QPen(Qt::green, 2));
  polygon_item_->setBrush(QColor(0, 255, 0, 40));
  scene_->addItem(polygon_item_);

  for (int i = 0; i < 4; ++i) {
    auto* handle = new HandleItem(i, this);
    scene_->addItem(handle);
    handles_.push_back(handle);
  }

  view_ = new QGraphicsView(scene_, this);
  view_->setRenderHint(QPainter::Antialiasing, true);
  view_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  view_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  view_->setFixedSize(kSceneWidth + 2, kSceneHeight + 2);

  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(view_);

  EnsureDefaultPolygon();
}

void RoiEditorWidget::EnsureDefaultPolygon() {
  SetPolygon({{0.1f, 0.1f}, {0.9f, 0.1f}, {0.9f, 0.9f}, {0.1f, 0.9f}});
}

void RoiEditorWidget::SetPolygon(
    const std::vector<std::pair<float, float>>& normalized) {
  std::vector<std::pair<float, float>> points = normalized;
  if (points.size() < 3) {
    points = {{0.1f, 0.1f}, {0.9f, 0.1f}, {0.9f, 0.9f}, {0.1f, 0.9f}};
  }

  QPolygonF polygon;
  for (size_t i = 0; i < points.size(); ++i) {
    const auto& [nx, ny] = points[i];
    const QPointF pos(nx * kSceneWidth, ny * kSceneHeight);
    polygon << pos;
    if (i < handles_.size()) {
      handles_[i]->setPos(pos);
    }
  }

  polygon_item_->setPolygon(polygon);
}

std::vector<std::pair<float, float>> RoiEditorWidget::Polygon() const {
  const QPolygonF polygon = polygon_item_->polygon();
  std::vector<std::pair<float, float>> normalized;
  normalized.reserve(static_cast<size_t>(polygon.size()));

  for (const QPointF& point : polygon) {
    normalized.emplace_back(
        static_cast<float>(point.x() / kSceneWidth),
        static_cast<float>(point.y() / kSceneHeight));
  }

  return normalized;
}

void RoiEditorWidget::SyncPolygonFromHandles() {
  QPolygonF polygon;
  for (const auto* handle : handles_) {
    polygon << handle->pos();
  }
  polygon_item_->setPolygon(polygon);
}
