#include "main_window.hpp"

#include "edger/config.hpp"

#include <QApplication>
#include <QCommandLineParser>
#include <QMessageBox>

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  app.setApplicationName("edger-qt-config");
  app.setApplicationVersion("0.1.0");

  QCommandLineParser parser;
  parser.setApplicationDescription("EdgeRec Alert Qt 配置工具");
  parser.addHelpOption();
  parser.addVersionOption();
  QCommandLineOption config_option(
      QStringList() << "c" << "config",
      "配置文件路径", "path", "config/example.json");
  parser.addOption(config_option);
  parser.process(app);

  const QString config_path = parser.value(config_option);

  edger::Config config;
  if (!config.Load(config_path.toStdString())) {
    QMessageBox::critical(nullptr, "加载失败",
                          QString("无法读取配置: %1").arg(config_path));
    return 1;
  }

  MainWindow window(std::move(config), config_path);
  window.show();
  return app.exec();
}
