# ⚡ 闪现 QuickTool

![License](https://img.shields.io/badge/License-MIT-blue.svg)
![Platform](https://img.shields.io/badge/Platform-Windows-0078D6.svg)
![Language](https://img.shields.io/badge/C%2B%2B%20%7C%20MFC-00599C.svg)
![Version](https://img.shields.io/badge/Version-v1.0.44-blue.svg)

**QuickTool**（产品显示名「闪现」）是一款 Windows 桌面**快速启动工具**：把常用程序像文件一样"丢"进一棵目录树，常用软件一键直达；窗口贴到屏幕边缘即收缩隐身，鼠标轻碰把手立刻"闪现"。

A lightweight Windows quick-launcher built with C++/MFC — organize your favorite apps in a drag-and-drop directory tree, launch with a single click, and dock the window to the screen edge until you need it.

---

## ✨ 功能特性

- 🌲 **树形分类管理** —— 从资源管理器把 exe / 文件夹 / 快捷方式直接拖进主窗口，自动转换为快捷方式（`.lnk`）并按目录归类；单击节点即启动程序；右键支持删除 / 重命名 / 新建文件夹 / 打开文件位置（打开文件夹即打开它本身，打开文件则定位并选中该文件）/ 复制路径 / 复制文件名。
- ⚡ **高频图标区** —— 树控件左侧的常用程序坞：拖入自动转快捷方式存到 exe 同目录 `Icons\`（保持原图标、去掉"快捷方式"小箭头），单击即启动并把目标窗口强制置前（专治"进程在、窗口不弹"）；支持拖拽排序（序号持久化）、右键删除 / 打开目标位置 / 修改属性（系统快捷方式属性页）。
- 📌 **贴边闪现** —— 主窗口贴到屏幕左 / 右缘即收缩为一条 6 像素"边条把手"，不占桌面；鼠标靠近把手，主窗口无闪烁唤出（临时置前、不抢焦点，点别处自然让开；前台运行独占全屏程序时自动让位、退出全屏自动恢复）。
- 🌍 **双语 + 亮暗双主题** —— 简体中文 / English 界面一键切换，亮 / 暗主题即时换肤，选择持久化，下次启动直接生效。
- 🖌 **全自绘界面** —— 无系统标题栏的 borderless 窗口（自绘渐变标题条、四边四角拖拽缩放），自绘卡片式"关于"窗口（顶部圆角、滚轮滚动查看帮助全文）。
- 📦 **完全便携** —— 全部运行数据都在 exe 同目录（`Config.ini`、`QuickPos.ini`、`Icons\*.lnk`、`Help.txt` / `Help_en.txt`），不写注册表、不写 AppData，拷贝目录即完成安装 / 卸载。

## 🚀 使用方法

1. 编译得到 `QuickTool.exe`（见下节），放入任意目录运行（首次运行自动生成默认配置）；
2. 标题栏「功能 → 添加目录」添加你关心的文件夹，其内容以目录树展示（「管理目录」可增删维护）；
3. 从资源管理器把常用程序拖到树节点（自动建快捷方式归类），或拖到左侧图标区（作为高频入口）；
4. 单击即启动；把主窗口拖到屏幕左 / 右缘试试贴边收缩。

## 🛠️ 从源码构建

| 项 | 说明 |
|---|---|
| IDE | Visual Studio 2015 及以上，打开 `QuickTool.sln`（Win32 配置 = v140 工具集，Release\|x64 = v143，目标平台版本 8.1） |
| 语言 / 框架 | C++ / MFC（`CDialogEx` 对话框程序、Unicode、动态链接 MFC，运行需 Visual C++ 运行库） |
| 输出 | `Execute\QuickTool.exe`（Release 配置输出目录 = `Execute\`，与便携配置同目录） |
| 命令行 | `msbuild QuickTool.sln /p:Configuration=Release /p:Platform=x64` |

> ⚠️ **外部依赖（clone 后必看）**：本工程引用**不在本仓库内**的 Sun 标准类库（`SunBase.h`、`CIniFile`、`CItemList` 等实用类，作者：梅文海），`.vcxproj` 以相对路径 `..\..\..\..\Company\Project\40.三化\模块化\Sun Standard Class Library\` 引用其头文件与源码**直接编入 exe**。直接 clone 编译会报找不到头文件——需先取得该类库放到相同相对位置，或自行修改 `.vcxproj` 中的引用路径。

> 📝 **编码提示**：源码为 GBK 与 UTF-8（BOM）混合编码，资源文件为 UTF-16 LE；修改时请保持各文件原有编码，勿整文件转码。

## 📁 项目结构

```
QuickTool/
├── QuickTool.sln / QuickTool.vcxproj(.filters)   工程文件
├── QuickTool.cpp / .h                            应用入口 CQuickToolApp
├── QuickToolDlg.cpp / .h                         主对话框 + 树 / 图标区 / 拖放 / 关于窗（核心）
├── stdafx.h / .cpp                               预编译头、应用名「闪现」/ 版本常量、双语与主题框架
├── EdgeBarWnd.cpp / .h                           贴边收缩边条 CEdgeBarWnd
├── CAutoStick.cpp / .h                           窗口贴边与位置持久化 CAutoStick
├── DlgPathManage.cpp / .h                        目录管理对话框
├── DlgInputBox.cpp / .h                          通用输入框
├── TrayIcon.cpp / .h                             托盘图标封装（保留未启用）
├── BrowseForFolder.cpp / .h                      文件夹选择框封装
├── resource.h / QuickTool.rc                     资源 ID 与资源脚本
├── res\ / icon\                                  图标资源（icon\quicktool.ico 为应用图标）
├── readme.txt                                    版本号（首行）+ 完整改动日志
└── readme.md                                     本文件（GitHub 展示页）
```

## 📜 版本历史

完整改动日志见 [`readme.txt`](readme.txt)（首行为当前版本号，v 主.次.编译）。

## 📄 许可证

[MIT](LICENSE) © wosens
