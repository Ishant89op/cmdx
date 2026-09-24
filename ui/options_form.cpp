#include "options_form.hpp"
#include "../core/command_builder.hpp"
#include <QMessageBox>
#include <QFileDialog>
#include <QStyle>
#include <QHBoxLayout>
#include <memory>

OptionsForm::OptionsForm(QWidget* parent) : QWidget(parent) {
    main_layout_ = new QVBoxLayout(this);

    scroll_area_ = new QScrollArea(this);
    scroll_area_->setWidgetResizable(true);
    scroll_content_ = new QWidget();
    form_layout_ = new QVBoxLayout(scroll_content_);
    scroll_area_->setWidget(scroll_content_);

    command_preview_ = new QLineEdit(this);
    command_preview_->setReadOnly(true);
    command_preview_->setPlaceholderText("Command preview will appear here...");

    sudo_checkbox_ = new QCheckBox("Run with sudo", this);

    run_button_ = new QPushButton("Run", this);
    run_button_->setEnabled(false);
    run_button_->setMinimumHeight(36);

    main_layout_->addWidget(scroll_area_);
    main_layout_->addWidget(new QLabel("Command Preview:", this));
    main_layout_->addWidget(command_preview_);

    auto* bottom_row = new QHBoxLayout();
    bottom_row->addWidget(sudo_checkbox_);
    bottom_row->addStretch();
    bottom_row->addWidget(run_button_);
    main_layout_->addLayout(bottom_row);

    connect(run_button_, &QPushButton::clicked, this, &OptionsForm::on_run_clicked);
    connect(sudo_checkbox_, &QCheckBox::toggled, this, &OptionsForm::on_value_changed);
}

void OptionsForm::clear() {
    widgets_.clear();
    positional_widgets_.clear();

    if (subcommand_combo_) {
        subcommand_combo_->deleteLater();
        subcommand_combo_ = nullptr;
    }

    QLayoutItem* child;
    while ((child = form_layout_->takeAt(0)) != nullptr) {
        if (child->widget()) {
            child->widget()->deleteLater();
        }
        if (child->layout()) {
            QLayoutItem* sub;
            while ((sub = child->layout()->takeAt(0)) != nullptr) {
                if (sub->widget()) {
                    sub->widget()->deleteLater();
                }
                delete sub;
            }
            delete child->layout();
        }
        delete child;
    }

    command_preview_->clear();
    run_button_->setEnabled(false);
}

void OptionsForm::load_tool(const core::tool_def& tool) {
    clear();
    tool_ = tool;

    if (tool_.meta.has_subcommands && !tool_.subcommands.empty()) {
        auto* label = new QLabel("Subcommand:", scroll_content_);
        form_layout_->addWidget(label);

        subcommand_combo_ = new QComboBox(scroll_content_);
        for (const auto& sc : tool_.subcommands) {
            subcommand_combo_->addItem(
                QString::fromStdString(sc.name),
                QString::fromStdString(sc.description)
            );
        }
        form_layout_->addWidget(subcommand_combo_);
        connect(subcommand_combo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &OptionsForm::on_subcommand_changed);

        const auto& sc = tool_.subcommands[0];
        build_form(sc.options, sc.groups, sc.positional, sc.mutex_groups);
    } else {
        build_form(tool_.options, tool_.groups, tool_.positional, tool_.mutex_groups);
    }

    update_command();
}

void OptionsForm::on_subcommand_changed(int index) {
    if (index < 0 || index >= static_cast<int>(tool_.subcommands.size())) {
        return;
    }

    widgets_.clear();
    positional_widgets_.clear();

    QLayoutItem* child;
    int start_index = 2;
    while (form_layout_->count() > start_index) {
        child = form_layout_->takeAt(start_index);
        if (child->widget()) {
            child->widget()->deleteLater();
        }
        delete child;
    }

    const auto& sc = tool_.subcommands[index];
    build_form(sc.options, sc.groups, sc.positional, sc.mutex_groups);
    update_command();
}

