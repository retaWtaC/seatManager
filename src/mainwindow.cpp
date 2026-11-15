#include "mainwindow.hpp"

#include "core.hpp"
#include "ruledialog.hpp"
#include "seattable.hpp"
#include "studentdialog.hpp"

#include <QAction>
#include <QByteArray>
#include <QColor>
#include <QDialog>
#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPoint>
#include <QScrollBar>
#include <QSet>
#include <QSplitter>
#include <QStatusBar>
#include <QStringList>
#include <QTableWidgetItem>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>

#include <algorithm>
#include <iostream>
#include <memory>
#include <sstream>
#include <streambuf>
#include <string>
#include <utility>
#include <vector>

namespace {

// 用 RAII 捕获 std::cout，保证异常时也能恢复原来的缓冲区
class CoutRedirectGuard
{
public:
    explicit CoutRedirectGuard(std::ostringstream& sink)
        : m_old(std::cout.rdbuf(sink.rdbuf()))
    {
    }

    ~CoutRedirectGuard()
    {
        std::cout.rdbuf(m_old);
    }

private:
    std::streambuf* m_old = nullptr;
};

} // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    // 规则根对象先建立 { "rules": [] }
    m_rulesObj.AddEmptySubArray(keyRules);
    buildUi();
    log(QStringLiteral("就绪：添加/加载学生与规则后，点击“执行排座”。"));
}

MainWindow::~MainWindow() = default;

void MainWindow::buildUi()
{
    setWindowTitle(QStringLiteral("排座系统"));
    resize(1100, 720);

    // ---- 工具栏 ----
    auto* tb = addToolBar(QStringLiteral("操作"));
    tb->setMovable(false);

    auto* newStudentsAct = tb->addAction(QStringLiteral("新建学生"));
    connect(newStudentsAct, &QAction::triggered, this, &MainWindow::onNewStudents);

    auto* newRulesAct = tb->addAction(QStringLiteral("新建规则"));
    connect(newRulesAct, &QAction::triggered, this, &MainWindow::onNewRules);

    auto* loadStudentsAct = tb->addAction(QStringLiteral("加载学生"));
    connect(loadStudentsAct, &QAction::triggered, this, &MainWindow::onLoadStudents);

    auto* loadRulesAct = tb->addAction(QStringLiteral("加载规则"));
    connect(loadRulesAct, &QAction::triggered, this, &MainWindow::onLoadRules);

    auto* saveStudentsAct = tb->addAction(QStringLiteral("保存学生"));
    connect(saveStudentsAct, &QAction::triggered, this, &MainWindow::onSaveStudents);

    auto* saveRulesAct = tb->addAction(QStringLiteral("保存规则"));
    connect(saveRulesAct, &QAction::triggered, this, &MainWindow::onSaveRules);

    tb->addSeparator();

    auto* addStudentAct = tb->addAction(QStringLiteral("添加学生"));
    connect(addStudentAct, &QAction::triggered, this, &MainWindow::onAddStudent);

    auto* addRuleAct = tb->addAction(QStringLiteral("添加规则"));
    connect(addRuleAct, &QAction::triggered, this, &MainWindow::onAddRule);

    tb->addSeparator();

    auto* runAct = tb->addAction(QStringLiteral("执行排座"));
    connect(runAct, &QAction::triggered, this, &MainWindow::onRun);

    m_showIdAction = tb->addAction(QStringLiteral("显示 id"));
    m_showIdAction->setCheckable(true);
    m_showIdAction->setChecked(true);
    connect(m_showIdAction, &QAction::toggled, this, &MainWindow::onToggleShowId);

    // ---- 中央布局：左座位表，右规则列表 + 日志 ----
    auto* central = new QWidget(this);
    auto* mainLayout = new QHBoxLayout(central);
    mainLayout->setContentsMargins(6, 6, 6, 6);

    auto* splitter = new QSplitter(Qt::Horizontal, central);

    auto* left = new QWidget(splitter);
    auto* leftLayout = new QVBoxLayout(left);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->addWidget(new QLabel(QStringLiteral("座位表（拖拽交换、双击编辑、右键菜单）"), left));

    m_table = new SeatTable(k_rowNum, k_columnNum, left);
    m_table->setHorizontalHeaderLabels(
        QStringList() << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8");
    QStringList rowLabels;
    for (int r = 0; r < k_rowNum; ++r) {
        rowLabels << QString::number(r + 1);
    }
    m_table->setVerticalHeaderLabels(rowLabels);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setContextMenuPolicy(Qt::CustomContextMenu);
    m_table->setMinimumWidth(560);

    connect(m_table, &QTableWidget::cellClicked, this, &MainWindow::onCellClicked);
    connect(m_table, &QTableWidget::cellDoubleClicked, this, &MainWindow::onCellDoubleClicked);
    connect(m_table, &QWidget::customContextMenuRequested, this, &MainWindow::onTableContextMenu);
    connect(m_table, &SeatTable::swapRequested, this, &MainWindow::onDragDrop);

    leftLayout->addWidget(m_table, 1);
    splitter->addWidget(left);

    auto* right = new QWidget(splitter);
    auto* rightLayout = new QVBoxLayout(right);
    rightLayout->setContentsMargins(0, 0, 0, 0);

    rightLayout->addWidget(new QLabel(QStringLiteral("规则列表（双击编辑、右键菜单）"), right));
    m_ruleList = new QListWidget(right);
    m_ruleList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_ruleList, &QListWidget::itemDoubleClicked, this, &MainWindow::onRuleDoubleClicked);
    connect(m_ruleList, &QWidget::customContextMenuRequested, this, &MainWindow::onRuleContextMenu);
    rightLayout->addWidget(m_ruleList, 1);

    rightLayout->addWidget(new QLabel(QStringLiteral("日志"), right));
    m_log = new QPlainTextEdit(right);
    m_log->setReadOnly(true);
    rightLayout->addWidget(m_log, 1);

    splitter->addWidget(right);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 2);

    mainLayout->addWidget(splitter);
    setCentralWidget(central);

    // ---- 状态栏 ----
    m_status = new QLabel(this);
    statusBar()->addWidget(m_status);

    clearBoard();
    renderBoard();
    updateStatus();
}

