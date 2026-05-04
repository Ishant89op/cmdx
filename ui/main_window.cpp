#include "main_window.hpp"
#include <QApplication>
#include <QMessageBox>

MainWindow::MainWindow(const core::tool_def& tool, QWidget* parent)
    : QMainWindow(parent), tool_(tool) {

    setWindowTitle(QString::fromStdString("cmdx - " + tool_.meta.cmd));
    resize(700, 600);

    setup_menu();

    options_form_ = new OptionsForm(this);
    setCentralWidget(options_form_);
    options_form_->load_tool(tool_);

    statusBar()->showMessage(QString::fromStdString(
        tool_.meta.cmd + " - " + tool_.meta.description
    ));

    connect(options_form_, &OptionsForm::command_changed,
            this, &MainWindow::on_command_changed);
    connect(options_form_, &OptionsForm::run_requested,
            this, &MainWindow::on_run_requested);
}

void MainWindow::setup_menu() {
    auto* file_menu = menuBar()->addMenu("&File");
    file_menu->addAction("&Quit", QKeySequence::Quit, qApp, &QApplication::quit);

    auto* help_menu = menuBar()->addMenu("&Help");
    help_menu->addAction("&About", this, &MainWindow::on_about);
}

std::string MainWindow::get_command() const {
    return final_command_;
}

bool MainWindow::was_run_requested() const {
    return run_requested_;
}

void MainWindow::on_command_changed(const QString& command) {
    statusBar()->showMessage(command);
}

void MainWindow::on_run_requested(const QString& command) {
    final_command_ = command.toStdString();
    run_requested_ = true;
    close();
}

void MainWindow::on_about() {
    QMessageBox::about(this, "About cmdx",
        "cmdx - GUI and terminal frontend for Linux CLI tools.\n\n"
        "Configure options and click Run to execute the command\n"
        "in your terminal."
    );
}
