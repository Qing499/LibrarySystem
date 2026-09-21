# 图书馆借阅管理系统（C++ / 面向对象程序设计实验）

西南科技大学 · 计算机科学与计算学院 ·《面向对象程序设计(C++)》实验指导书（第 29 页）
题目：**图书馆借阅管理系统** —— 对学生用户信息、教师用户信息和图书信息进行管理，
实现借书 / 还书，并根据借还日期判断是否逾期、计算罚款。

---

## 一、实验要求与实现对照

| 实验指导书要求 | 实现位置 |
| --- | --- |
| 学生基本信息：学号、姓名、院系、最大借阅图书数量等属性 | `src\Model\Student.h` / `Student.cpp`（基类 `Person`） |
| 教师基本信息：工号、姓名、院系、最大借阅图书数量等属性 | `src\Model\Teacher.h` / `Teacher.cpp`（基类 `Person`） |
| 图书基本信息：图书条码号、图书号 ISBN、书名、作者、出版社、是否可借 | `src\Model\Book.h` / `Book.cpp` |
| 输入、输出、修改、查询以上信息功能 | `src\UI\main.cpp` 菜单：借阅人管理、图书管理、查询与统计 |
| 借阅人可以借书 / 还书 | `Library::borrowBook` / `Library::returnBook`（`src\Service\Library.cpp`） |
| 根据借还日期判断是否逾期、逾期计算罚款 | `Date` 类的日期运算 + `BorrowRecord::overdueDaysAsOf` + `Library::returnBook` |
| 面向对象设计（封装、继承、多态） | `Person` 抽象基类 + 派生类 `Student` / `Teacher`（虚函数多态） |
| 设计并实现功能 | 完整可运行的控制台系统 + 数据存档 + 自检程序 |

> 课本“实验要求”中关于**类的设计与访问控制、默认构造函数、重载构造函数、
> 多文件组织、代码无编译错误**的逐条对照，见本文档第十节；
> 运行 `LibrarySystem.exe --demo`（或主菜单选 7）可看到构造函数与多态的实际运行结果。

---

## 二、编译与运行

### 方法 1：Visual Studio（推荐）

1. 双击 `LibrarySystem.sln`（Visual Studio 2022 打开，工具集 v143）；
2. 直接按 `F5` 或 `Ctrl+F5` 运行；
3. 也可以双击 `build_msvc.bat` 用 MSBuild 命令行编译。

工程已配置：`C++17`、警告等级 `Level 4`、`/utf-8`（保证中文源码与输出不乱码）、
输出目录 `build\x64\Debug\LibrarySystem.exe`。

### 方法 2：MinGW g++（备用）

```bat
build_gcc.bat
```

### 运行自检（检查程序是否出错）

```bat
run_selftest.bat
:: 等价于：
build\x64\Debug\LibrarySystem.exe --selftest
```

`--selftest` 会跑 130 项断言，覆盖日期运算、信息增删改查、借还书、最大借阅数量限制、
逾期判断与罚款计算、**默认构造函数与重载构造函数**、多态、存档与读取；
**全部通过时返回退出码 0**，可作为“防止出错”的回归测试。

### 运行类与对象演示

```bat
build\x64\Debug\LibrarySystem.exe --demo
:: 或者启动程序后，在主菜单选择 7
```

演示默认构造函数 / 重载构造函数的初始化结果、对象调用成员函数的过程，以及基类指针的多态行为。

### 本机实际验证结果

| 验证项 | 结果 |
| --- | --- |
| MSVC（VS2022 17.14 / MSVC 14.44，`/W4`）Debug x64 | 0 错误 0 警告 |
| MSVC Release x64 | 0 错误 0 警告 |
| MinGW g++ 14.2（`-Wall -Wextra -Wpedantic -O2`） | 0 错误 0 警告 |
| 自检程序（Debug / Release / g++ 三个版本） | 通过 130 项，失败 0 项 |
| 类与对象演示 `--demo`（Debug / Release / g++） | 输出正确，退出码 0 |
| 菜单交互冒烟测试（脚本化输入走完全部功能，含主菜单 7） | 0 次无效输入、0 次操作失败 |

---

## 三、目录结构（代码按功能分层）