void OptionsForm::build_form(
    const std::vector<core::option_def>& options,
    const std::vector<core::group_def>& groups,
    const std::vector<core::positional_def>& positionals,
    const std::vector<core::mutex_group_def>& mutex_groups
) {
    std::map<std::string, QGroupBox*> group_boxes;
    std::map<std::string, QFormLayout*> group_layouts;

    for (const auto& g : groups) {
        auto* box = new QGroupBox(scroll_content_);
        box->setFlat(true);
        auto* layout = new QFormLayout(box);
        group_boxes[g.id] = box;
        group_layouts[g.id] = layout;

        bool start_collapsed = g.collapsed_by_default;

        auto* toggle = new QPushButton(
            QString::fromStdString(
                (start_collapsed ? "\u25b6  " : "\u25bc  ") + g.label
            ), scroll_content_
        );
        toggle->setFlat(true);
        toggle->setStyleSheet(
            "text-align: left; font-weight: bold; padding: 6px 4px; "
            "font-size: 13px; border-bottom: 1px solid #444;"
        );
        toggle->setCursor(Qt::PointingHandCursor);

        box->setVisible(!start_collapsed);

        connect(toggle, &QPushButton::clicked, [box, toggle, g]() {
            bool visible = !box->isVisible();
            box->setVisible(visible);
            toggle->setText(QString::fromStdString(
                (visible ? "\u25bc  " : "\u25b6  ") + g.label
            ));
        });

        form_layout_->addWidget(toggle);
        form_layout_->addWidget(box);
    }

    for (const auto& opt : options) {
        auto* widget = new OptionWidget(opt, scroll_content_);
        widgets_[opt.id] = widget;

        QString label_text = QString::fromStdString(opt.label);
        if (opt.required) {
            label_text += " *";
        }
        label_text += QString::fromStdString("  [" + opt.flag + "]");

        auto it = group_layouts.find(opt.group);
        if (it != group_layouts.end()) {
            it->second->addRow(label_text, widget);
        } else {
            auto* fallback = new QFormLayout();
            fallback->addRow(label_text, widget);
            form_layout_->addLayout(fallback);
        }

        connect(widget, &OptionWidget::value_changed, this, &OptionsForm::on_value_changed);
    }

    if (!positionals.empty()) {
        auto* pos_box = new QGroupBox("Positional Arguments", scroll_content_);
        auto* pos_layout = new QFormLayout(pos_box);
        for (const auto& pos : positionals) {
            auto* edit = new QLineEdit(scroll_content_);
            if (pos.default_value.has_value()) {
                edit->setText(QString::fromStdString(pos.default_value.value()));
            }
            if (pos.placeholder.has_value()) {
                edit->setPlaceholderText(QString::fromStdString(pos.placeholder.value()));
            }
            edit->setToolTip(QString::fromStdString(pos.description));

            QString label_text = QString::fromStdString(pos.label);
            if (pos.required) {
                label_text += " *";
            }

            positional_widgets_[pos.id] = edit;

            bool is_path = (pos.type == core::positional_type::DIR_PATH ||
                            pos.type == core::positional_type::FILE_PATH);

            if (is_path) {
                auto* row = new QWidget(scroll_content_);
                auto* row_layout = new QHBoxLayout(row);
                row_layout->setContentsMargins(0, 0, 0, 0);
                row_layout->addWidget(edit);

                auto* browse = new QPushButton(scroll_content_);
                browse->setIcon(browse->style()->standardIcon(QStyle::SP_DirOpenIcon));
                browse->setToolTip("Browse...");
                browse->setFixedWidth(32);
                row_layout->addWidget(browse);

                if (pos.type == core::positional_type::DIR_PATH) {
                    connect(browse, &QPushButton::clicked, [edit]() {
                        QString path = QFileDialog::getExistingDirectory(edit, "Select Directory");
                        if (!path.isEmpty()) { edit->setText(path); }
                    });
                } else {
                    connect(browse, &QPushButton::clicked, [edit]() {
                        QString path = QFileDialog::getOpenFileName(edit, "Select File");
                        if (!path.isEmpty()) { edit->setText(path); }
                    });
                }

                pos_layout->addRow(label_text, row);
            } else {
                pos_layout->addRow(label_text, edit);
            }

            connect(edit, &QLineEdit::textChanged, this, &OptionsForm::on_value_changed);
        }
        form_layout_->addWidget(pos_box);
    }

    setup_depends_on(options);
    setup_mutex_groups(options, mutex_groups);
    form_layout_->addStretch();
}

void OptionsForm::setup_depends_on(const std::vector<core::option_def>& options) {
    for (const auto& opt : options) {
        if (!opt.depends_on.has_value()) {
            continue;
        }

        auto dep_it = widgets_.find(opt.depends_on.value());
        auto self_it = widgets_.find(opt.id);
        if (dep_it == widgets_.end() || self_it == widgets_.end()) {
            continue;
        }

        OptionWidget* parent_w = dep_it->second;
        OptionWidget* child_w = self_it->second;

        bool parent_active = (parent_w->get_value() == "true");
        child_w->set_enabled(parent_active);

        connect(parent_w, &OptionWidget::value_changed, [parent_w, child_w]() {
            child_w->set_enabled(parent_w->get_value() == "true");
        });
    }
}

void OptionsForm::setup_mutex_groups(
    const std::vector<core::option_def>& options,
    const std::vector<core::mutex_group_def>& mutex_groups
) {
    std::map<std::string, std::vector<OptionWidget*>> mutex_map;

    for (const auto& opt : options) {
        if (!opt.mutex_group.has_value()) {
            continue;
        }
        if (opt.type != core::option_type::BOOL) {
            continue;
        }

        auto it = widgets_.find(opt.id);
        if (it != widgets_.end()) {
            mutex_map[opt.mutex_group.value()].push_back(it->second);
        }
    }

    for (auto& [group_id, group_widgets] : mutex_map) {
        auto shared_widgets = std::make_shared<std::vector<OptionWidget*>>(group_widgets);
        for (auto* w : *shared_widgets) {
            connect(w, &OptionWidget::value_changed, [w, shared_widgets]() {
                if (w->get_value() == "true") {
                    for (auto* other : *shared_widgets) {
                        if (other != w && other->get_value() == "true") {
                            other->set_enabled(true);
                            other->blockSignals(true);
                            auto* cb = other->findChild<QCheckBox*>();
                            if (cb) {
                                cb->setChecked(false);
                            }
                            other->blockSignals(false);
                        }
                    }
                }
            });
        }
    }
}