void MainWindow::clearBoard()
{
    // m_cells 全部置为默认（id=0 表示空位），并重建所有单元格项。
    // 这里显式打开拖拽/放置标志，保证 SeatTable 能收到单元格拖拽。
    m_cells.assign(k_rowNum, std::vector<CellInfo>(k_columnNum));

    for (int r = 0; r < k_rowNum; ++r) {
        for (int c = 0; c < k_columnNum; ++c) {
            auto* item = new QTableWidgetItem;
            item->setTextAlignment(Qt::AlignCenter);
            item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable
                           | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
            m_table->setItem(r, c, item);
        }
    }
}

void MainWindow::renderBoard()
{
    // 尺寸兜底，避免外部数据异常导致越界
    if (static_cast<int>(m_cells.size()) != k_rowNum) {
        m_cells.assign(k_rowNum, std::vector<CellInfo>(k_columnNum));
    }

    // 以 m_students 为准清理 m_cells：
    // 1. 单元格里的 id 已不存在 -> 清空；
    // 2. 同一个 id 重复出现 -> 只保留第一次出现的位置；
    // 3. 姓名/性别以 m_students 为准刷新。
    QSet<int> seen;
    for (int r = 0; r < k_rowNum; ++r) {
        if (static_cast<int>(m_cells[r].size()) != k_columnNum) {
            m_cells[r].resize(k_columnNum);
        }
        for (int c = 0; c < k_columnNum; ++c) {
            CellInfo& cell = m_cells[r][c];

            if (cell.id > 0) {
                const int studentIndex = findStudentIndex(cell.id);
                if (studentIndex < 0 || seen.contains(cell.id)) {
                    cell = CellInfo {};
                } else {
                    cell.name = m_students[studentIndex].name;
                    cell.gender = m_students[studentIndex].gender;
                    seen.insert(cell.id);
                }
            } else {
                cell.id = 0;
                cell.name.clear();
                cell.gender = k_defaultGender;
            }

            auto* item = m_table->item(r, c);
            if (!item) {
                item = new QTableWidgetItem;
                item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable
                               | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
                m_table->setItem(r, c, item);
            }
            item->setTextAlignment(Qt::AlignCenter);

            if (cell.id > 0) {
                QString text = cell.name;
                if (m_showId) {
                    text += QStringLiteral("\n(id=%1)").arg(cell.id);
                }
                item->setText(text);

                // 性别着色：男蓝、女粉、未知灰
                if (cell.gender == 0) {
                    item->setBackground(QColor(220, 235, 255));
                } else if (cell.gender == 1) {
                    item->setBackground(QColor(255, 225, 235));
                } else {
                    item->setBackground(QColor(240, 240, 240));
                }

                item->setToolTip(QStringLiteral("第 %1 行 第 %2 列\nid=%3\n姓名：%4\n性别：%5")
                                     .arg(r + 1)
                                     .arg(c + 1)
                                     .arg(cell.id)
                                     .arg(cell.name, genderText(cell.gender)));
            } else {
                item->setText(QStringLiteral("—"));
                item->setBackground(QColor(245, 245, 245));
                item->setToolTip(QStringLiteral("第 %1 行 第 %2 列\n空位").arg(r + 1).arg(c + 1));
            }
        }
    }
}