```
LibrarySystem\
├─ LibrarySystem.sln                  Visual Studio 解决方案（双击打开，F5 运行）
├─ LibrarySystem.vcxproj              VS 工程（已按功能层配置筛选器与头文件搜索路径）
├─ LibrarySystem.vcxproj.filters      VS 筛选器：Core / Model / Service / UI / Test / Demo
├─ README.md                          本说明
├─ build_msvc.bat                     一键用 MSBuild 编译（Visual Studio 工具链）
├─ build_gcc.bat                      一键用 MinGW g++ 编译（备用/交叉验证）
├─ run_selftest.bat                   一键运行自检
├─ src\                               ——全部源码，按功能分 6 个文件夹——
│  ├─ Core\        基础工具层         Config.h、TextUtils.h、Date.h、Date.cpp
│  ├─ Model\       数据模型层         Person.h/.cpp、Student.h/.cpp、Teacher.h/.cpp、
│  │                                  Book.h/.cpp、BorrowRecord.h/.cpp
│  ├─ Service\     业务逻辑层         Library.h、Library.cpp
│  ├─ UI\          界面交互层         ConsoleUtils.h/.cpp、main.cpp
│  ├─ Test\        自检测试层         SelfTest.h、SelfTest.cpp
│  └─ Demo\        类与对象演示       ClassDemo.h、ClassDemo.cpp
└─ build\                            编译输出（自动生成，可随时删除）
   ├─ x64\Debug\LibrarySystem.exe    Visual Studio Debug 版
   ├─ x64\Release\LibrarySystem.exe  Visual Studio Release 版
   └─ gcc\LibrarySystem_gcc.exe      MinGW g++ 版
```

各层职责与依赖方向（依赖单向：UI → Service → Model → Core，不出现反向依赖）：

| 层 | 文件夹 | 职责 | 依赖 |
| --- | --- | --- | --- |
| 基础工具层 | `src\Core` | 日期运算与校验（`Date`）、文本处理（`TextUtils`）、借阅规则参数与金额格式化（`Config`） | 无 |
| 数据模型层 | `src\Model` | 实体类：借阅人抽象基类 `Person` 与派生类 `Student` / `Teacher`、`Book`、`BorrowRecord` | Core |
| 业务逻辑层 | `src\Service` | `Library`：信息增删改查、借书还书、逾期判断与罚款、统计、数据存档 | Core + Model |
| 界面交互层 | `src\UI` | `ConsoleUtils` 输入校验与控制台编码、`main.cpp` 全部菜单 | Core + Model + Service |
| 自检测试层 | `src\Test` | `SelfTest`：130 项回归断言（`--selftest`） | Core + Model + Service |
| 类与对象演示 | `src\Demo` | `ClassDemo`：构造函数、对象与成员函数调用、多态的运行演示（`--demo`） | Core + Model + Service |

工程已在 `LibrarySystem.vcxproj` 中把 6 个层目录加入头文件搜索路径
（属性 `LayerIncludeDirs`，对应 g++ 的 `-I src\Core -I src\Model ...`），
因此源码中仍然使用 `#include "Book.h"` 这样的简单写法，移动文件时只需改工程配置。

---

## 四、类设计（继承与多态）

```
                 +---------------------------+
                 |   Person  (抽象基类)       |
                 |---------------------------|
                 | - id_ / name_ / department_|
                 | - maxBorrow_ / borrowedCount_ |
                 |---------------------------|
                 | + roleName() = 0  (纯虚)   |
                 | + idLabel()  = 0  (纯虚)   |
                 | + typeTag()  = 0  (纯虚)   |
                 | + borrowDays(policy) = 0   |
                 | + finePerDay(policy) = 0   |
                 | + toString()               |
                 +-------------+-------------+
                               |
              +----------------+----------------+
              |                                 |
      +-------+--------+                +-------+--------+
      |    Student     |                |    Teacher     |
      | 学号 / 借期30天 |                | 工号 / 借期60天 |
      | 罚款 0.20 元/天 |                | 罚款 0.10 元/天 |
      +----------------+                +----------------+

  Library  --1..*--> Person（unique_ptr 多态持有）
           --1..*--> Book
           --1..*--> BorrowRecord --> Date
```

要点：

* **封装**：所有数据成员私有，通过 getter / setter 访问；业务规则集中在 `Library` 中，界面只负责输入输出。
* **继承 + 多态**：`Library` 用 `std::vector<std::unique_ptr<Person>>` 保存学生和教师，
  调用 `roleName()`、`borrowDays()`、`finePerDay()` 时自动绑定到派生类实现，
  新增一种借阅人（如研究生）只需增加一个派生类。
