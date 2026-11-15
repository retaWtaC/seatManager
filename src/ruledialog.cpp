#include "ruledialog.hpp"

#include "core.hpp"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QSpinBox>
#include <QStringList>
#include <QVBoxLayout>
#include <QWidget>

namespace {

// 把两个 QSpinBox 排成 “min ~ max”
QWidget* makeRangeRow(QSpinBox* minBox, QSpinBox* maxBox, QWidget* parent)
{
    auto* widget = new QWidget(parent);
    auto* layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(minBox);
    layout->addWidget(new QLabel(QStringLiteral("~"), widget));
    layout->addWidget(maxBox);
    return widget;
}

} // namespace

RuleDialog::RuleDialog(QWidget* parent, int defaultRuleId)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("添加规则"));
    setModal(true);
    resize(460, 560);

    auto* layout = new QVBoxLayout(this);

    // ---- 规则所属学生 id ----
    auto* idForm = new QFormLayout;
    m_id = new QSpinBox(this);
    m_id->setRange(1, 999999);
    m_id->setValue(defaultRuleId > 0 ? defaultRuleId : 1);
    idForm->addRow(QStringLiteral("规则 id（学生 id）："), m_id);
    layout->addLayout(idForm);

    // ---- L：行列范围 ----
    m_lEnable = new QCheckBox(QStringLiteral("启用 L（行列范围）"), this);
    layout->addWidget(m_lEnable);

    m_lGroup = new QGroupBox(QStringLiteral("L 参数"), this);
    auto* lForm = new QFormLayout(m_lGroup);

    m_lInverse = new QCheckBox(QStringLiteral("反选（范围外）"), m_lGroup);
    lForm->addRow(m_lInverse);

    m_allColumns = new QCheckBox(QStringLiteral("全列（不输出 columnRange）"), m_lGroup);
    lForm->addRow(m_allColumns);

    m_colMin = new QSpinBox(m_lGroup);
    m_colMin->setRange(1, k_columnNum);
    m_colMin->setValue(1);
    m_colMax = new QSpinBox(m_lGroup);
    m_colMax->setRange(1, k_columnNum);
    m_colMax->setValue(k_columnNum);
    lForm->addRow(QStringLiteral("列范围："), makeRangeRow(m_colMin, m_colMax, m_lGroup));

    m_allRows = new QCheckBox(QStringLiteral("全行（不输出 rowRange）"), m_lGroup);
    lForm->addRow(m_allRows);

    m_rowMin = new QSpinBox(m_lGroup);
    m_rowMin->setRange(1, k_rowNum);
    m_rowMin->setValue(1);
    m_rowMax = new QSpinBox(m_lGroup);
    m_rowMax->setRange(1, k_rowNum);
    m_rowMax->setValue(k_rowNum);
    lForm->addRow(QStringLiteral("行范围："), makeRangeRow(m_rowMin, m_rowMax, m_lGroup));

    layout->addWidget(m_lGroup);

    // ---- C：圆形范围 ----
    m_cEnable = new QCheckBox(QStringLiteral("启用 C（圆形范围）"), this);
    layout->addWidget(m_cEnable);

    m_cGroup = new QGroupBox(QStringLiteral("C 参数"), this);
    auto* cForm = new QFormLayout(m_cGroup);

    m_cInverse = new QCheckBox(QStringLiteral("反选（圆外）"), m_cGroup);
    cForm->addRow(m_cInverse);

    m_centerId = new QSpinBox(m_cGroup);
    m_centerId->setRange(1, 999999);
    m_centerId->setValue(defaultRuleId > 0 ? defaultRuleId : 1);
    cForm->addRow(QStringLiteral("圆心学生 id："), m_centerId);

    m_radius = new QSpinBox(m_cGroup);
    m_radius->setRange(0, 64);
    m_radius->setValue(1);
    cForm->addRow(QStringLiteral("半径："), m_radius);

    layout->addWidget(m_cGroup);

    // ---- 其他 ----
    m_diffGender = new QCheckBox(QStringLiteral("允许异性（diffGender）"), this);
    layout->addWidget(m_diffGender);

    layout->addStretch(1);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &RuleDialog::onAccept);
    connect(buttons, &QDialogButtonBox::rejected, this, &RuleDialog::reject);
    layout->addWidget(buttons);

    // 联动：未启用时子控件置灰；勾“全列/全行”时对应范围置灰
    connect(m_lEnable, &QCheckBox::toggled, this, &RuleDialog::updateEnabled);
    connect(m_cEnable, &QCheckBox::toggled, this, &RuleDialog::updateEnabled);
    connect(m_allColumns, &QCheckBox::toggled, this, &RuleDialog::updateEnabled);
    connect(m_allRows, &QCheckBox::toggled, this, &RuleDialog::updateEnabled);

    updateEnabled();
}

