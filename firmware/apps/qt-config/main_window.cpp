#include "main_window.hpp"
#include "roi_editor_widget.hpp"

#include "edger/config_reload.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QStatusBar>
#include <QTabWidget>
#include <QTableWidget>
#include <QVBoxLayout>

MainWindow::MainWindow(edger::Config config, QString config_path,
                       QWidget* parent)
    : QMainWindow(parent),
      config_(std::move(config)),
      config_path_(std::move(config_path)) {
  setWindowTitle("EdgeRec Alert 配置");
  resize(900, 700);
  BuildUi();
  LoadFromConfig();
}

void MainWindow::BuildUi() {
  auto* tabs = new QTabWidget(this);

  auto* channel_page = new QWidget(tabs);
  auto* channel_layout = new QVBoxLayout(channel_page);
  channel_table_ = new QTableWidget(0, 4, channel_page);
  channel_table_->setHorizontalHeaderLabels({"ID", "名称", "RTSP URL", "启用"});
  channel_table_->horizontalHeader()->setStretchLastSection(true);
  channel_table_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
  channel_layout->addWidget(channel_table_);

  auto* channel_buttons = new QHBoxLayout();
  auto* add_button = new QPushButton("添加通道", channel_page);
  auto* remove_button = new QPushButton("删除选中", channel_page);
  connect(add_button, &QPushButton::clicked, this, &MainWindow::AddChannel);
  connect(remove_button, &QPushButton::clicked, this, &MainWindow::RemoveChannel);
  channel_buttons->addWidget(add_button);
  channel_buttons->addWidget(remove_button);
  channel_buttons->addStretch();
  channel_layout->addLayout(channel_buttons);
  tabs->addTab(channel_page, "通道");

  auto* record_page = new QWidget(tabs);
  auto* record_form = new QFormLayout(record_page);
  record_root_edit_ = new QLineEdit(record_page);
  retention_spin_ = new QSpinBox(record_page);
  retention_spin_->setRange(1, 365);
  storage_spin_ = new QSpinBox(record_page);
  storage_spin_->setRange(0, 10000);
  storage_spin_->setSpecialValueText("不限制");
  segment_spin_ = new QSpinBox(record_page);
  segment_spin_->setRange(60, 3600);
  record_form->addRow("录像目录", record_root_edit_);
  record_form->addRow("保留天数", retention_spin_);
  record_form->addRow("容量上限 (GB)", storage_spin_);
  record_form->addRow("分片时长 (秒)", segment_spin_);
  tabs->addTab(record_page, "录像");

  auto* alert_page = new QWidget(tabs);
  auto* alert_layout = new QVBoxLayout(alert_page);
  auto* alert_form = new QFormLayout();
  webhook_edit_ = new QLineEdit(alert_page);
  cooldown_spin_ = new QSpinBox(alert_page);
  cooldown_spin_->setRange(1, 3600);
  intrusion_enabled_ = new QCheckBox("启用区域入侵", alert_page);
  intrusion_channel_ = new QComboBox(alert_page);
  alert_form->addRow("Webhook URL", webhook_edit_);
  alert_form->addRow("告警冷却 (秒)", cooldown_spin_);
  alert_form->addRow(intrusion_enabled_);
  alert_form->addRow("检测通道", intrusion_channel_);
  alert_layout->addLayout(alert_form);

  auto* roi_group = new QGroupBox("检测区域（拖动角点）", alert_page);
  auto* roi_layout = new QVBoxLayout(roi_group);
  roi_editor_ = new RoiEditorWidget(roi_group);
  roi_layout->addWidget(roi_editor_);
  alert_layout->addWidget(roi_group);
  tabs->addTab(alert_page, "告警 / ROI");

  auto* central = new QWidget(this);
  auto* root_layout = new QVBoxLayout(central);
  root_layout->addWidget(tabs);

  auto* save_row = new QHBoxLayout();
  auto* save_button = new QPushButton("保存并通知重载", central);
  connect(save_button, &QPushButton::clicked, this, &MainWindow::SaveConfig);
  save_row->addStretch();
  save_row->addWidget(save_button);
  root_layout->addLayout(save_row);

  setCentralWidget(central);
  status_bar_ = statusBar();
}