void MainWindow::refreshRuleList(int selectIndex)
{
    m_ruleList->clear();

    if (!m_rulesObj.KeyExist(keyRules)) {
        return;
    }

    neb::CJsonObject& rules = m_rulesObj[keyRules];
    const int count = rules.GetArraySize();
    for (int i = 0; i < count; ++i) {
        neb::CJsonObject& rule = rules[i];

        int id = 0;
        const bool hasId = rule.Get(keyId, id);
        QString text = QStringLiteral("#%1  id=%2")
                           .arg(i)
                           .arg(hasId ? QString::number(id) : QStringLiteral("?"));
        if (rule.KeyExist(keyL)) {
            text += QStringLiteral("  [L]");
        }
        if (rule.KeyExist(keyC)) {
            text += QStringLiteral("  [C]");
        }
        bool allowDiffGender = false;
        if (rule.KeyExist(keyDiffGender) && rule.Get(keyDiffGender, allowDiffGender)
            && allowDiffGender) {
            text += QStringLiteral("  [允许异性]");
        }

        auto* item = new QListWidgetItem(text, m_ruleList);
        item->setData(Qt::UserRole, i);       // 存规则在 rules 数组中的下标
        item->setToolTip(QString::fromStdString(rule.ToFormattedString()));
    }

    if (selectIndex >= 0 && selectIndex < count) {
        m_ruleList->setCurrentRow(selectIndex);
    }
}

void MainWindow::updateStatus()
{
    if (!m_status) {
        return;
    }

    m_status->setText(QStringLiteral("学生 %1 人 | 规则 %2 条 | %3")
                          .arg(static_cast<int>(m_students.size()))
                          .arg(ruleCount())
                          .arg(m_manualModified ? QStringLiteral("手动调整（执行排座将覆盖）")
                                                : QStringLiteral("未手动调整")));
}

void MainWindow::onNewStudents()
{
    m_students.clear();
    clearBoard();
    m_manualModified = false;
    renderBoard();
    updateStatus();

    statusBar()->showMessage(QStringLiteral("新建学生列表（空）"), 5000);
    log(QStringLiteral("新建学生列表（空）。"));
}

void MainWindow::onNewRules()
{
    m_rulesObj.Clear();
    m_rulesObj.AddEmptySubArray(keyRules);
    m_ruleList->clear();
    updateStatus();

    statusBar()->showMessage(QStringLiteral("新建规则列表（空）"), 5000);
    log(QStringLiteral("新建规则列表（空）。"));
}

void MainWindow::onAddStudent()
{
    if (static_cast<int>(m_students.size()) >= k_peopleNum) {
        showError(QStringLiteral("无法添加"), QStringLiteral("学生人数已达上限 64 人。"));
        return;
    }

    // 默认 id 取当前最大值 +1，减少连续添加时的手工输入
    int nextId = 1;
    for (const auto& student : m_students) {
        nextId = std::max(nextId, student.id + 1);
    }
    if (nextId > 999999) {
        // id 已经顶到上限，找第一个空位
        QSet<int> used;
        for (const auto& student : m_students) {
            used.insert(student.id);
        }
        nextId = 1;
        while (used.contains(nextId)) {
            ++nextId;
        }
    }

    StudentDialog dialog(this);
    dialog.setStudent(nextId, 170, 0, QString());
    while (dialog.exec() == QDialog::Accepted) {
        const int id = dialog.studentId();

        // 唯一性校验；重复时保留对话框内容，让用户直接改 id
        if (findStudentIndex(id) >= 0) {
            QMessageBox::warning(this, QStringLiteral("id 重复"),
                                 QStringLiteral("学生 id=%1 已存在，请修改。").arg(id));
            continue;
        }

        m_students.push_back(StudentRec {id, dialog.height(), dialog.gender(), dialog.name()});
        updateStatus();
        log(QStringLiteral("添加学生：id=%1，姓名=%2，身高=%3，性别=%4。")
                .arg(id)
                .arg(dialog.name())
                .arg(dialog.height())
                .arg(genderText(dialog.gender())));
        statusBar()->showMessage(QStringLiteral("已添加学生 id=%1").arg(id), 5000);
        break;
    }
}