void RuleDialog::setValidStudentIds(const QSet<int>& ids)
{
    m_validIds = ids;
    m_validateIds = true;
}

void RuleDialog::setRule(const neb::CJsonObject& rule)
{
    setWindowTitle(QStringLiteral("编辑规则"));

    // 先恢复默认状态
    m_lEnable->setChecked(false);
    m_cEnable->setChecked(false);
    m_lInverse->setChecked(false);
    m_cInverse->setChecked(false);
    m_allColumns->setChecked(true);
    m_allRows->setChecked(true);
    m_colMin->setValue(1);
    m_colMax->setValue(k_columnNum);
    m_rowMin->setValue(1);
    m_rowMax->setValue(k_rowNum);
    m_centerId->setValue(1);
    m_radius->setValue(1);
    m_diffGender->setChecked(false);

    // 拷贝一份，方便使用非 const 的 operator[] 读取子对象
    neb::CJsonObject r(rule);

    int id = 1;
    if (r.Get(keyId, id)) {
        m_id->setValue(id);
    }

    // 回填 L
    if (r.KeyExist(keyL) && r[keyL].GetArraySize() > 0) {
        m_lEnable->setChecked(true);
        neb::CJsonObject& l = r[keyL][0];

        bool inverse = false;
        if (l.Get(keyIsInverse, inverse)) {
            m_lInverse->setChecked(inverse);
        }

        m_allColumns->setChecked(!l.KeyExist(keyColumn));
        if (l.KeyExist(keyColumn)) {
            int minValue = 1;
            int maxValue = k_columnNum;
            l[keyColumn].Get(0, minValue);
            l[keyColumn].Get(1, maxValue);
            m_colMin->setValue(minValue);
            m_colMax->setValue(maxValue);
        }

        m_allRows->setChecked(!l.KeyExist(keyRow));
        if (l.KeyExist(keyRow)) {
            int minValue = 1;
            int maxValue = k_rowNum;
            l[keyRow].Get(0, minValue);
            l[keyRow].Get(1, maxValue);
            m_rowMin->setValue(minValue);
            m_rowMax->setValue(maxValue);
        }
    }

    // 回填 C
    if (r.KeyExist(keyC) && r[keyC].GetArraySize() > 0) {
        m_cEnable->setChecked(true);
        neb::CJsonObject& c = r[keyC][0];

        bool inverse = false;
        if (c.Get(keyIsInverse, inverse)) {
            m_cInverse->setChecked(inverse);
        }

        if (c.KeyExist(keyCircle)) {
            int centerId = 1;
            int radius = 0;
            c[keyCircle].Get(0, centerId);
            c[keyCircle].Get(1, radius);
            m_centerId->setValue(centerId);
            m_radius->setValue(radius);
        }
    }

    // 回填 diffGender
    if (r.KeyExist(keyDiffGender)) {
        bool allow = false;
        if (r.Get(keyDiffGender, allow)) {
            m_diffGender->setChecked(allow);
        }
    }

    updateEnabled();
}

