#ifndef CMDX_MAIN_WINDOW_HPP
#define CMDX_MAIN_WINDOW_HPP

#include "../core/tool_def.hpp"
#include "options_form.hpp"
#include <QMainWindow>
#include <QStatusBar>
#include <QMenuBar>
#include <string>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(const core::tool_def& tool, bool use_cache = true, QWidget* parent = nullptr);

    std::string get_command() const;
    core::cached_command_config get_current_config() const;
    bool was_run_requested() const;

private slots:
    void on_command_changed(const QString& command);
    void on_run_requested(const QString& command);
    void on_reset_defaults();
    void on_clear_cache();
    void on_about();

private:
    core::tool_def tool_;
    bool use_cache_ = true;
    OptionsForm* options_form_ = nullptr;
    std::string final_command_;
    bool run_requested_ = false;

    void setup_menu();
};

#endif // CMDX_MAIN_WINDOW_HPP