void MainWindow::onLoadStudents()
{
    const QString path = QFileDialog::getOpenFileName(
        this, QStringLiteral("选择学生 JSON"), m_studentsPath,
        QStringLiteral("JSON (*.json);;所有文件 (*)"));
    if (path.isEmpty()) {
        return;     // 未选路径：静默取消
    }

    std::string content;
    if (!loadFileToString(path, content)) {
        showError(QStringLiteral("加载失败"), QStringLiteral("无法读取文件：") + path);
        return;
    }

    // 先解析到临时容器，全部成功后再替换，避免半加载状态
    neb::CJsonObject root(content);
    if (!root.GetErrMsg().empty()) {
        showError(QStringLiteral("加载失败"),
                  QStringLiteral("JSON 解析失败：") + QString::fromStdString(root.GetErrMsg()));
        return;
    }
    if (!root.KeyExist(keyStudents) || !root[keyStudents].IsArray()) {
        showError(QStringLiteral("加载失败"),
                  QStringLiteral("学生 JSON 缺少 \"students\" 数组。"));
        return;
    }

    neb::CJsonObject& array = root[keyStudents];
    const int count = array.GetArraySize();
    if (count > k_peopleNum) {
        showError(QStringLiteral("加载失败"),
                  QStringLiteral("学生人数 %1 超过上限 %2。").arg(count).arg(k_peopleNum));
        return;
    }

    std::vector<StudentRec> loaded;
    loaded.reserve(static_cast<size_t>(count));
    QSet<int> ids;

    for (int i = 0; i < count; ++i) {
        neb::CJsonObject& object = array[i];

        int id = 0;
        int height = 0;
        int gender = k_defaultGender;
        std::string name;

        if (!object.Get(keyId, id) || !object.Get(keyHeight, height)
            || !object.Get(keyGender, gender) || !object.Get(keyName, name)) {
            showError(QStringLiteral("加载失败"),
                      QStringLiteral("第 %1 条学生记录缺少字段或类型错误。").arg(i + 1));
            return;
        }
        if (id <= 0 || id > 999999) {
            showError(QStringLiteral("加载失败"),
                      QStringLiteral("第 %1 条学生 id=%2 非法（应在 1..999999）。").arg(i + 1).arg(id));
            return;
        }
        if (gender != 0 && gender != 1) {
            showError(QStringLiteral("加载失败"),
                      QStringLiteral("第 %1 条学生 id=%2 的性别值 %3 非法（只能是 0 或 1）。")
                          .arg(i + 1)
                          .arg(id)
                          .arg(gender));
            return;
        }
        if (ids.contains(id)) {
            showError(QStringLiteral("加载失败"),
                      QStringLiteral("学生 id=%1 重复。").arg(id));
            return;
        }

        ids.insert(id);
        loaded.push_back(StudentRec {id, height, gender, QString::fromStdString(name)});
    }

    m_students = std::move(loaded);
    m_studentsPath = path;

    // 加载后不自动排座：保持空座位，等用户点“执行排座”
    clearBoard();
    m_manualModified = false;
    renderBoard();
    updateStatus();

    log(QStringLiteral("已加载学生 JSON：%1（%2 人）。等待执行排座。")
            .arg(path)
            .arg(static_cast<int>(m_students.size())));
    statusBar()->showMessage(
        QStringLiteral("已加载学生 %1 人").arg(static_cast<int>(m_students.size())), 5000);
}

