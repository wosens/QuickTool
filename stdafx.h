
// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // 从 Windows 头中排除极少使用的资料
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // 某些 CString 构造函数将是显式的

// 关闭 MFC 对某些常见但经常可放心忽略的警告消息的隐藏
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC 核心组件和标准组件
#include <afxext.h>         // MFC 扩展


#include <afxdisp.h>        // MFC 自动化类



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC 对 Internet Explorer 4 公共控件的支持
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC 对 Windows 公共控件的支持
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // 功能区和控件条的 MFC 支持




#include "SunBase.h"


typedef list<CString> CListString;


const CString DEF_APPLICATION_TITLE =_T("闪现");      // 软件名中文来源；英文模式经 QtAppTitle() 显示"QuickTool" 2026.10.22
const CString DEF_APPLICATION_VER   =_T("v1.0.43");   // 保留 v 前缀，AboutVersionBadge 已 TrimLeft("vV") 适配 2026.10.22
CString QtAppTitle();  // 按当前语言返回软件名：英文"QuickTool"/中文"闪现"（主窗标题/标题栏 tooltip/关于窗共用）2026.10.22


// ==================== 界面语言/主题运行时状态 2026.10.22 ====================
// 状态定义与双语文本表见 QuickToolDlg.cpp；持久化于 exe 同目录 Config.ini [General] Lang/Theme
enum QT_LANG  { QT_LANG_ZH = 0, QT_LANG_EN = 1 };         // 界面语言（缺省简体中文）
enum QT_THEME { QT_THEME_LIGHT = 0, QT_THEME_DARK = 1 };  // 界面主题（缺省亮）
extern QT_LANG  g_eQtLang;
extern QT_THEME g_eQtTheme;

// 双语文本表索引（中/英文案见 QuickToolDlg.cpp 的 g_aQtTxtZh/g_aQtTxtEn，经 QtLS 取词）
enum QT_TXT {
    QT_TXT_CAP_MENU = 0,      // 标题栏"功能"钮
    QT_TXT_CAP_HELP,          // 标题栏"帮助"钮
    QT_TXT_CAP_TIP,           // 标题栏 tooltip 操作提示行
    QT_TXT_MNU_PATH_MGR,      // 菜单：管理目录
    QT_TXT_MNU_LANG,          // 菜单：选择语言（弹出项标题）
    QT_TXT_MNU_LANG_EN,       // 菜单：English
    QT_TXT_MNU_LANG_ZH,       // 菜单：简体中文
    QT_TXT_MNU_THEME,         // 菜单：选择主题（弹出项标题）
    QT_TXT_MNU_THEME_LIGHT,   // 菜单：亮主题
    QT_TXT_MNU_THEME_DARK,    // 菜单：暗主题
    QT_TXT_MNU_ABOUT,         // 菜单：关于
    QT_TXT_MNU_EXIT,          // 菜单：退出
    QT_TXT_POP_ADD_FILE,      // 树右键：添加文件
    QT_TXT_POP_ADD_FOLDER,    // 树右键：添加文件夹
    QT_TXT_POP_CREATE_FOLDER, // 树右键：创建文件夹
    QT_TXT_POP_DELETE,        // 树右键：删除
    QT_TXT_POP_RENAME,        // 树右键：改名
    QT_TXT_POP_COPY_PATH,     // 树右键：复制路径
    QT_TXT_POP_COPY_NAME,     // 树右键：复制文件名
    QT_TXT_POP_REFRESH,       // 树右键：刷新
    QT_TXT_POP_OPEN_LOCATION, // 树右键：打开文件所在位置
    QT_TXT_POP_OPEN_SHORTCUT, // 树右键：打开快捷键所指向的位置
    QT_TXT_ICON_OPEN,         // 图标区右键：打开目标位置
    QT_TXT_ICON_PROPERTY,     // 图标区右键：修改属性
    QT_TXT_ICON_DELETE,       // 图标区右键：从图标区删除
    QT_TXT_MSG_DEL_ICON,      // 确认框：从图标区删除 %s（含 %s 占位）
    QT_TXT_MSG_DEL_FILE,      // 确认框：删除 %s（含 %s 占位）
    QT_TXT_MSG_PROP_FAIL,     // 打开属性对话框失败
    QT_TXT_MSG_FILE_EXISTS,   // 该文件已存在
    QT_TXT_MSG_DIR_EXISTS,    // 该文件夹已存在
    QT_TXT_MSG_MKDIR_FAIL,    // 创建文件夹失败
    QT_TXT_MSG_ALLOC_FAIL,    // 分配暂存空间出错
    QT_TXT_MSG_ERR_TITLE,     // 消息框标题：错误信息
    QT_TXT_MSG_SAME_NAME,     // 目标文件夹已存在同名文件
    QT_TXT_MSG_MOVE_FAIL,     // 移动文件失败
    QT_TXT_MSG_DEL_PATHS,     // 路径管理：真的要删除吗
    QT_TXT_INP_RENAME_FILE,   // 输入框：修改文件名称（标题标签）
    QT_TXT_INP_RENAME_DIR,    // 输入框：修改文件夹名称（标题标签）
    QT_TXT_INP_MODIFY,        // 输入框：修改（窗口标题）
    QT_TXT_INP_NEW_DIR,       // 输入框：创建文件夹（标题标签）
    QT_TXT_INP_DIR_NAME,      // 输入框：文件夹名称（窗口标题）
    QT_TXT_SELECT_DIR,        // 浏览对话框标题：请选择目录
    QT_TXT_ABOUT_SUBTITLE,    // 关于窗副标题
    QT_TXT_ABOUT_VER,         // 关于窗版本徽章前缀（版本/Version）2026.10.22
    QT_TXT_ABOUT_SECTION,     // 关于窗"使用说明"节标题
    QT_TXT_ABOUT_OK,          // 关于窗"确定"按钮
    QT_TXT_ABOUT_NO_HELP,     // 关于窗：未找到 Help.txt
    QT_TXT_BTN_OK,            // 通用按钮：确定
    QT_TXT_BTN_CANCEL,        // 通用按钮：取消
    QT_TXT_BTN_DELETE,        // 通用按钮：删除
    QT_TXT_DLG_PATH_MGR,      // 路径管理窗标题
    QT_TXT_COUNT
};
LPCTSTR QtLS(QT_TXT a_eIdx);  // 按当前语言（g_eQtLang）取界面文本，越界兜底中文
void QtGradientH(CDC* a_pDC, const CRect& a_rc, COLORREF a_cr1, COLORREF a_cr2);   // 水平渐变填充（标题栏/关于窗头部/收缩边条共用）



#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif


