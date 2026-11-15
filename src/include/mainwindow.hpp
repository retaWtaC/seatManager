#pragma once

#include <QMainWindow>
#include <QString>

#include <string>
#include <vector>

#include "CJsonObject.hpp"

class SeatTable;
class QListWidget;
class QListWidgetItem;
class QPlainTextEdit;
class QLabel;
class QAction;
class QPoint;

// 主窗口：长期持有学生、规则与座位三份数据。
// SeatManager 只在“执行排座”时构造一次，先 main() 校验，再 execute() 排座。
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    // 学生
    void onNewStudents();
    void onLoadStudents();
    void onSaveStudents();
    void onAddStudent();

    // 规则
    void onNewRules();
    void onLoadRules();
    void onSaveRules();
    void onAddRule();

    // 排座 / 座位表 / 日志
    void onRun();
    void onCellClicked(int row, int col);
    void onCellDoubleClicked(int row, int col);
    void onTableContextMenu(const QPoint& pos);
    void onDragDrop(int fromRow, int fromCol, int toRow, int toCol);
    void onToggleShowId(bool checked);
    void onClearLog();

    // 规则列表
    void onRuleDoubleClicked(QListWidgetItem* item);
    void onRuleContextMenu(const QPoint& pos);

private:
    struct StudentRec {
        int id = 0;
        int height = 0;
        int gender = 2;
        QString name;
    };

    struct CellInfo {
        int id = 0;
        QString name;
        int gender = 2;
    };

    void buildUi();
    void clearBoard();                 // 重置 m_cells 与表格项
    void renderBoard();                // 把 m_cells 画到表格，并清理与 m_students 的不一致
    void refreshRuleList(int selectIndex = -1);
    void updateStatus();

    void editStudentAt(int row, int col);
    void deleteStudentAt(int row, int col);
    void editRuleAt(int index);
    void deleteRuleAt(int index);

    int findStudentIndex(int id) const;
    int ruleCount();
    std::string buildStudentsJson() const;

    void log(const QString& text);
    void showError(const QString& title, const QString& text);
    bool loadFileToString(const QString& path, std::string& out);
    bool saveStringToFile(const QString& path, const std::string& text, QString& errorText);
    static QString genderText(int gender);

    // 数据模型
    std::vector<StudentRec> m_students;
    neb::CJsonObject m_rulesObj;       // 根对象：{ "rules": [...] }
    std::vector<std::vector<CellInfo>> m_cells;  // [row][col]

    bool m_manualModified = false;     // 拖拽/编辑/删除后置 true
    bool m_showId = true;
    QString m_studentsPath;
    QString m_rulesPath;

    // UI
    SeatTable* m_table = nullptr;
    QListWidget* m_ruleList = nullptr;
    QPlainTextEdit* m_log = nullptr;
    QLabel* m_status = nullptr;
    QAction* m_showIdAction = nullptr;
};