void MainWindow::onLoadRules()
{
    const QString path = QFileDialog::getOpenFileName(
        this, QStringLiteral("选择规则 JSON"), m_rulesPath,
        QStringLiteral("JSON (*.json);;所有文件 (*)"));
    if (path.isEmpty()) {
        return;
    }

    std::string content;
    if (!loadFileToString(path, content)) {
        showError(QStringLiteral("加载失败"), QStringLiteral("无法读取文件：") + path);
        return;
    }

    neb::CJsonObject root(content);
    if (!root.GetErrMsg().empty()) {
        showError(QStringLiteral("加载失败"),
                  QStringLiteral("JSON 解析失败：") + QString::fromStdString(root.GetErrMsg()));
        return;
    }
    if (!root.KeyExist(keyRules) || !root[keyRules].IsArray()) {
        showError(QStringLiteral("加载失败"),
                  QStringLiteral("规则 JSON 缺少 \"rules\" 数组。"));
        return;
    }

    m_rulesObj = std::move(root);
    m_rulesPath = path;

    refreshRuleList();
    updateStatus();

    log(QStringLiteral("已加载规则 JSON：%1（%2 条）。").arg(path).arg(ruleCount()));
    statusBar()->showMessage(QStringLiteral("已加载规则 %1 条").arg(ruleCount()), 5000);
}

void MainWindow::onSaveStudents()
{
    const QString defaultPath =
        m_studentsPath.isEmpty() ? QStringLiteral("students.json") : m_studentsPath;
    const QString path = QFileDialog::getSaveFileName(
        this, QStringLiteral("保存学生 JSON"), defaultPath,
        QStringLiteral("JSON (*.json);;所有文件 (*)"));
    if (path.isEmpty()) {
        return;     // 未选路径：静默取消
    }

    QString errorText;
    if (!saveStringToFile(path, buildStudentsJson(), errorText)) {
        showError(QStringLiteral("保存失败"), errorText);
        return;
    }

    m_studentsPath = path;
    log(QStringLiteral("学生已保存到：%1（%2 人，不含座位顺序）")
            .arg(path)
            .arg(static_cast<int>(m_students.size())));
    statusBar()->showMessage(QStringLiteral("学生已保存"), 5000);
}

void MainWindow::onSaveRules()
{
    if (!m_rulesObj.KeyExist(keyRules)) {
        m_rulesObj.AddEmptySubArray(keyRules);
    }

    const QString defaultPath =
        m_rulesPath.isEmpty() ? QStringLiteral("rules.json") : m_rulesPath;
    const QString path = QFileDialog::getSaveFileName(
        this, QStringLiteral("保存规则 JSON"), defaultPath,
        QStringLiteral("JSON (*.json);;所有文件 (*)"));
    if (path.isEmpty()) {
        return;
    }

    QString errorText;
    if (!saveStringToFile(path, m_rulesObj.ToFormattedString(), errorText)) {
        showError(QStringLiteral("保存失败"), errorText);
        return;
    }

    m_rulesPath = path;
    log(QStringLiteral("规则已保存到：%1（%2 条）").arg(path).arg(ruleCount()));
    statusBar()->showMessage(QStringLiteral("规则已保存"), 5000);
}

void MainWindow::onAddRule()
{
    if (!m_rulesObj.KeyExist(keyRules)) {
        m_rulesObj.AddEmptySubArray(keyRules);
    }

    QSet<int> validIds;
    for (const auto& student : m_students) {
        validIds.insert(student.id);
    }
    const int defaultId = m_students.empty() ? 1 : m_students.front().id;

    RuleDialog dialog(this, defaultId);
    dialog.setValidStudentIds(validIds);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    neb::CJsonObject rule = dialog.getRule();
    m_rulesObj[keyRules].Add(rule);
    const int index = ruleCount() - 1;

    refreshRuleList(index);
    updateStatus();

    int id = 0;
    rule.Get(keyId, id);
    log(QStringLiteral("添加规则 #%1：id=%2。").arg(index).arg(id));
    statusBar()->showMessage(QStringLiteral("已添加规则 #%1").arg(index), 5000);
}

