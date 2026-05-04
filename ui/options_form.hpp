#ifndef CMDX_OPTIONS_FORM_HPP
#define CMDX_OPTIONS_FORM_HPP

#include "../core/tool_def.hpp"
#include "option_widget.hpp"
#include <QWidget>
#include <QScrollArea>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QGroupBox>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QLabel>
#include <map>
#include <string>

class OptionsForm : public QWidget {
    Q_OBJECT

public:
    explicit OptionsForm(QWidget* parent = nullptr);

    void load_tool(const core::tool_def& tool);
    void clear();

signals:
    void command_changed(const QString& command);
    void run_requested(const QString& command);

private slots:
    void on_subcommand_changed(int index);
    void on_value_changed();
    void on_run_clicked();

private:
    core::tool_def tool_;
    QVBoxLayout* main_layout_ = nullptr;
    QScrollArea* scroll_area_ = nullptr;
    QWidget* scroll_content_ = nullptr;
    QVBoxLayout* form_layout_ = nullptr;
    QComboBox* subcommand_combo_ = nullptr;
    QLineEdit* command_preview_ = nullptr;
    QCheckBox* sudo_checkbox_ = nullptr;
    QPushButton* run_button_ = nullptr;
    std::map<std::string, OptionWidget*> widgets_;
    std::map<std::string, QLineEdit*> positional_widgets_;

    void build_form(const std::vector<core::option_def>& options,
                    const std::vector<core::group_def>& groups,
                    const std::vector<core::positional_def>& positionals,
                    const std::vector<core::mutex_group_def>& mutex_groups);
    void setup_depends_on(const std::vector<core::option_def>& options);
    void setup_mutex_groups(const std::vector<core::option_def>& options,
                           const std::vector<core::mutex_group_def>& mutex_groups);
    void update_command();
    bool all_required_filled() const;
};

#endif // CMDX_OPTIONS_FORM_HPP
