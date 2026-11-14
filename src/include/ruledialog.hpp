#pragma once

#include <QDialog>
#include <QSet>

#include "CJsonObject.hpp"

class QSpinBox;
class QCheckBox;
class QGroupBox;

// 规则编辑对话框。
// 内部用控件收集字段，getRule() 直接返回符合 core::Rule 期望的 JSON：
// { "id":3, "L":[{...}], "C":[{...}], "diffGender":true }
class RuleDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RuleDialog(QWidget* parent = nullptr, int defaultRuleId = 1);

    // 用于在确定时提示“id 不在学生列表中”的情况（不阻止保存，交给 Main 校验）
    void setValidStudentIds(const QSet<int>& ids);

    // 回填已有规则（编辑模式）；只读取存在且类型正确的字段
    void setRule(const neb::CJsonObject& rule);

    // 按当前控件状态拼装规则 JSON
    neb::CJsonObject getRule() const;

private slots:
    void updateEnabled();
    void onAccept();

private:
    QSpinBox* m_id = nullptr;

    QCheckBox* m_lEnable = nullptr;
    QGroupBox* m_lGroup = nullptr;
    QCheckBox* m_lInverse = nullptr;
    QCheckBox* m_allColumns = nullptr;
    QSpinBox* m_colMin = nullptr;
    QSpinBox* m_colMax = nullptr;
    QCheckBox* m_allRows = nullptr;
    QSpinBox* m_rowMin = nullptr;
    QSpinBox* m_rowMax = nullptr;

    QCheckBox* m_cEnable = nullptr;
    QGroupBox* m_cGroup = nullptr;
    QCheckBox* m_cInverse = nullptr;
    QSpinBox* m_centerId = nullptr;
    QSpinBox* m_radius = nullptr;

    QCheckBox* m_diffGender = nullptr;

    QSet<int> m_validIds;
    bool m_validateIds = false;
};
