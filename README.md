# seatManager —— 班级座位编排工具

`seatManager` 是一个使用 **C++17 + Qt6 Widgets** 编写的桌面座位编排程序，面向 **8 列 × 8 行（最多 64 人）** 的教室。

它支持学生与规则的 JSON 管理、按身高排座、按规则交换座位、手动拖拽换位，以及保存/加载数据。项目使用 CMake 构建，核心算法与 GUI 分离，方便继续扩展。

---

## 目录

- [1. 功能概览](#1-功能概览)
- [2. 界面与操作](#2-界面与操作)
- [3. 构建与运行](#3-构建与运行)
- [4. 数据文件格式](#4-数据文件格式)
- [5. 排座算法](#5-排座算法)
- [6. 代码结构](#6-代码结构)
- [7. 核心接口说明](#7-核心接口说明)
- [8. 常见问题](#8-常见问题)
- [9. 已知限制](#9-已知限制)
- [10. 开发说明](#10-开发说明)

---

## 1. 功能概览

- **学生管理**
  - 新建空学生列表
  - 添加学生（id 唯一，最多 64 人）
  - 双击座位编辑学生
  - 右键座位删除学生
  - 从 UTF-8 JSON 加载/保存学生列表
- **规则管理**
  - 新建空规则列表
  - 图形化添加/编辑规则
  - 右键或双击规则列表编辑/删除规则
  - 从 UTF-8 JSON 加载/保存规则列表
- **自动排座**
  - 按身高从矮到高排序
  - 同性别学生两两相邻就座
  - 按规则随机寻找符合条件的位置并交换
  - 规则按列表顺序执行（越靠前的规则越晚执行）
- **座位表交互**
  - 8×8 表格展示
  - 男蓝、女粉、未知灰
  - 单击查看详情
  - 双击编辑
  - 右键菜单编辑/删除
  - 单元格拖拽交换座位
- **辅助功能**
  - 日志区显示加载、保存、排座、错误信息
  - 一键清空日志
  - 状态栏显示学生数、规则数和手动调整状态
  - 执行排座前提示“手动调整将被覆盖”

---

## 2. 界面与操作

### 2.1 顶部菜单

程序顶部有三个菜单：**文件**、**编辑**、**视图**。

| 菜单 | 菜单项 | 快捷键 | 说明 |
| --- | --- | --- | --- |
| 文件 | 新建学生列表 | `Ctrl+N` | 清空学生与座位表 |
| 文件 | 新建规则列表 | `Ctrl+Shift+N` | 清空规则列表 |
| 文件 | 加载学生... | `Ctrl+O` | 读取 UTF-8 学生 JSON |
| 文件 | 加载规则... | `Ctrl+Shift+O` | 读取 UTF-8 规则 JSON |
| 文件 | 保存学生... | `Ctrl+S` | 保存学生列表 |
| 文件 | 保存规则... | `Ctrl+Shift+S` | 保存规则列表 |
| 文件 | 退出 | `Ctrl+Q` | 关闭程序 |
| 编辑 | 添加学生... | `Ctrl+Alt+N` | 打开学生对话框 |
| 编辑 | 添加规则... | `Ctrl+Alt+R` | 打开规则对话框 |
| 编辑 | 执行排座 | `F5` | 重新计算座位 |
| 视图 | 显示 id | `Ctrl+I` | 切换座位表是否显示 id |
| 视图 | 清空日志 | `Ctrl+L` | 清空日志区 |

### 2.2 座位表

左侧是 8×8 座位表，行列编号均为 1 到 8。

- 单元格显示 `姓名` 和 `(id=N)`；空位显示 `—`。
- 男生浅蓝，女生浅粉，未知性别浅灰。
- **单击**：状态栏显示该座位的行列、id、姓名、性别。
- **双击**：编辑该座位上的学生，id 只读。
- **右键**：菜单包含“编辑”和“删除”。
- **拖拽**：按住某个单元格拖动到另一个单元格，松开后交换两个座位。
  - 可以拖到空位。
  - 拖到自己身上、拖到表格外会被忽略。
  - 拖拽后状态栏提示“手动调整”。

> 注意：座位表初始为空。加载学生 JSON 后不会自动排座，需要点击“执行排座”。

### 2.3 学生管理

- **新建学生列表**：清空 `m_students` 和座位表，状态栏提示“新建学生列表（空）”。
- **添加学生**：弹窗填写 id、姓名、身高、性别。
  - id 范围 1..999999。
  - id 不能重复。
  - 学生总数不能超过 64。
  - 姓名不能为空。
- **编辑学生**：双击座位或右键“编辑”打开对话框，id 只读。
- **删除学生**：右键座位“删除”，二次确认后从学生列表和座位表移除。
- **保存学生**：写入 `{"students": [...]}` 格式的 UTF-8 JSON。

> 学生编辑/删除依赖座位表：只有已经出现在座位表上的学生才能通过双击或右键编辑/删除。加载学生后如果还没排座，座位表是空的，此时建议先执行排座。

### 2.4 规则管理

右侧上方是规则列表，每一项显示：

```text
#0  id=3  [L]  [C]  [允许异性]
```

- `#i` 是规则在数组中的下标。
- `id` 是**规则所属学生**的 id，不是规则编号。
- 双击或右键“编辑”打开规则对话框。
- 右键“删除”二次确认后删除规则。
- 规则列表为空时允许排座，此时只按身高和性别排座，不执行额外规则。

规则对话框支持：

- **规则 id**：这条规则作用于哪个学生。
- **L（行列范围）**
  - 启用/禁用。
  - `isInverse`：反选，表示“范围外”。
  - 全列/全行：不输出对应的 `columnRange` / `rowRange`，由 core 使用默认全范围。
  - 列范围、行范围：1..8，最小值不能大于最大值。
- **C（圆形范围）**
  - 启用/禁用。
  - 圆心是**学生 id**，不是行列坐标。
  - 半径使用曼哈顿距离：`|x1-x2| + |y1-y2| <= radius`。
  - `isInverse`：反选，表示“圆外”。
- **允许异性（diffGender）**：勾选后不限制性别，否则只能和同性学生交换。

> GUI 要求 L 和 C 至少启用一个。直接编辑 JSON 时如果两者都没有，规则在 core 中会被当作只做性别约束，行为可能与 GUI 预期不同。

### 2.5 日志与状态栏

- 右侧下方是只读日志区，自动滚到底部。
- 标题右侧有“清空”按钮，也可以通过“视图 → 清空日志”或 `Ctrl+L` 清空。
- 底部状态栏显示：学生人数、规则条数、是否做过手动调整。

---

## 3. 构建与运行

### 3.1 环境要求

- **C++17** 编译器（Windows 上推荐 MinGW 13+ 或 MSVC 2022）
- **CMake 3.16+**（项目声明的最低版本是 3.10，但 Qt6 通常需要更高版本）
- **Qt 6.5+**，需要 `Widgets` 模块
- 支持的操作系统：Windows / Linux / macOS（本项目主要在 Windows + Qt 6.10.3 + MinGW 下验证）

### 3.2 构建命令

在项目根目录执行：

```bash
cmake -S . -B build
cmake --build build -j
```

如果 CMake 找不到 Qt6，请指定 Qt 的安装路径。例如 Windows + MinGW：

```bash
cmake -S . -B build -G "MinGW Makefiles" ^
  -DCMAKE_PREFIX_PATH="C:/Qt/6.10.3/mingw_64"
cmake --build build -j
```

Linux 示例：

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH="$HOME/Qt/6.10.3/gcc_64"
cmake --build build -j
```

也可以直接用 Qt Creator 打开项目根目录的 `CMakeLists.txt`，选择对应的 Qt Kit 构建。

### 3.3 运行

Windows：

```powershell
.\build\seatManager.exe
```

Linux/macOS：

```bash
./build/seatManager
```

如果 Windows 上提示缺少 Qt DLL，可以使用 Qt 自带的 `windeployqt`：

```powershell
C:\Qt\6.10.3\mingw_64\bin\windeployqt.exe .\build\seatManager.exe
```

### 3.4 构建产物

- `build/seatManager.exe`（Windows）
- `build/seatManager`（Linux/macOS）

`build/` 是生成目录，可以安全删除后重新执行 CMake 配置。

---

## 4. 数据文件格式

所有 JSON 文件都使用 **UTF-8** 编码。项目根目录提供了两个示例文件：

- `studentsList.json`
- `rules.json`

### 4.1 学生文件

```json
{
  "students": [
    { "id": 1, "height": 178, "gender": 0, "name": "张三" },
    { "id": 2, "height": 163, "gender": 1, "name": "李四" },
    { "id": 3, "height": 176, "gender": 0, "name": "王五" }
  ]
}
```

字段说明：

| 字段 | 类型 | 说明 |
| --- | --- | --- |
| `students` | 数组 | 所有学生，最多 64 个 |
| `id` | 整数 | 学生唯一编号，必须大于 0，不能重复 |
| `height` | 整数 | 身高，通常使用厘米 |
| `gender` | 整数 | `0` 男，`1` 女 |
| `name` | 字符串 | 学生姓名 |

加载时会校验：

- 文件必须是合法 JSON。
- 根对象必须包含 `students` 数组。
- 学生数量不能超过 64。
- 每条记录必须包含 `id`、`height`、`gender`、`name`。
- `id` 必须在 1..999999 之间，且不能重复。
- `gender` 只能是 0 或 1。

### 4.2 规则文件

```json
{
  "rules": [
    {
      "id": 1,
      "diffGender": false,
      "L": [
        {
          "isInverse": false,
          "columnRange": [1, 8],
          "rowRange": [1, 4]
        }
      ]
    },
    {
      "id": 2,
      "diffGender": true,
      "L": [
        {
          "isInverse": false,
          "columnRange": [1, 8],
          "rowRange": [1, 7]
        }
      ],
      "C": [
        {
          "isInverse": true,
          "circle": [20, 8]
        }
      ]
    }
  ]
}
```

字段说明：

| 字段 | 类型 | 说明 |
| --- | --- | --- |
| `rules` | 数组 | 所有规则，可以为空 |
| `id` | 整数 | **规则所属学生的 id**，不是规则序号；必须存在于学生列表，且规则之间不能重复 |
| `diffGender` | 布尔 | 是否允许和异性交换；缺省为 `false` |
| `L` | 数组 | 行列范围；可选 |
| `L[0].isInverse` | 布尔 | `true` 表示范围外 |
| `L[0].columnRange` | 数组 | 列范围 `[min, max]`，1..8；省略表示全列 |
| `L[0].rowRange` | 数组 | 行范围 `[min, max]`，1..8；省略表示全行 |
| `C` | 数组 | 圆形范围；可选 |
| `C[0].isInverse` | 布尔 | `true` 表示圆外 |
| `C[0].circle` | 数组 | `[圆心学生id, 半径]`，圆心是学生 id，不是坐标 |

说明：

- `L` 和 `C` 可以单独存在，也可以同时存在（两者同时存在时取交集）。
- `isInverse` 为 `true` 时，L 表示“行列范围之外”，C 表示“圆之外”。
- `C` 的半径使用曼哈顿距离，不是欧氏距离。
- 规则按数组顺序保存；执行时从后向前执行，越靠前的规则越晚执行。
- `id` 是学生 id，因此同一个学生可以有多条规则，但规则 id 在 `rules` 数组内必须唯一。

### 4.3 关于座位顺序

学生 JSON 只保存学生信息，**不保存座位顺序**。手动拖拽后的座位顺序不会写入学生文件；再次点击“执行排座”会重新计算座位。

---

## 5. 排座算法

执行排座时，GUI 会构造一次 `SeatManager`，调用顺序为：

```cpp
SeatManager manager(studentsJson, rulesJson);

if (!manager.main()) {
    // 学生或规则校验失败，停止
    return;
}

manager.execute();
```

- `main()`：校验学生 JSON 和规则 JSON，准备规则列表。
- `execute()`：按身高排序、排座、执行规则。
- **只有 `main()` 返回 `true` 时才调用 `execute()`。**

具体流程：

1. **校验学生**
   - 人数不超过 64。
   - id 合法且不重复。
   - 性别只能是 0/1。
   - 必填字段存在。
2. **身高排序**
   - 按 `height` 从小到大排序。
3. **初始排座**
   - 依次取未安排的学生。
   - 同性别学生两两相邻放置。
   - 如果某个性别剩余人数为奇数，插入一个空位占位。
   - 其余座位用空位填充。
4. **执行规则**
   - 按规则数组从后向前遍历。
   - 对每条规则，随机尝试最多 1000 个已占用的座位。
   - 找到同时满足 L、C、性别条件的座位后，与该座位上的学生交换。
   - 如果 1000 次都没有找到，跳过该规则。

> 排座结果是随机的。相同的学生和规则多次执行可能得到不同座位。

---

## 6. 代码结构

```text
seatManager/
├── CMakeLists.txt              # CMake 构建脚本
├── README.md                   # 本文件
├── studentsList.json           # 学生示例数据
├── rules.json                  # 规则示例数据
├── src/
│   ├── main.cpp                # 程序入口，创建 MainWindow
│   ├── core.cpp                # 核心算法：学生、规则、排座
│   ├── mainwindow.cpp          # 主窗口：菜单、座位表、规则列表、日志
│   ├── studentdialog.cpp       # 学生添加/编辑对话框
│   ├── ruledialog.cpp          # 规则添加/编辑对话框
│   ├── seattable.cpp           # 座位表，处理单元格拖拽交换
│   ├── include/
│   │   ├── core.hpp            # 核心类与常量声明
│   │   ├── mainwindow.hpp
│   │   ├── studentdialog.hpp
│   │   ├── ruledialog.hpp
│   │   ├── seattable.hpp
│   │   ├── CJsonObject.hpp     # JSON 封装
│   │   ├── cJSON.h             # cJSON 头文件
│   │   └── random.hpp          # 随机数工具
│   └── utils/
│       ├── CJsonObject.cpp
│       └── cJSON.c
└── build/                      # CMake 生成目录（可删除）
```

主要数据流：

- `MainWindow` 长期持有：
  - `m_students`：学生记录
  - `m_rulesObj`：规则 JSON
  - `m_cells`：当前座位表内容
- 添加/编辑/删除学生或规则时，直接更新这三份数据。
- 执行排座时，从 `m_students` 和 `m_rulesObj` 生成 JSON，交给 `SeatManager`。
- 排座完成后，把 `SeatManager` 的 board 结果写回 `m_cells` 并刷新界面。
- 保存时直接从 `m_students` / `m_rulesObj` 生成 JSON，不依赖 `SeatManager`。

---

## 7. 核心接口说明

### 7.1 `SeatManager`

`SeatManager` 是排座入口，定义在 `src/include/core.hpp`。

```cpp
SeatManager(std::string studentsJson, std::string rulesJson);
bool main();       // 校验学生/规则
void execute();    // 排序、排座、执行规则
```

常用接口：

```cpp
int getId(Point point) const;
const std::string& getName(Point point) const;
int getGender(Point point) const;
int getHeight(Point point) const;

const StudentsList& getStudentsList() const;
const RulesList& getRulesList() const;
const neb::CJsonObject& getStudentsJson() const;
const neb::CJsonObject& getRulesJson() const;

bool addStudent(Student student);
bool deleteStudent(int id);
bool addRule(Rule rule);
bool deleteRule(int id);
```

`Point` 的行列号从 **1** 开始：

```cpp
Point p {3, 2};   // 第 3 列、第 2 行
int id = manager.getId(p);
```

空位 id 为 0 或负数；大于 0 的 id 表示真实学生。

### 7.2 空列表常量

`core.hpp` 提供了两个空列表常量，方便构造空的 `SeatManager`：

```cpp
inline const std::string k_studentsJson {"{\"students\": []}"};
inline const std::string k_rulesJson {"{\"rules\": []}"};
```

例如：

```cpp
SeatManager manager(k_studentsJson, k_rulesJson);
```

### 7.3 其他核心类

- `StudentsList`：学生集合与座位数组，负责校验、排序、排座、交换。
- `RulesList`：规则集合，负责校验、设置、执行规则。
- `Rule`：单条规则，支持从 JSON 读取，也支持通过 `Range`/`CRange` 设置。
- `Range`：一维范围，支持反选。
- `CRange`：圆形范围，内部使用坐标和半径。

更详细的接口请直接查看 `src/include/core.hpp`。

---

## 8. 常见问题

### 8.1 CMake 找不到 Qt6

错误通常类似：

```text
Could not find a package configuration file provided by "Qt6"
```

解决方式：

- 确认安装了 Qt6 的 `Widgets` 模块。
- 设置 `CMAKE_PREFIX_PATH` 指向 Qt 安装目录，例如：

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH="C:/Qt/6.10.3/mingw_64"
```

- 使用 Qt Creator 的 Kit 构建，通常会自动配置路径。

### 8.2 中文显示乱码

- 所有 JSON 文件必须保存为 **UTF-8**。
- 不要使用 GBK/ANSI 编码保存学生或规则文件。
- README 和源代码也使用 UTF-8。
- 如果编辑器显示乱码，请检查文件编码设置。

### 8.3 提示“学生信息错误”

常见原因：

- 学生人数超过 64。
- `id` 小于等于 0。
- `id` 重复。
- `gender` 不是 0 或 1。
- 缺少 `id`、`height`、`gender`、`name` 字段。
- 根对象缺少 `students` 数组。

### 8.4 提示“规则信息错误”

常见原因：

- 根对象缺少 `rules` 数组。
- 规则 `id` 不存在于学生列表。
- 两条规则使用了相同的 `id`。
- `L` 或 `C` 的数组为空，或缺少 `isInverse`。
- `L` 或 `C` 的结构不完整，例如 `circle` 不足两项。

### 8.5 拖拽没有反应

- 需要按住某个**单元格**拖动，而不是表头。
- 必须拖到另一个**单元格**上再松开。
- 只支持座位表内部拖拽，不支持拖到其他程序。
- 拖拽后如果移动了座位，执行排座时会提示是否覆盖手动调整。

### 8.6 执行排座后手动座位被覆盖

这是设计行为。手动拖拽/编辑/删除会设置“手动调整”标记；执行排座前会弹窗确认。选择“Yes”后，座位会按最新学生和规则重新计算。

### 8.7 保存后座位顺序没有恢复

学生 JSON 只保存学生信息，不保存座位顺序。再次加载后需要重新执行排座，或者在界面上重新拖拽。

---

## 9. 已知限制

- 教室固定为 8×8，最多 64 人；行列数由 `k_columnNum` / `k_rowNum` / `k_peopleNum` 常量控制。
- 按身高升序排座，暂不支持其他排序字段。
- 规则执行是随机尝试，不保证一定找到满足条件的位置；找不到时会跳过该规则。
- 学生编辑/删除依赖座位表：加载后未排座时，座位表为空，无法通过双击/右键编辑学生。
- 手动调整的座位顺序不会写入学生 JSON。
- 规则 id 必须唯一；GUI 不做强制拦截，重复 id 会在执行排座时由 core 报错。
- C 规则的圆心必须是学生 id。当前 core 在排座前调用 `RulesList::set()`，此时座位表尚未生成，C 规则可能只保留默认半径 `0`；执行时圆心会刷新到正确座位，但半径可能仍按 0 处理。若 C 规则效果不符合预期，请检查 `src/core.cpp` 中 `main()` 与 `execute()` 的调用顺序。
- 当前没有撤销/重做功能。

---

## 10. 开发说明

- C++ 标准：C++17。
- GUI：Qt6 Widgets，使用 `AUTOMOC`。
- JSON：`neb::CJsonObject` + `cJSON`。
- 座位表拖拽通过 `SeatTable` 子类实现：
  - `startDrag()` 使用模型自己的 MIME 数据并记录源单元格。
  - `dragEnterEvent()` / `dragMoveEvent()` 只接受本表内部、落在单元格上的拖拽。
  - `dropEvent()` 不调用基类实现，只发出 `swapRequested` 信号，由主窗口交换 `m_cells`。
- 日志捕获：执行排座时把 `std::cout` 重定向到 `std::ostringstream`，结束后恢复，避免 core 的错误输出丢失。

如果需要扩展功能，建议优先修改 `core` 的接口，再调整 `MainWindow` 的数据流，避免 GUI 与核心逻辑各自维护一套规则。
