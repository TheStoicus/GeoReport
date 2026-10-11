#pragma once

#include <QMainWindow>
#include <QString>

class QLabel;
class QLineEdit;
class QListWidget;

class MainWindow final : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    void createMenus();
    void openProjectDirectory();
    void updateProjectView(const QString &directoryPath);

    QLabel *projectPathLabel_ = nullptr;
    QLabel *statusLabel_ = nullptr;
    QListWidget *moduleList_ = nullptr;
    QLineEdit *projectNameEdit_ = nullptr;
    QString projectDirectory_;
};
