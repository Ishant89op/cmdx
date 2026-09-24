#include "main_window.hpp"
#include "../core/command_cache.hpp"
#include <QApplication>
#include <QMessageBox>

MainWindow::MainWindow(const core::tool_def& tool, bool use_cache, QWidget* parent)
    : QMainWindow(parent), tool_(tool), use_cache_(use_cache) {

    setWindowTitle(QString::fromStdString("cmdx - " + tool_.meta.cmd));
    resize(700, 600);

    setup_menu();

    options_form_ = new OptionsForm(this);
    setCentralWidget(options_form_);
    options_form_->load_tool(tool_);

    if (use_cache_) {
        auto cached = core::command_cache::load(tool_.meta.cmd);
        if (cached.has_value()) {
            options_form_->apply_config(*cached);
            statusBar()->showMessage(QString::fromStdString(
                "Restored previous configuration for " + tool_.meta.cmd
            ), 5000);
        } else {
            statusBar()->showMessage(QString::fromStdString(
                tool_.meta.cmd + " - " + tool_.meta.description
            ));
        }
    } else {
        statusBar()->showMessage(QString::fromStdString(
            tool_.meta.cmd + " - " + tool_.meta.description
        ));
    }

    connect(options_form_, &OptionsForm::command_changed,
            this, &MainWindow::on_command_changed);
    connect(options_form_, &OptionsForm::run_requested,
            this, &MainWindow::on_run_requested);
}

void MainWindow::setup_menu() {
    auto* file_menu = menuBar()->addMenu("&File");
    file_menu->addAction("&Quit", QKeySequence::Quit, qApp, &QApplication::quit);

    auto* edit_menu = menuBar()->addMenu("&Edit");
    edit_menu->addAction("&Reset to Defaults", QKeySequence(Qt::CTRL | Qt::Key_R), this, &MainWindow::on_reset_defaults);
    edit_menu->addAction("&Clear Saved Configuration", this, &MainWindow::on_clear_cache);

    auto* help_menu = menuBar()->addMenu("&Help");
    help_menu->addAction("&About", this, &MainWindow::on_about);
}

std::string MainWindow::get_command() const {
    return final_command_;
}

core::cached_command_config MainWindow::get_current_config() const {
    if (options_form_) {
        return options_form_->get_current_config();
    }
    return core::cached_command_config{};
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

void MainWindow::on_reset_defaults() {
    if (options_form_) {
        options_form_->reset_defaults();
        statusBar()->showMessage("Reset options to default values", 3000);
    }
}

void MainWindow::on_clear_cache() {
    core::command_cache::clear(tool_.meta.cmd);
    if (options_form_) {
        options_form_->reset_defaults();
        statusBar()->showMessage("Cleared saved configuration and reset form", 3000);
    }
}

void MainWindow::on_about() {
    QMessageBox::about(this, "About cmdx",
        "cmdx - Visual command builder and runner.\n\n"
        "Configure options and click Run to execute the command\n"
        "in your terminal."
    );
}
