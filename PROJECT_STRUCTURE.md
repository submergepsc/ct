# 项目结构说明

本文档介绍算术表达式求值器项目的目录和核心文件结构，帮助快速了解各模块的职责。

## 顶层目录

```
.
├── LICENSE
├── PROJECT_STRUCTURE.md
├── README.md
├── project.pro
├── resources.qrc
└── src/
```

- **LICENSE**：MIT 许可证文本。
- **README.md**：项目概述、构建方式与使用说明。
- **project.pro**：qmake 工程文件，定义编译配置、源文件及资源文件。
- **resources.qrc**：Qt 资源文件，目前用于在构建系统中占位，可扩展用于打包图标或其它资源。
- **src/**：存放应用的所有源代码及界面描述文件。

## `src/` 目录

```
src/
├── expressionevaluator.cpp
├── expressionevaluator.h
├── main.cpp
├── mainwindow.cpp
├── mainwindow.h
└── mainwindow.ui
```

- **expressionevaluator.h / expressionevaluator.cpp**：实现表达式解析与求值的核心逻辑。包括：
  - 词法分析器，将输入拆分为 token 序列；
  - 使用递归下降算法构建带有运算符优先级的表达式语法树；
  - 对变量赋值并最终计算表达式结果；
  - 将语法树转换为 `QStandardItemModel` 结构，供界面中的树形视图展示。
- **mainwindow.h / mainwindow.cpp / mainwindow.ui**：Qt 主窗口界面，负责用户交互与结果展示。主要功能：
  - 读取表达式文本及变量赋值，触发解析求值；
  - 将 `ExpressionEvaluator` 返回的树结构绑定到 `QTreeView`；
  - 处理文件打开、输入校验、错误提示等界面逻辑；
  - 在 `.ui` 文件中定义界面布局（文本框、按钮、树视图等），由 Qt Designer 生成。
- **main.cpp**：应用入口，创建 `QApplication` 并显示 `MainWindow`。

## 构建流程概览

1. 使用 `qmake` 解析 `project.pro`，生成 Makefile。
2. 通过 `make` 编译 `src/` 中的源文件，并链接生成可执行程序 `expression_evaluator`。
3. 在运行时，主窗口调用 `ExpressionEvaluator` 完成表达式解析、求值与树形结构展示。

如需扩展功能（例如新增运算符、国际化文本或添加图标资源），可分别在 `ExpressionEvaluator`、`mainwindow` 相关文件与 `resources.qrc` 中进行调整。
