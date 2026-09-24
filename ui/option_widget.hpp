#ifndef CMDX_OPTION_WIDGET_HPP
#define CMDX_OPTION_WIDGET_HPP

#include "../core/tool_def.hpp"
#include <QWidget>
#include <QCheckBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QString>

class OptionWidget : public QWidget {
    Q_OBJECT

public:
    explicit OptionWidget(const core::option_def& opt, QWidget* parent = nullptr);

    std::string get_value() const;
    void set_value(const std::string& value);
    void reset_to_default();
    void set_enabled(bool enabled);
    const core::option_def& option() const;

signals:
    void value_changed();

private:
    core::option_def opt_;
    QCheckBox* checkbox_ = nullptr;
    QLineEdit* line_edit_ = nullptr;
    QSpinBox* spin_box_ = nullptr;
    QDoubleSpinBox* double_spin_box_ = nullptr;
    QComboBox* combo_box_ = nullptr;
    QPushButton* browse_button_ = nullptr;

    void setup_bool();
    void setup_string();
    void setup_int();
    void setup_float();
    void setup_enum();
    void setup_file_path();
    void setup_dir_path();
    void setup_string_list();
    void on_browse_file();
    void on_browse_dir();
};

#endif // CMDX_OPTION_WIDGET_HPP