* **组合**：`Library` 组合 `Book`、`BorrowRecord`、`BorrowPolicy`；`BorrowRecord` 组合 `Date`。

---

## 五、功能菜单

```
主菜单
  1. 借阅人管理    添加学生 / 添加教师 / 修改 / 删除 / 按学号工号查询 / 关键字查询 / 列出
  2. 图书管理      添加 / 修改 / 删除 / 按条码号或 ISBN 查询 / 关键字查询 / 列出全部
  3. 借书 / 还书   借书 / 还书（按条码号）/ 还书（按借阅人+条码号）/ 查询某人在借图书
  4. 查询与统计    逾期未还清单（含罚款）/ 某借阅人借阅历史 / 馆藏与借阅统计 / 全部记录
  5. 参数设置      查看 / 修改（最大借阅数量、借期、逾期罚款单价）
  6. 数据存档      保存 / 另存为 / 读取
  0. 退出系统      （退出时自动保存到 library_data.txt）
```

首次运行若不存在 `library_data.txt`，会自动载入演示数据（5 名借阅人、6 种图书），可直接试用。

---

## 六、业务规则（可在“参数设置”中修改）

| 角色 | 最大借阅数量 | 借期 | 逾期罚款 |
| --- | --- | --- | --- |
| 学生 | 5 本 | 30 天 | 0.20 元/天 |
| 教师 | 10 本 | 60 天 | 0.10 元/天 |

**借书**依次校验：借阅人存在 → 图书存在 → 图书可借（`是否可借`）→ 未超过最大借阅数量 →
没有逾期未还的图书 → 生成借阅记录（应还日期 = 借出日期 + 借期），并把图书置为“已借出”。

**还书**：找到该借阅人的在借记录 → 校验归还日期不早于借出日期 →
`逾期天数 = max(0, 归还日期 - 应还日期)` → `罚款 = 逾期天数 × 每日罚款单价`（四舍五入到分）→
图书恢复“可借”、在借数量 -1、记录标记为已归还并保存罚款金额。

**其他约束**：未归还的图书不能删除；有未归还图书的借阅人不能删除；
条码号是图书唯一标识（修改图书信息时不允许改条码号）；存档读取时按借阅记录自动重建
“当前在借数量”和“是否可借”，保证数据不会自相矛盾。

---

## 七、数据文件格式

`library_data.txt`，一行一条记录，字段用 `|` 分隔（`#` 开头为注释）：

```
POLICY|5|10|30|60|0.20|0.10
MEMBER|STUDENT|20210001|张三|计算机科学与技术学院|5|0
BOOK|BC-0001|978-7-111-40701-0|C++ Primer（第5版）|Stanley B. Lippman|机械工业出版社|1
RECORD|20210001|BC-0001|2026-09-20|2026-10-20|1|2026-10-30|10|2.00
```

`RECORD` 字段依次为：借阅人编号、条码号、借出日期、应还日期、是否已归还（1/0）、
归还日期（未归还为 `-`）、逾期天数、罚款。

---

## 八、自检覆盖内容（`--selftest`，共 130 项）

1. 日期：多种格式解析、非法日期拒绝、闰年、日期加减、相差天数；
2. 信息管理：学生 / 教师 / 图书的添加、修改、删除、重复校验、字段非空校验、关键字查询；
3. 借还书：正常借还、重复借阅、超出最大借阅数量、图书已借出、逾期未还禁止再借；
4. 逾期罚款：按期归还 0 元、逾期 6 天 = 1.20 元（学生）、逾期 6 天 = 0.60 元（教师）、
   修改借期与罚款单价后的重算；
5. 异常分支：归还未借出的图书、归还日期早于借出日期、重复归还、删除保护；
6. 统计与一致性：逾期清单、预计罚款、已收罚款合计、`refreshDerivedState` 自动修复；
7. 存档：保存→读取后人员 / 图书 / 记录数量与罚款金额一致，读取不存在的文件不破坏原有数据；
8. **构造函数与多态**：`Student` / `Teacher` / `Book` / `Date` / `BorrowRecord` / `Library`
   的默认构造函数初始化值、三参数与四参数重载构造函数、`Library` 自定义借阅规则构造、
   基类指针 `Person*` 调用虚函数。