neb::CJsonObject RuleDialog::getRule() const
{
    neb::CJsonObject rule;
    rule.Add(keyId, m_id->value());

    // L 可选：未勾选则完全不输出 "L" 键
    if (m_lEnable->isChecked()) {
        neb::CJsonObject l;
        l.Add(keyIsInverse, m_lInverse->isChecked(), true);

        // “全列/全行”时不输出对应范围键，core::Rule::setL 会使用默认 1..k_columnNum / 1..k_rowNum
        if (!m_allColumns->isChecked()) {
            neb::CJsonObject columnRange;
            columnRange.Add(m_colMin->value());
            columnRange.Add(m_colMax->value());
            l.Add(keyColumn, columnRange);
        }
        if (!m_allRows->isChecked()) {
            neb::CJsonObject rowRange;
            rowRange.Add(m_rowMin->value());
            rowRange.Add(m_rowMax->value());
            l.Add(keyRow, rowRange);
        }

        neb::CJsonObject lArray;
        lArray.Add(l);
        rule.Add(keyL, lArray);
    }

    // C 可选：circle[0] 是圆心学生 id（不是坐标），circle[1] 是半径
    if (m_cEnable->isChecked()) {
        neb::CJsonObject c;
        c.Add(keyIsInverse, m_cInverse->isChecked(), true);

        neb::CJsonObject circle;
        circle.Add(m_centerId->value());
        circle.Add(m_radius->value());
        c.Add(keyCircle, circle);

        neb::CJsonObject cArray;
        cArray.Add(c);
        rule.Add(keyC, cArray);
    }

    // diffGender 仅在勾选时输出 true
    if (m_diffGender->isChecked()) {
        rule.Add(keyDiffGender, true, true);
    }

    return rule;
}

void RuleDialog::updateEnabled()
{
    m_lGroup->setEnabled(m_lEnable->isChecked());
    m_cGroup->setEnabled(m_cEnable->isChecked());

    m_colMin->setEnabled(!m_allColumns->isChecked());
    m_colMax->setEnabled(!m_allColumns->isChecked());
    m_rowMin->setEnabled(!m_allRows->isChecked());
    m_rowMax->setEnabled(!m_allRows->isChecked());
}

void RuleDialog::onAccept()
{
    // core::Rule::check 要求 L、C 至少存在一个
    if (!m_lEnable->isChecked() && !m_cEnable->isChecked()) {
        QMessageBox::warning(this, QStringLiteral("规则无效"),
                             QStringLiteral("L 和 C 至少需要启用一个，否则规则无法通过校验。"));
        return;
    }

    if (m_lEnable->isChecked()) {
        if (!m_allColumns->isChecked() && m_colMin->value() > m_colMax->value()) {
            QMessageBox::warning(this, QStringLiteral("规则无效"),
                                 QStringLiteral("列范围的最小值不能大于最大值。"));
            return;
        }
        if (!m_allRows->isChecked() && m_rowMin->value() > m_rowMax->value()) {
            QMessageBox::warning(this, QStringLiteral("规则无效"),
                                 QStringLiteral("行范围的最小值不能大于最大值。"));
            return;
        }
    }

    // 学生 id 不存在时给出提示，但仍允许保存；SeatManager::main 会再次校验
    if (m_validateIds) {
        QStringList warnings;
        if (!m_validIds.contains(m_id->value())) {
            warnings << QStringLiteral("规则 id=%1 不在当前学生列表中。").arg(m_id->value());
        }
        if (m_cEnable->isChecked() && !m_validIds.contains(m_centerId->value())) {
            warnings << QStringLiteral("圆心学生 id=%1 不在当前学生列表中。").arg(m_centerId->value());
        }
        if (!warnings.isEmpty()) {
            QMessageBox::warning(this, QStringLiteral("提示"),
                                 warnings.join(QLatin1Char('\n'))
                                     + QStringLiteral("\n\n执行排座时该规则可能导致失败。"));
        }
    }

    accept();
}
