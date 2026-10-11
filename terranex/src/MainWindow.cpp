#include "MainWindow.h"

#include <QAction>
#include <QDir>
#include <QFileDialog>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("TerraNex — геологическая система"));
    resize(1080, 700);

    auto *central = new QWidget(this);
    auto *layout = new QVBoxLayout(central);
    layout->setContentsMargins(28, 24, 28, 24);
    layout->setSpacing(16);

    auto *heading = new QLabel(tr("TerraNex"), central);
    QFont headingFont = heading->font();
    headingFont.setPointSize(26);
    headingFont.setBold(true);
    heading->setFont(headingFont);

    auto *subtitle = new QLabel(
        tr("Рабочее пространство геологического проекта"), central);
    subtitle->setWordWrap(true);

    auto *projectForm = new QFormLayout();
    projectNameEdit_ = new QLineEdit(central);
    projectNameEdit_->setPlaceholderText(tr("Например: Лещинское"));
    projectNameEdit_->setClearButtonEnabled(true);
    projectForm->addRow(tr("Название проекта:"), projectNameEdit_);

    projectPathLabel_ = new QLabel(tr("Папка проекта не выбрана"), central);
    projectPathLabel_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    projectPathLabel_->setWordWrap(true);

    auto *openButton = new QPushButton(tr("Открыть папку проекта…"), central);
    connect(openButton, &QPushButton::clicked,
            this, &MainWindow::openProjectDirectory);

    auto *modulesHeading = new QLabel(tr("Модули системы"), central);
    QFont sectionFont = modulesHeading->font();
    sectionFont.setPointSize(14);
    sectionFont.setBold(true);
    modulesHeading->setFont(sectionFont);

    moduleList_ = new QListWidget(central);
    moduleList_->addItem(tr("Геологические данные — подготовка"));
    moduleList_->addItem(tr("Скважины и интервалы — подготовка"));
    moduleList_->addItem(tr("Пробы и лабораторные анализы — подготовка"));
    moduleList_->addItem(tr("Классификация и нормативы — подготовка"));
    moduleList_->addItem(tr("Запасы и статистика — подготовка"));
    moduleList_->addItem(tr("Планы и разрезы — подготовка"));
    moduleList_->addItem(tr("Отчёты Excel / DOCX — подготовка"));

    auto *notice = new QLabel(
        tr("Это начальная оболочка TerraNex. Расчётные алгоритмы GeoReport "
           "будут переноситься поэтапно после фиксации контрольных результатов. "
           "На этом этапе оболочка не изменяет исходные данные и не запускает "
           "геологические расчёты."), central);
    notice->setWordWrap(true);

    layout->addWidget(heading);
    layout->addWidget(subtitle);
    layout->addLayout(projectForm);
    layout->addWidget(projectPathLabel_);
    layout->addWidget(openButton, 0, Qt::AlignLeft);
    layout->addWidget(modulesHeading);
    layout->addWidget(moduleList_, 1);
    layout->addWidget(notice);

    setCentralWidget(central);
    createMenus();

    statusLabel_ = new QLabel(tr("Готово"), this);
    statusBar()->addWidget(statusLabel_, 1);
}

void MainWindow::createMenus()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("Файл"));

    QAction *openAction = fileMenu->addAction(tr("Открыть папку проекта…"));
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered,
            this, &MainWindow::openProjectDirectory);

    fileMenu->addSeparator();
    QAction *exitAction = fileMenu->addAction(tr("Выход"));
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    QMenu *helpMenu = menuBar()->addMenu(tr("Справка"));
    QAction *aboutAction = helpMenu->addAction(tr("О TerraNex"));
    connect(aboutAction, &QAction::triggered, this, [this]() {
        QMessageBox::about(
            this,
            tr("О TerraNex"),
            tr("TerraNex 0.1.0\n"
               "Настольная геологическая система.\n\n"
               "Проект развивается на основе проверенных алгоритмов GeoReport. "
               "Существующая консольная программа сохраняется отдельно."));
    });
}

void MainWindow::openProjectDirectory()
{
    const QString selectedDirectory = QFileDialog::getExistingDirectory(
        this,
        tr("Выберите папку геологического проекта"),
        projectDirectory_.isEmpty() ? QDir::homePath() : projectDirectory_);

    if (selectedDirectory.isEmpty()) {
        return;
    }

    updateProjectView(selectedDirectory);
}

void MainWindow::updateProjectView(const QString &directoryPath)
{
    projectDirectory_ = QDir(directoryPath).absolutePath();
    projectPathLabel_->setText(projectDirectory_);

    if (projectNameEdit_->text().trimmed().isEmpty()) {
        projectNameEdit_->setText(QDir(projectDirectory_).dirName());
    }

    statusLabel_->setText(tr("Выбрана папка проекта: %1").arg(projectDirectory_));
}