---

## 九、常见问题

* **中文乱码**：程序运行时会自动把控制台切到 UTF-8；工程已加 `/utf-8` 编译选项。
  若在旧版控制台仍显示异常，可在命令行先执行 `chcp 65001`。
* **提示找不到 v143 工具集**：说明 Visual Studio 没有安装“使用 C++ 的桌面开发”工作负载，
  请在 Visual Studio Installer 中勾选安装；或改用 `build_gcc.bat`。
* **想要清空数据重新开始**：删除程序目录下的 `library_data.txt` 即可。

---

## 十、实验设计要求符合性对照

### （一）设计部分

| 实验要求 | 本系统的做法 |
| --- | --- |
| （1）根据参考类与提示设计类，设计数据成员与成员函数，确定访问控制，符合待抽象的问题 | `Person`（借阅人抽象基类）提炼出学生/教师的公共属性：编号、姓名、院系、最大借阅数量、当前在借数量；差异部分用**纯虚函数**（`roleName` / `idLabel` / `typeTag` / `borrowDays` / `finePerDay`）留给派生类。`Book`、`BorrowRecord`、`Date`、`Library` 各司其职。访问控制：数据成员一律 `private`，基类中需要派生类直接使用的成员放在 `protected`，对外只提供 `public` 的 getter / setter 与业务函数；`Library` 的成员容器私有，外部只能通过成员函数操作 |
| （2）按类与对象的概念完成对象定义，并由对象调用其成员函数，成员函数设计合理 | `src\Demo\ClassDemo.cpp` 演示了各类对象的定义与成员函数调用（`setName` / `toString` / `matches` / `addDays` / `overdueDaysAsOf` / `addMember` / `borrowBook` …）；`main.cpp` 的菜单也是“对象调用成员函数”的实际应用。成员函数职责单一：`Date` 只负责日期运算，`Book` 只管图书自身状态，业务规则集中在 `Library`，界面只做输入输出 |
| （3）设计默认构造函数，把数据成员初始化为一定的值 | 每个类都有明确的默认构造函数，并在 `.cpp` 中用初始化列表写清初值：`Person()`（"未设置"/"未命名"/"未填写"/0）、`Student()`（最大可借取规则默认值 5）、`Teacher()`（默认 10）、`Book()`（文本为空串、状态可借）、`Date()`（0/0/0，空日期）、`BorrowRecord()`（空串、空日期、未归还、罚款 0）、`Library()`（容器为空、规则取默认值） |
| （4）设计重载的构造函数，以相应参数构造需要的对象 | `Student` / `Teacher` 各有 3 个构造函数（默认、三参数、四参数）；`Book` 支持 5 参数与 6 参数；`Date(年,月,日)`；`BorrowRecord(编号,条码号,借出日期,应还日期)`；`Library` 支持默认构造与 `explicit Library(const BorrowPolicy&)` 自定义规则构造。编译期由参数个数/类型决定调用哪个重载版本 |

### （二）实现部分

| 实验要求 | 本系统的做法 |
| --- | --- |
| （1）在 IDE 中采用多文件方式组织代码：类声明、类定义、对象生成在不同的文件里 | **类声明**在头文件（`src\Core\*.h`、`src\Model\*.h`、`src\Service\Library.h`）；**成员函数定义**在同名 `.cpp`（`Date.cpp`、`Person.cpp`、`Student.cpp`、`Teacher.cpp`、`Book.cpp`、`BorrowRecord.cpp`、`Library.cpp`）；**对象生成**在 `src\UI\main.cpp` 与 `src\Demo\ClassDemo.cpp`。头文件用 `#pragma once` 防止重复包含，`.vcxproj` 中已按 6 个功能层配置筛选器与头文件搜索路径，在 Visual Studio 解决方案资源管理器里可直接看到分层结构 |
| （2）完成代码且无编译错误，执行结果正确 | 已用两套编译器实测：Visual Studio 2022（MSVC 14.44，`/W4`）Debug/Release 与 MinGW g++ 14.2（`-Wall -Wextra -Wpedantic`）**均为 0 错误 0 警告**；`--selftest` 的 130 项断言全部通过（退出码 0）；`--demo` 与菜单冒烟测试输出正确，无失败操作 |