void OptionsForm::on_value_changed() {
    update_command();
}

void OptionsForm::update_command() {
    std::map<std::string, std::string> values;
    for (const auto& [id, widget] : widgets_) {
        values[id] = widget->get_value();
    }

    std::map<std::string, std::string> pos_values;
    for (const auto& [id, edit] : positional_widgets_) {
        pos_values[id] = edit->text().toStdString();
    }

    std::optional<std::string> subcmd;
    if (subcommand_combo_) {
        subcmd = subcommand_combo_->currentText().toStdString();
    }

    std::string cmd = core::build_command(tool_, values, subcmd, pos_values);
    if (sudo_checkbox_ && sudo_checkbox_->isChecked()) {
        cmd = "sudo " + cmd;
    }
    command_preview_->setText(QString::fromStdString(cmd));
    run_button_->setEnabled(all_required_filled());
    emit command_changed(QString::fromStdString(cmd));
}

bool OptionsForm::all_required_filled() const {
    for (const auto& [id, widget] : widgets_) {
        const auto& opt = widget->option();
        if (opt.required) {
            std::string val = widget->get_value();
            if (val.empty() || (opt.type == core::option_type::BOOL && val == "false")) {
                // BOOL required means it must be checked
                if (opt.type != core::option_type::BOOL) {
                    return false;
                }
            }
            if (opt.type != core::option_type::BOOL && val.empty()) {
                return false;
            }
        }
    }

    for (const auto& [id, edit] : positional_widgets_) {
        // check if this positional is required
        const auto* positionals = &tool_.positional;
        if (subcommand_combo_ && !tool_.subcommands.empty()) {
            int idx = subcommand_combo_->currentIndex();
            if (idx >= 0 && idx < static_cast<int>(tool_.subcommands.size())) {
                positionals = &tool_.subcommands[idx].positional;
            }
        }
        for (const auto& pos : *positionals) {
            if (pos.id == id && pos.required && edit->text().isEmpty()) {
                return false;
            }
        }
    }

    return true;
}

void OptionsForm::on_run_clicked() {
    std::string cmd = command_preview_->text().toStdString();
    if (cmd.empty()) {
        return;
    }
    emit run_requested(QString::fromStdString(cmd));
}

core::cached_command_config OptionsForm::get_current_config() const {
    core::cached_command_config config;
    config.tool = tool_.meta.cmd;
    if (subcommand_combo_) {
        config.subcommand = subcommand_combo_->currentText().toStdString();
    }
    config.sudo = sudo_checkbox_ && sudo_checkbox_->isChecked();
    for (const auto& [id, widget] : widgets_) {
        config.options[id] = widget->get_value();
    }
    for (const auto& [id, edit] : positional_widgets_) {
        config.positionals[id] = edit->text().toStdString();
    }
    config.command_string = command_preview_->text().toStdString();
    return config;
}

void OptionsForm::apply_config(const core::cached_command_config& config) {
    if (subcommand_combo_ && config.subcommand.has_value()) {
        int idx = subcommand_combo_->findText(QString::fromStdString(*config.subcommand));
        if (idx >= 0 && idx != subcommand_combo_->currentIndex()) {
            subcommand_combo_->setCurrentIndex(idx);
        }
    }

    if (sudo_checkbox_) {
        sudo_checkbox_->setChecked(config.sudo);
    }

    for (const auto& [id, val] : config.options) {
        auto it = widgets_.find(id);
        if (it != widgets_.end()) {
            it->second->set_value(val);
        }
    }

    for (const auto& [id, val] : config.positionals) {
        auto it = positional_widgets_.find(id);
        if (it != positional_widgets_.end()) {
            it->second->setText(QString::fromStdString(val));
        }
    }

    update_command();
}

void OptionsForm::reset_defaults() {
    if (subcommand_combo_) {
        subcommand_combo_->setCurrentIndex(0);
    }

    if (sudo_checkbox_) {
        sudo_checkbox_->setChecked(false);
    }

    for (auto& [id, widget] : widgets_) {
        widget->reset_to_default();
    }

    const auto* positionals = &tool_.positional;
    if (subcommand_combo_ && !tool_.subcommands.empty()) {
        int idx = subcommand_combo_->currentIndex();
        if (idx >= 0 && idx < static_cast<int>(tool_.subcommands.size())) {
            positionals = &tool_.subcommands[idx].positional;
        }
    }

    for (auto& [id, edit] : positional_widgets_) {
        std::string def_val = "";
        for (const auto& pos : *positionals) {
            if (pos.id == id && pos.default_value.has_value()) {
                def_val = pos.default_value.value();
                break;
            }
        }
        edit->setText(QString::fromStdString(def_val));
    }

    update_command();
}