void MainWindow::onRun()
{
    if (m_manualModified) {
        const auto answer = QMessageBox::question(
            this, QStringLiteral("确认排座"),
            QStringLiteral("手动调整将被排座结果覆盖，是否继续？"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (answer != QMessageBox::Yes) {
            log(QStringLiteral("已取消排座。"));
            return;
        }
    }

    if (static_cast<int>(m_cells.size()) != k_rowNum) {
        clearBoard();
    }

    log(QStringLiteral("开始执行排座..."));

    // SeatManager 只在执行排座时临时构造；捕获它（以及 Rule::execute）写到 std::cout 的内容
    std::ostringstream captured;
    std::unique_ptr<SeatManager> engine;
    bool ok = false;
    QString runError;

    {
        CoutRedirectGuard guard(captured);
        try {
            engine = std::make_unique<SeatManager>(buildStudentsJson(), m_rulesObj.ToFormattedString());
            ok = engine->main();
			engine->execute();
        } catch (const std::exception& e) {
            runError = QString::fromUtf8(e.what());
        } catch (...) {
            runError = QStringLiteral("未知异常");
        }
    }

    const QString capturedText = QString::fromStdString(captured.str()).trimmed();

    if (!runError.isEmpty()) {
        if (!capturedText.isEmpty()) {
            log(capturedText);
        }
        showError(QStringLiteral("执行异常"), runError);
        return;
    }

    if (!ok) {
        showError(QStringLiteral("排座失败"),
                  capturedText.isEmpty()
                      ? QStringLiteral("SeatManager::main() 返回 false，详见日志。")
                      : capturedText);
        return;
    }

    if (!capturedText.isEmpty()) {
        log(capturedText);
    }

    // 排座成功：把 board 结果写回 m_cells，姓名/性别按 id 从 m_students 查
    for (int r = 0; r < k_rowNum; ++r) {
        for (int c = 0; c < k_columnNum; ++c) {
            m_cells[r][c] = CellInfo {};

            const Point point {c + 1, r + 1};
            const int id = engine->getId(point);
            if (id > 0) {
                const int studentIndex = findStudentIndex(id);
                if (studentIndex >= 0) {
                    m_cells[r][c] = CellInfo {id,
                                              m_students[studentIndex].name,
                                              m_students[studentIndex].gender};
                }
            }
        }
    }

    m_manualModified = false;
    renderBoard();
    updateStatus();

    statusBar()->showMessage(QStringLiteral("排座完成"), 5000);
    log(QStringLiteral("排座完成。"));
}

void MainWindow::onCellClicked(int row, int col)
{
    if (row < 0 || row >= k_rowNum || col < 0 || col >= k_columnNum) {
        return;
    }

    const CellInfo& cell = m_cells[row][col];
    QString info;
    if (cell.id > 0) {
        info = QStringLiteral("第 %1 行 第 %2 列 | id=%3 | 姓名：%4 | 性别：%5")
                   .arg(row + 1)
                   .arg(col + 1)
                   .arg(cell.id)
                   .arg(cell.name, genderText(cell.gender));
    } else {
        info = QStringLiteral("第 %1 行 第 %2 列 | 空位").arg(row + 1).arg(col + 1);
    }

    statusBar()->showMessage(info, 5000);
}

void MainWindow::onCellDoubleClicked(int row, int col)
{
    editStudentAt(row, col);
}

void MainWindow::onTableContextMenu(const QPoint& pos)
{
    QTableWidgetItem* item = m_table->itemAt(pos);
    if (!item) {
        return;
    }

    const int row = item->row();
    const int col = item->column();
    m_table->setCurrentCell(row, col);

    QMenu menu(this);
    QAction* editAction = menu.addAction(QStringLiteral("编辑"));
    QAction* deleteAction = menu.addAction(QStringLiteral("删除"));
    QAction* selected = menu.exec(m_table->viewport()->mapToGlobal(pos));

    if (selected == editAction) {
        editStudentAt(row, col);
    } else if (selected == deleteAction) {
        deleteStudentAt(row, col);
    }
}

void MainWindow::onDragDrop(int fromRow, int fromCol, int toRow, int toCol)
{
    if (fromRow < 0 || fromRow >= k_rowNum || fromCol < 0 || fromCol >= k_columnNum
        || toRow < 0 || toRow >= k_rowNum || toCol < 0 || toCol >= k_columnNum) {
        return;
    }
    if (fromRow == toRow && fromCol == toCol) {
        return;
    }

    // 直接交换单元格数据；空位（id=0）也参与交换
    std::swap(m_cells[fromRow][fromCol], m_cells[toRow][toCol]);
    m_manualModified = true;

    renderBoard();
    updateStatus();

    log(QStringLiteral("拖拽交换：(第 %1 行,第 %2 列) <-> (第 %3 行,第 %4 列)")
            .arg(fromRow + 1)
            .arg(fromCol + 1)
            .arg(toRow + 1)
            .arg(toCol + 1));
    statusBar()->showMessage(QStringLiteral("已交换座位（手动调整）"), 5000);
}

void MainWindow::onRuleDoubleClicked(QListWidgetItem* item)
{
    if (!item) {
        return;
    }

    bool ok = false;
    int index = item->data(Qt::UserRole).toInt(&ok);
    if (!ok) {
        index = m_ruleList->row(item);
    }
    editRuleAt(index);
}

void MainWindow::onRuleContextMenu(const QPoint& pos)
{
    QListWidgetItem* item = m_ruleList->itemAt(pos);
    if (!item) {
        return;
    }

    m_ruleList->setCurrentItem(item);

    bool ok = false;
    int index = item->data(Qt::UserRole).toInt(&ok);
    if (!ok) {
        index = m_ruleList->row(item);
    }

    QMenu menu(this);
    QAction* editAction = menu.addAction(QStringLiteral("编辑"));
    QAction* deleteAction = menu.addAction(QStringLiteral("删除"));
    QAction* selected = menu.exec(m_ruleList->viewport()->mapToGlobal(pos));

    if (selected == editAction) {
        editRuleAt(index);
    } else if (selected == deleteAction) {
        deleteRuleAt(index);
    }
}

void MainWindow::onToggleShowId(bool checked)
{
    m_showId = checked;
    renderBoard();
}

void MainWindow::editStudentAt(int row, int col)
{
    if (row < 0 || row >= k_rowNum || col < 0 || col >= k_columnNum) {
        return;
    }

    const CellInfo& cell = m_cells[row][col];
    if (cell.id <= 0) {
        statusBar()->showMessage(QStringLiteral("空位没有学生可编辑"), 3000);
        return;
    }

    const int studentIndex = findStudentIndex(cell.id);
    if (studentIndex < 0) {
        // m_cells 与 m_students 不一致：清理后直接返回
        renderBoard();
        updateStatus();
        return;
    }

    StudentRec& student = m_students[studentIndex];

    StudentDialog dialog(this);
    dialog.setStudent(student.id, student.height, student.gender, student.name);
    dialog.setIdReadOnly(true);     // 编辑时 id 只读
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    student.height = dialog.height();
    student.gender = dialog.gender();
    student.name = dialog.name();

    // 同步所有该学生所在的单元格（正常情况下只有一个）
    for (int r = 0; r < k_rowNum; ++r) {
        for (int c = 0; c < k_columnNum; ++c) {
            if (m_cells[r][c].id == student.id) {
                m_cells[r][c].name = student.name;
                m_cells[r][c].gender = student.gender;
            }
        }
    }

    m_manualModified = true;
    renderBoard();
    updateStatus();

    log(QStringLiteral("编辑学生：id=%1，姓名=%2，身高=%3，性别=%4。")
            .arg(student.id)
            .arg(student.name)
            .arg(student.height)
            .arg(genderText(student.gender)));
    statusBar()->showMessage(QStringLiteral("已修改学生 id=%1").arg(student.id), 5000);
}

void MainWindow::deleteStudentAt(int row, int col)
{
    if (row < 0 || row >= k_rowNum || col < 0 || col >= k_columnNum) {
        return;
    }

    const int id = m_cells[row][col].id;
    if (id <= 0) {
        statusBar()->showMessage(QStringLiteral("空位没有学生可删除"), 3000);
        return;
    }
    const QString name = m_cells[row][col].name;

    const auto answer = QMessageBox::question(
        this, QStringLiteral("确认删除"),
        QStringLiteral("确定删除学生 id=%1（%2）吗？").arg(id).arg(name),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer != QMessageBox::Yes) {
        return;
    }

    auto it = std::remove_if(m_students.begin(), m_students.end(),
                             [id](const StudentRec& student) { return student.id == id; });
    const bool removed = (it != m_students.end());
    m_students.erase(it, m_students.end());

    // 清空该学生在座位表中的所有出现位置
    for (int r = 0; r < k_rowNum; ++r) {
        for (int c = 0; c < k_columnNum; ++c) {
            if (m_cells[r][c].id == id) {
                m_cells[r][c] = CellInfo {};
            }
        }
    }

    m_manualModified = true;
    renderBoard();
    updateStatus();

    if (removed) {
        log(QStringLiteral("删除学生：id=%1，姓名=%2。").arg(id).arg(name));
        statusBar()->showMessage(QStringLiteral("已删除学生 id=%1").arg(id), 5000);
    } else {
        log(QStringLiteral("学生 id=%1 已不在学生列表中，仅清空座位。").arg(id));
        statusBar()->showMessage(QStringLiteral("座位已清空"), 5000);
    }
}

void MainWindow::editRuleAt(int index)
{
    if (index < 0 || index >= ruleCount()) {
        return;
    }

    QSet<int> validIds;
    for (const auto& student : m_students) {
        validIds.insert(student.id);
    }
    const int defaultId = m_students.empty() ? 1 : m_students.front().id;

    RuleDialog dialog(this, defaultId);
    dialog.setValidStudentIds(validIds);
    dialog.setRule(m_rulesObj[keyRules][index]);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    neb::CJsonObject rule = dialog.getRule();
    m_rulesObj[keyRules].Replace(index, rule);

    refreshRuleList(index);
    updateStatus();

    int id = 0;
    rule.Get(keyId, id);
    log(QStringLiteral("编辑规则 #%1：id=%2。").arg(index).arg(id));
    statusBar()->showMessage(QStringLiteral("已修改规则 #%1").arg(index), 5000);
}

void MainWindow::deleteRuleAt(int index)
{
    if (index < 0 || index >= ruleCount()) {
        return;
    }

    int id = 0;
    m_rulesObj[keyRules][index].Get(keyId, id);

    const auto answer = QMessageBox::question(
        this, QStringLiteral("确认删除"),
        QStringLiteral("确定删除规则 #%1（id=%2）吗？").arg(index).arg(id),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer != QMessageBox::Yes) {
        return;
    }

    // CJsonObject 没有可靠的“按下标删除后刷新引用”语义，这里重建整个 rules 数组
    const int oldCount = ruleCount();
    neb::CJsonObject newRules;
    newRules.AddEmptySubArray(keyRules);
    for (int i = 0; i < oldCount; ++i) {
        if (i == index) {
            continue;
        }
        newRules[keyRules].Add(m_rulesObj[keyRules][i]);
    }
    m_rulesObj = std::move(newRules);

    const int newCount = ruleCount();
    refreshRuleList(std::min(index, newCount - 1));
    updateStatus();

    log(QStringLiteral("删除规则 #%1（id=%2）。").arg(index).arg(id));
    statusBar()->showMessage(QStringLiteral("已删除规则 #%1").arg(index), 5000);
}

int MainWindow::findStudentIndex(int id) const
{
    for (int i = 0; i < static_cast<int>(m_students.size()); ++i) {
        if (m_students[i].id == id) {
            return i;
        }
    }
    return -1;
}

int MainWindow::ruleCount()
{
    if (!m_rulesObj.KeyExist(keyRules)) {
        return 0;
    }
    return m_rulesObj[keyRules].GetArraySize();
}

std::string MainWindow::buildStudentsJson() const
{
    // 存盘与排座共用：从 m_students 重新拼 {"students":[...]}
    neb::CJsonObject root;
    root.AddEmptySubArray(keyStudents);

    for (const auto& student : m_students) {
        neb::CJsonObject object;
        object.Add(keyId, student.id);
        object.Add(keyHeight, student.height);
        object.Add(keyGender, student.gender);
        object.Add(keyName, student.name.toStdString());
        root[keyStudents].Add(object);
    }

    return root.ToFormattedString();
}

void MainWindow::log(const QString& text)
{
    m_log->appendPlainText(text);
    m_log->verticalScrollBar()->setValue(m_log->verticalScrollBar()->maximum());
}

void MainWindow::showError(const QString& title, const QString& text)
{
    log(QStringLiteral("[错误] %1：%2").arg(title, text));
    QMessageBox::warning(this, title, text);
}

bool MainWindow::loadFileToString(const QString& path, std::string& out)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    const QByteArray data = file.readAll();
    out = data.toStdString();
    return true;
}

bool MainWindow::saveStringToFile(const QString& path, const std::string& text, QString& errorText)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        errorText = QStringLiteral("无法写入文件：%1（%2）").arg(path, file.errorString());
        return false;
    }

    QByteArray data = QByteArray::fromStdString(text);
    if (!data.endsWith('\n')) {
        data.append('\n');
    }

    if (file.write(data) != data.size()) {
        errorText = QStringLiteral("写入文件失败：%1（%2）").arg(path, file.errorString());
        return false;
    }
    if (!file.flush()) {
        errorText = QStringLiteral("刷新文件失败：%1（%2）").arg(path, file.errorString());
        return false;
    }

    return true;
}

QString MainWindow::genderText(int gender)
{
    switch (gender) {
    case 0:
        return QStringLiteral("男");
    case 1:
        return QStringLiteral("女");
    default:
        return QStringLiteral("未知");
    }
}
