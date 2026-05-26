#pragma once

#include "edger/config.hpp"

#include <QMainWindow>
#include <QString>

class QCheckBox;
class QComboBox;
class QLineEdit;
class QSpinBox;
class QStatusBar;
class QTableWidget;
class RoiEditorWidget;

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit MainWindow(edger::Config config, QString config_path,
                      QWidget* parent = nullptr);

 private slots:
  void AddChannel();
  void RemoveChannel();
  void SaveConfig();

 private:
  void BuildUi();
  void LoadFromConfig();
  void ApplyToConfig();

  edger::Config config_;
  QString config_path_;

  QTableWidget* channel_table_ = nullptr;
  QLineEdit* record_root_edit_ = nullptr;
  QSpinBox* retention_spin_ = nullptr;
  QSpinBox* storage_spin_ = nullptr;
  QSpinBox* segment_spin_ = nullptr;
  QLineEdit* webhook_edit_ = nullptr;
  QSpinBox* cooldown_spin_ = nullptr;
  QCheckBox* intrusion_enabled_ = nullptr;
  QComboBox* intrusion_channel_ = nullptr;
  RoiEditorWidget* roi_editor_ = nullptr;
  QStatusBar* status_bar_ = nullptr;
};
