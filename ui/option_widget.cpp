#include "option_widget.hpp"
#include <QFileDialog>
#include <QStyle>

OptionWidget::OptionWidget(const core::option_def& opt, QWidget* parent)
    : QWidget(parent), opt_(opt) {

    switch (opt.type) {
        case core::option_type::BOOL:
            setup_bool();
            break;
        case core::option_type::STRING:
            setup_string();
            break;
        case core::option_type::INT:
            setup_int();
            break;
        case core::option_type::FLOAT:
            setup_float();
            break;
        case core::option_type::ENUM:
            setup_enum();
            break;
        case core::option_type::FILE_PATH:
            setup_file_path();
            break;
        case core::option_type::DIR_PATH:
            setup_dir_path();
            break;
        case core::option_type::STRING_LIST:
            setup_string_list();
            break;
    }
}

void OptionWidget::setup_bool() {
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    checkbox_ = new QCheckBox(this);
    checkbox_->setToolTip(QString::fromStdString(opt_.description));
    if (opt_.default_value.has_value() && opt_.default_value.value() == "true") {
        checkbox_->setChecked(true);
    }
    layout->addWidget(checkbox_);
    layout->addStretch();
    connect(checkbox_, &QCheckBox::toggled, this, &OptionWidget::value_changed);
}

void OptionWidget::setup_string() {
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    line_edit_ = new QLineEdit(this);
    line_edit_->setToolTip(QString::fromStdString(opt_.description));
    if (opt_.placeholder.has_value()) {
        line_edit_->setPlaceholderText(QString::fromStdString(opt_.placeholder.value()));
    }
    if (opt_.default_value.has_value()) {
        line_edit_->setText(QString::fromStdString(opt_.default_value.value()));
    }
    layout->addWidget(line_edit_);
    connect(line_edit_, &QLineEdit::textChanged, this, &OptionWidget::value_changed);
}

void OptionWidget::setup_int() {
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    spin_box_ = new QSpinBox(this);
    spin_box_->setToolTip(QString::fromStdString(opt_.description));
    if (opt_.min.has_value()) {
        spin_box_->setMinimum(static_cast<int>(opt_.min.value()));
    } else {
        spin_box_->setMinimum(0);
    }
    if (opt_.max.has_value()) {
        spin_box_->setMaximum(static_cast<int>(opt_.max.value()));
    } else {
        spin_box_->setMaximum(999999);
    }
    if (opt_.default_value.has_value()) {
        spin_box_->setValue(std::stoi(opt_.default_value.value()));
    }
    layout->addWidget(spin_box_);
    layout->addStretch();
    connect(spin_box_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &OptionWidget::value_changed);
}

void OptionWidget::setup_float() {
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    double_spin_box_ = new QDoubleSpinBox(this);
    double_spin_box_->setToolTip(QString::fromStdString(opt_.description));
    double_spin_box_->setDecimals(3);
    if (opt_.min.has_value()) {
        double_spin_box_->setMinimum(opt_.min.value());
    } else {
        double_spin_box_->setMinimum(0.0);
    }
    if (opt_.max.has_value()) {
        double_spin_box_->setMaximum(opt_.max.value());
    } else {
        double_spin_box_->setMaximum(999999.0);
    }
    if (opt_.default_value.has_value()) {
        double_spin_box_->setValue(std::stod(opt_.default_value.value()));
    }
    layout->addWidget(double_spin_box_);
    layout->addStretch();
    connect(double_spin_box_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &OptionWidget::value_changed);
}

void OptionWidget::setup_enum() {
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    combo_box_ = new QComboBox(this);
    combo_box_->setToolTip(QString::fromStdString(opt_.description));
    combo_box_->addItem("");
    for (const auto& choice : opt_.choices) {
        combo_box_->addItem(QString::fromStdString(choice));
    }
    if (opt_.default_value.has_value()) {
        int idx = combo_box_->findText(QString::fromStdString(opt_.default_value.value()));
        if (idx >= 0) {
            combo_box_->setCurrentIndex(idx);
        }
    }
    layout->addWidget(combo_box_);
    layout->addStretch();
    connect(combo_box_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &OptionWidget::value_changed);
}

void OptionWidget::setup_file_path() {
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    line_edit_ = new QLineEdit(this);
    line_edit_->setToolTip(QString::fromStdString(opt_.description));
    if (opt_.placeholder.has_value()) {
        line_edit_->setPlaceholderText(QString::fromStdString(opt_.placeholder.value()));
    }
    if (opt_.default_value.has_value()) {
        line_edit_->setText(QString::fromStdString(opt_.default_value.value()));
    }
    browse_button_ = new QPushButton(this);
    browse_button_->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
    browse_button_->setToolTip("Browse for file...");
    browse_button_->setFixedWidth(32);
    layout->addWidget(line_edit_);
    layout->addWidget(browse_button_);
    connect(line_edit_, &QLineEdit::textChanged, this, &OptionWidget::value_changed);
    connect(browse_button_, &QPushButton::clicked, this, &OptionWidget::on_browse_file);
}

void OptionWidget::setup_dir_path() {
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    line_edit_ = new QLineEdit(this);
    line_edit_->setToolTip(QString::fromStdString(opt_.description));
    if (opt_.placeholder.has_value()) {
        line_edit_->setPlaceholderText(QString::fromStdString(opt_.placeholder.value()));
    }
    if (opt_.default_value.has_value()) {
        line_edit_->setText(QString::fromStdString(opt_.default_value.value()));
    }
    browse_button_ = new QPushButton(this);
    browse_button_->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));
    browse_button_->setToolTip("Browse for directory...");
    browse_button_->setFixedWidth(32);
    layout->addWidget(line_edit_);
    layout->addWidget(browse_button_);
    connect(line_edit_, &QLineEdit::textChanged, this, &OptionWidget::value_changed);
    connect(browse_button_, &QPushButton::clicked, this, &OptionWidget::on_browse_dir);
}

void OptionWidget::setup_string_list() {
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    line_edit_ = new QLineEdit(this);
    line_edit_->setToolTip(QString::fromStdString(opt_.description));
    line_edit_->setPlaceholderText("comma-separated values");
    if (opt_.default_value.has_value()) {
        line_edit_->setText(QString::fromStdString(opt_.default_value.value()));
    }
    layout->addWidget(line_edit_);
    connect(line_edit_, &QLineEdit::textChanged, this, &OptionWidget::value_changed);
}

std::string OptionWidget::get_value() const {
    if (checkbox_) {
        return checkbox_->isChecked() ? "true" : "false";
    }
    if (line_edit_) {
        return line_edit_->text().toStdString();
    }
    if (spin_box_) {
        return std::to_string(spin_box_->value());
    }
    if (double_spin_box_) {
        return std::to_string(double_spin_box_->value());
    }
    if (combo_box_) {
        return combo_box_->currentText().toStdString();
    }
    return "";
}

void OptionWidget::set_enabled(bool enabled) {
    setEnabled(enabled);
}

const core::option_def& OptionWidget::option() const {
    return opt_;
}

void OptionWidget::on_browse_file() {
    QString path = QFileDialog::getOpenFileName(this, "Select File");
    if (!path.isEmpty() && line_edit_) {
        line_edit_->setText(path);
    }
}

void OptionWidget::on_browse_dir() {
    QString path = QFileDialog::getExistingDirectory(this, "Select Directory");
    if (!path.isEmpty() && line_edit_) {
        line_edit_->setText(path);
    }
}
