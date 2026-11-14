#pragma once

#include <QDialog>
#include <QString>

class QSpinBox;
class QComboBox;
class QLineEdit;

// 学生添加/编辑对话框。
// 添加时 id 可编辑；编辑时由主窗口调用 setIdReadOnly(true) 锁定 id。
class StudentDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StudentDialog(QWidget* parent = nullptr);

    // 回填已有学生数据（编辑模式）
    void setStudent(int id, int height, int gender, const QString& name);
    void setIdReadOnly(bool readOnly);

    int studentId() const;
    int height() const;
    int gender() const;
    QString name() const;

private slots:
    void onAccept();

private:
    QSpinBox* m_id = nullptr;
    QSpinBox* m_height = nullptr;
    QComboBox* m_gender = nullptr;
    QLineEdit* m_name = nullptr;
};