void MainWindow::LoadFromConfig() {
  record_root_edit_->setText(QString::fromStdString(config_.app().record_root));
  retention_spin_->setValue(config_.app().retention_days);
  storage_spin_->setValue(config_.app().max_storage_gb);
  segment_spin_->setValue(config_.app().segment_sec);
  webhook_edit_->setText(QString::fromStdString(config_.alert().webhook_url));
  cooldown_spin_->setValue(config_.alert().cooldown_sec);
  intrusion_enabled_->setChecked(config_.ai().intrusion.enabled);

  channel_table_->setRowCount(0);
  intrusion_channel_->clear();
  for (const auto& ch : config_.channels()) {
    const int row = channel_table_->rowCount();
    channel_table_->insertRow(row);
    channel_table_->setItem(row, 0, new QTableWidgetItem(QString::number(ch.id)));
    channel_table_->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(ch.name)));
    channel_table_->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(ch.rtsp_url)));

    auto* enabled_item = new QTableWidgetItem();
    enabled_item->setCheckState(ch.enabled ? Qt::Checked : Qt::Unchecked);
    channel_table_->setItem(row, 3, enabled_item);

    intrusion_channel_->addItem(QString("[%1] %2").arg(ch.id).arg(QString::fromStdString(ch.name)),
                                ch.id);
  }

  const int channel_id = config_.ai().intrusion.channel_id;
  const int idx = intrusion_channel_->findData(channel_id);
  if (idx >= 0) {
    intrusion_channel_->setCurrentIndex(idx);
  }

  if (!config_.ai().intrusion.polygon.empty()) {
    roi_editor_->SetPolygon(config_.ai().intrusion.polygon);
  }
}

void MainWindow::ApplyToConfig() {
  config_.mutable_app().record_root = record_root_edit_->text().toStdString();
  config_.mutable_app().retention_days = retention_spin_->value();
  config_.mutable_app().max_storage_gb = storage_spin_->value();
  config_.mutable_app().segment_sec = segment_spin_->value();
  config_.mutable_alert().webhook_url = webhook_edit_->text().toStdString();
  config_.mutable_alert().cooldown_sec = cooldown_spin_->value();
  config_.mutable_ai().intrusion.enabled = intrusion_enabled_->isChecked();
  config_.mutable_ai().intrusion.channel_id = intrusion_channel_->currentData().toInt();
  config_.mutable_ai().intrusion.polygon = roi_editor_->Polygon();

  auto& channels = config_.mutable_channels();
  channels.clear();
  channels.reserve(channel_table_->rowCount());

  for (int row = 0; row < channel_table_->rowCount(); ++row) {
    edger::ChannelConfig ch;
    ch.id = channel_table_->item(row, 0)->text().toInt();
    ch.name = channel_table_->item(row, 1)->text().toStdString();
    ch.rtsp_url = channel_table_->item(row, 2)->text().toStdString();
    ch.enabled = channel_table_->item(row, 3)->checkState() == Qt::Checked;
    channels.push_back(ch);
  }
}

void MainWindow::AddChannel() {
  const int row = channel_table_->rowCount();
  channel_table_->insertRow(row);
  channel_table_->setItem(row, 0, new QTableWidgetItem(QString::number(row)));
  channel_table_->setItem(row, 1, new QTableWidgetItem(QString("cam-%1").arg(row + 1)));
  channel_table_->setItem(row, 2, new QTableWidgetItem("rtsp://"));
  auto* enabled_item = new QTableWidgetItem();
  enabled_item->setCheckState(Qt::Checked);
  channel_table_->setItem(row, 3, enabled_item);

  const int id = row;
  intrusion_channel_->addItem(QString("[%1] cam-%2").arg(id).arg(row + 1), id);
}

void MainWindow::RemoveChannel() {
  const int row = channel_table_->currentRow();
  if (row < 0) {
    return;
  }

  const int channel_id = channel_table_->item(row, 0)->text().toInt();
  channel_table_->removeRow(row);

  const int idx = intrusion_channel_->findData(channel_id);
  if (idx >= 0) {
    intrusion_channel_->removeItem(idx);
  }
}

void MainWindow::SaveConfig() {
  ApplyToConfig();

  if (!config_.Save(config_path_.toStdString())) {
    QMessageBox::critical(this, "保存失败", "无法写入配置文件");
    return;
  }

  if (!edger::ConfigReload::Notify(config_path_.toStdString())) {
    QMessageBox::warning(this, "重载通知失败", "配置已保存，但重载标记写入失败");
    return;
  }

  const auto generation =
      edger::ConfigReload::ReadGeneration(config_path_.toStdString());
      status_bar_->showMessage(
      QString("已保存 %1，重载 generation=%2").arg(config_path_).arg(generation));
}
