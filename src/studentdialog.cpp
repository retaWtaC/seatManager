#include "studentdialog.hpp"

#include <QAbstractSpinBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QSpinBox>
#include <QVBoxLayout>

StudentDialog::StudentDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("添加学生"));
    setModal(true);
    resize(360, 220);

    auto* layout = new QVBoxLayout(this);
    auto* form = new QFormLayout;

    m_id = new QSpinBox(this);
    m_id->setRange(1, 999999);          // id 必须为正，且给一个合理上限
    m_id->setValue(1);

    m_height = new QSpinBox(this);
    m_height->setRange(0, 300);         // 身高按厘米
    m_height->setValue(170);

    m_gender = new QComboBox(this);
    m_gender->addItem(QStringLiteral("男"), 0);
    m_gender->addItem(QStringLiteral("女"), 1);

    m_name = new QLineEdit(this);
    m_name->setMaxLength(32);
    m_name->setPlaceholderText(QStringLiteral("学生姓名"));

    form->addRow(QStringLiteral("id："), m_id);
    form->addRow(QStringLiteral("姓名："), m_name);
    form->addRow(QStringLiteral("身高(cm)："), m_height);
    form->addRow(QStringLiteral("性别："), m_gender);
    layout->addLayout(form);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &StudentDialog::onAccept);
    connect(buttons, &QDialogButtonBox::rejected, this, &StudentDialog::reject);
    layout->addWidget(buttons);
}

void StudentDialog::setStudent(int id, int height, int gender, const QString& name)
{
    m_id->setValue(id);
    m_height->setValue(height);
    const int genderIndex = m_gender->findData(gender);
    m_gender->setCurrentIndex(genderIndex >= 0 ? genderIndex : 0);
    m_name->setText(name);
}

void StudentDialog::setIdReadOnly(bool readOnly)
{
    // 编辑模式：id 是主键，不允许修改（禁用以同时挡住鼠标滚轮/键盘）
    m_id->setReadOnly(readOnly);
    m_id->setButtonSymbols(readOnly ? QAbstractSpinBox::NoButtons : QAbstractSpinBox::UpDownArrows);
    m_id->setEnabled(!readOnly);
    setWindowTitle(readOnly ? QStringLiteral("编辑学生") : QStringLiteral("添加学生"));
    if (readOnly) {
        m_name->setFocus();
    }
}

int StudentDialog::studentId() const
{
    return m_id->value();
}

int StudentDialog::height() const
{
    return m_height->value();
}

int StudentDialog::gender() const
{
    return m_gender->currentData().toInt();
}

QString StudentDialog::name() const
{
    return m_name->text().trimmed();
}

void StudentDialog::onAccept()
{
    // 非 0、id 范围由 QSpinBox 保证；这里只做姓名的空值校验
    if (m_name->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("输入有误"), QStringLiteral("姓名不能为空。"));
        m_name->setFocus();
        return;
    }
    accept();
}
