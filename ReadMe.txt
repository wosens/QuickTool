版本：v1.0.43



2022.3.10
1.完成基本功能

2023.2.8
1.增加了目录管理功能，可以删除添加的目录


2023.7.7
1.隐藏任务栏图标

2024.3.4
1.解决右键打开文件位置经常无效的问题（是因为当鼠标移动到菜单的位置时，主窗口已经被隐藏，导致记录的鼠标位置失效）

2024.9.24
1.打开文件夹所在的路径和打开文件所在路径分开编写，规则是：
  打开文件夹就是真的打开该文件夹，打开文件就是打开该文件所在的文件夹，并且选中该文件
  之前的代码都是大概其所在的文件夹，然后选择该文件/文件夹

2025.4.28
1.增加”打开快捷键所指向的位置”的功能

2026.5.22
1.优化树控件刷新机制，支持节点级刷新，操作后仅刷新受影响的节点
2.刷新后自动保持用户当前选择，并恢复水平滚动条位置
3.重构OnPopmenuRename函数，合并文件/文件夹两个分支的重复逻辑

[2026-09-29] 修复 BrowseForFolder 缓冲/状态缺陷与 CAutoStick 贴边样式 XOR 缺陷 - BrowseForFolder.h BrowseForFolder.cpp CAutoStick.cpp
[2026-09-30] 版本号统一为 v1.0.9，同步标题栏与 VERSIONINFO - stdafx.h QuickTool.rc

2026.10.8
1.树控件拖动快捷方式靠近上/下边缘时自动滚动（CMyTreeCtrl::OnTimer 定时发 WM_VSCROLL），拖动悬停文件夹以选择状态高亮（TVIS_SELECTED） - QuickToolDlg.h QuickToolDlg.cpp

2026.10.20
1.新增图标显示区（树控件左侧，32x32 大图标，缺省宽度可显示两个图标），与树控件之间为 3 像素可拖动分隔条，宽度持久化到 Config.ini
2.图标区支持从资源管理器拖拽添加：非快捷方式自动转为快捷方式存放于 exe 同目录 Icons\，保持原有图标；Tooltip 显示原目标文件名与路径
3.图标区内部拖拽排序（按 001.lnk 序号文件名持久化顺序），双击运行，右键可删除（不删原文件）或打开目标位置
4.窗口吸附到屏幕右缘时布局镜像：图标区自动换到右侧，树控件换到左侧

2026.10.20
1.图标区图标改为 16x16（SHGFI_SMALLICON）
2.修复从资源管理器拖动快捷方式到图标区被误加到树控件的问题：图标区注册 OLE 拖放目标并加 WS_EX_ACCEPTFILES，新增 CMyIconListCtrl::OnDropFiles 兜底，两条路径均进 AddIconFromDrop

2026.10.20
1.图标区只显示图标不显示文字（名称/路径由 Tooltip 展示），LVM_SETICONSPACING 紧凑间距(24x24)，窄面板可显示多列

2026.10.20
1.图标区增加悬停高亮（WM_MOUSEMOVE/WM_MOUSELEAVE + NM_CUSTOMDRAW 浅蓝圆角背景）
2.修复点击下方图标误选中上方图标的问题：命中测试改为按图标矩形精确判定（IconHitTest），按下/双击/右键/拖动落点统一使用；列表应用 1pt 极小字体压缩空标签命中区

2026.10.20
1.修复图标区 Tooltip 字号太小看不清的问题：Tooltip 设为 11pt 系统消息字体并限宽 420 像素自动换行（ApplyTipFont）
2.图标单击即运行（抬起时判定，未构成拖动且仍命中同一图标才启动），拖动排序不再受影响，原双击处理移除
3.修复鼠标移出图标后悬停高亮残留的问题：悬停失效重绘改为带擦除，移出图标/离开控件时同时清除选中框

2026.10.21
1.修复图标单击无效的问题：原生 listview 在 OnLButtonDown 内部 DragDetect 会泵走原地抬起的 WM_LBUTTONUP，导致抬起判定收不到；改为 OnLButtonDown 不调基类、选中/焦点由 SetItemState 自行完成，单击启动消息链全通

2026.10.21
1.图标区右键菜单新增"修改属性"：调用系统快捷方式属性页（SHObjectProperties）可修改目标、起始位置与图标；关闭后重新提取该项图标刷新显示，旧图无引用时从图像列表移除

2026.10.21
1.修复单击已运行程序时"进程在、窗口不弹"的问题：ShellExecute 建的新进程把已有实例置前后退出，该切换常被系统前台锁拦截；启动后由后台线程（LaunchWatchThread）轮询目标进程主窗口，出现即组合置前（AttachThreadInput + SwitchToThisWindow + TOPMOST/NOTOPMOST 翻转），3 秒未见新进程则提升旧实例主窗口；首次启动场景不受影响

2026.10.21
1.窗口增加点击穿透自愈：应用自身从不使用 WS_EX_TRANSPARENT，若被外部软件加上该样式（表现为界面看得见点不着），OnTimer 周期检测并在 0.2 秒内自动摘除

2026.10.21
1.修复"修改属性"取消后图标错位/清空的问题：原实现点取消也无条件重取图标并 CImageList::Remove 删旧图——Remove 会把后续图标整体前移而 m_aIconImgs 索引不同步，导致全部图标错位、索引越界的项图标清空；改为对比属性页打开前后的 .lnk 修改时间，取消/未改动则完全不动列表，真修改则 LoadIconBar 整体重建（索引天然一致）

2026.10.21
1.图标区图标去掉左下角的"快捷方式"小箭头：SHGetFileInfo 直取 lnk 图标会被系统叠加箭头标识，改为解析 lnk 的图标位置（GetShortcutIconLocation：优先自定义"更改图标"，未设置则用目标文件），ExtractIconEx 按索引提取原图标；解析失败退回系统方式兜底

2026.10.21
1.重新设计"关于"窗口（高颜值自绘卡片）：去掉系统边框与模板控件，深蓝渐变头部（应用图标+产品名+副标题+Version 胶囊徽章+悬停变红的 × 关闭钮）、"使用说明"小节标题带下划短线、Help.txt 全文自绘排版、鼠标滚轮可滚动查看（带细滚动条）、底部圆角确定按钮；头部区域可拖动窗口，头部显示手型光标

2026.10.21
1.重新设计应用图标（icon\quicktool.ico）：深靛→亮蓝对角渐变圆角方块+白色闪电+顶部微高光，与关于窗口头部同视觉语言，含 256/64/48/32/24/16 六档
2.主窗口标题栏显示应用图标：主对话框样式加 WS_SYSMENU；因 WS_POPUP+工具窗口组合下系统标题栏不绘制图标，另在 OnNcPaint 自绘 16px 小图标与标题文字（MENUBARINFO 区分标题栏与菜单行定位，激活/非激活双色）
3.关于窗口正文字体改为 11pt（按钮同步 11pt，窗口加高至 640 容纳全文）
4.确认配置全部便携：Config.ini（目录/图标区宽度）、QuickPos.ini（窗口位置/收缩态）、Icons\*.lnk（图标区）均在 exe 同目录，全工程无注册表写入

2026.10.21
1.标题栏彻底自绘（修复系统标题栏不显示图标）：WM_NCCALCSIZE 去掉系统标题栏（SM_CYCAPTION 高度并入客户区），OnPaint 顶部 30px 画深蓝渐变标题条——16px 闪电图标+标题文字+功能/帮助/退出三个文字钮（替代不可用的系统菜单栏，点击弹原菜单资源）+悬停变红关闭钮；NCHITTEST 标题区返回 HTCAPTION 支持拖动，菜单钮/关闭钮按客户区点击；控件布局整体下移标题栏高度
2.关于窗口头部图标改用 LoadImage 按显示尺寸提取（原 LoadIcon 32px 缩到 40px 显示发虚），像素感明显改善

2026.10.21
1.修复窗口缩放时显示错乱：改为完全 borderless——rc 去掉 WS_THICKFRAME/WS_SYSMENU（无系统边框），WM_NCCALCSIZE 客户区=整个窗口矩形，系统 NC 与客户区双轨并存导致的 resize 抖动消失；窗口四边/四角 6 像素热区在 NCHITTEST 返回缩放命中（HTLEFT/HTRIGHT/HTTOP/HTBOTTOM/四角），贴边收缩态不启用；标题栏为唯一标题栏

2026.10.21
1.修复标题栏菜单项悬停"白底白字看不清"：悬停高亮从纯白圆角块改为亮蓝灰 RGB(88,110,168)（深蓝底上的标准高亮色），悬停时文字保持白色清晰

2026.10.21
1.修复屏幕中间残留"隐形窗口"：标题栏菜单 TrackPopupMenu 弹出前未 SetForegroundWindow，属主不对时菜单弹出不接收点击、点外部也不消失，一直挂在屏幕上（可误触运行）；弹出前置前台+补 TPM_RIGHTBUTTON，关闭后重新置前
2.修复点击 .bat 等脚本"窗口闪现后被挤下去"：LaunchWatchThread 的"旧实例提升"分支只对 .exe 目标生效（脚本/文档由 shell 代理打开，无单实例旧窗口，原逻辑会把无关同名进程拽到前台挤掉新开窗口）

2026.10.21
1.贴边收缩改为"独立边条窗口"方案（不再变形主窗口）：贴右缘时主窗口整体隐藏，另建 10x120 独立边条窗口（深蓝渐变把手+三个白点提示，WS_EX_TOPMOST 工具窗）贴屏幕右缘垂直居中；鼠标进入边条外扩 6 像素热区即唤出主窗口（贴边展开位置）并隐藏边条；新增 EdgeBarWnd.h/.cpp（入 vcxproj+filters），OnTimer 由 UpdateEdgeShrink 状态机接管，停用 CAutoStick 的变形/半透明逻辑（OnMove/OnSize/OnTimer 不再调 m_asStick），QuickPos.ini 位置持久化兼容保留

2026.10.21
1.边条改为与主窗口同高（OnShowWindow 位置恢复后按主窗口实际高重建），宽度 10→6 像素更纤细
2.修复唤出时主窗口闪烁：SetRedraw(FALSE)→原子定位+显示(SWP_NOREDRAW)→SetRedraw(TRUE)+RedrawWindow 一次成画，不再露出默认白底
3.标题栏去掉"退出"文字钮（× 关闭钮已承担退出），仅保留功能/帮助
4.标题栏增加 tooltip：悬停标题栏空白区显示"Quick Tool 版本 + 拖动移动窗口 · 拖边缘调整大小"提示

2026.10.21
1.修复"弹出主窗口不稳定"：唤出改为事件驱动——边条 WM_MOUSEMOVE 同步 SendMessage(WM_APP+301) 直接触发主窗口唤出（原依赖主窗口 200ms Timer 轮询 GetCursorPos，菜单弹出期间 KillTimer 等场景会停摆导致唤不灵）；Timer 轮询保留兜底；唤出双重保障（UpdateEdgeShrink 判定失败则强制贴边展开）；收缩改用 WM_MOUSELEAVE（唤出后注册，鼠标离开主窗口即缩回边条），贴边状态才收缩
2.修复唤出后立即被藏回（"显示不了"的另一半根因）：唤出保持判断原来只认主窗口矩形，鼠标停在边条热区（主窗口右缘外 6px 条带）时 PtInRect=FALSE，唤出后 200ms 内即被 Timer 藏回；保持区扩为主窗口矩形+上下 8px+右缘 12px（覆盖边条热区），鼠标真正离开右缘区域才收缩

2026.10.21
1.修复缩放时标题栏残影：OnSize 尺寸变化后 RedrawWindow 整窗同帧重画（含子控件）；新增 OnEraseBkgnd 自行擦除（先画客户区底色与标题栏渐变再上屏，不再走系统白底），拖拽缩放不再有旧尺寸残影与白闪
2.彻底禁用原始菜单栏：主对话框模板移除 MENU IDR_MENU1（标题栏功能/帮助按钮用 LoadMenu 弹出不受影响），系统不会再绘制原始菜单干扰新标题栏
3.标题文字区改为动态计算（图标右侧到最左菜单钮左侧，按当前窗口宽度自适应），宽窗口显示完整标题，窄窗口才出现省略号

2026.10.22
1.修复收缩边条把手 top 与主窗口不一致：边条显示时（ShowBar）按主窗口当前位置对齐（AlignToWindow：高度=主窗口高、top=主窗口 top），不再固定屏幕垂直居中
2.支持贴屏幕左缘收缩：贴边判定改为 GetStickEdge（-1=贴左缘 1=贴右缘），收缩/唤出/保持区判定按贴边侧取对应缘，边条把手自动贴到主窗口同侧缘（高亮线画朝屏幕内侧），唤出时主窗口展开到对应缘；拖到左缘同样触发收缩为边条 - EdgeBarWnd.h EdgeBarWnd.cpp QuickToolDlg.h QuickToolDlg.cpp

2026.10.22
1.鼠标移到门把手（边条）唤出主窗口时临时置前：唤出 SetWindowPos 由保持原 z 序改为 &wndTop+SWP_NOACTIVATE——主窗口升到普通层最前、不激活不抢焦点（不会被其它窗口挡住），点击其它窗口后自然被盖回（"临时"置前）
2.边条把手永久置顶 TOPMOST：新增 CEdgeBarWnd::EnsureTopmost 周期维护（主窗口 200ms Timer 驱动，仅状态与目标不符才动窗口，不闪烁）——TOPMOST 被系统/其它程序丢掉立即补回；前台出现 DX 独占全屏程序时（前台窗口矩形盖满主屏且非桌面/自身，IsForegroundFullScreen 检测）自动摘除 TOPMOST 让位，退出全屏后自动恢复；ShowBar 显示时同步重申置顶 - EdgeBarWnd.h EdgeBarWnd.cpp QuickToolDlg.cpp
3.修复 QuickToolDlg.cpp/.h、stdafx.h 丢失 UTF-8 BOM 导致编译失败（外部编辑所致；字节级补回 BOM，内容未动）

[2026-10-22] 实现"选择语言/选择主题"菜单功能（原为观测桩）：语言即时生效——标题栏功能/帮助钮与 tooltip、主菜单、树/图标区右键菜单、输入框标题与按钮、路径管理窗、关于窗副标题/节标题/按钮、全部消息框按双语文本表 QtLS 取词，英文下重测标题栏按钮宽度并整窗重绘；主题即时换肤——标题栏渐变、客户区底色、树控件（暗主题自绘深底浅字+图标底色随动）、图标区背景与悬停高亮、分隔条、关于窗、收缩边条把手随主题色表取色（亮=原配色，暗=深灰蓝）；语言/主题子菜单 CheckMenuRadioItem 单选圆点显示当前选择；选择持久化到 Config.ini [General] Lang/Theme，启动时按配置直接生效；同步版本号 v1.0.39 - stdafx.h QuickToolDlg.h QuickToolDlg.cpp DlgInputBox.cpp DlgPathManage.cpp EdgeBarWnd.cpp QuickTool.rc readme.txt

[2026-10-22] 英文模式软件名与关于页全英文化：新增 QtAppTitle()（英文显示 QuickTool，中文显示 闪现），主窗标题改在 LoadConfig 读取语言之后设置、ApplyLanguage 切换时即时刷新（修自绘标题栏英文下仍显中文），标题栏 tooltip/关于窗产品名随语言；关于窗帮助文本随语言——英文读 Help_en.txt（新增于 Execute，UTF-8 与 Help.txt 同法），缺失时用内置英文兜底，中文仍读 Help.txt；关于窗版本徽章前缀 版本/Version 随语言（双语文本表新增 QT_TXT_ABOUT_VER）；同步版本号 v1.0.40 - stdafx.h QuickToolDlg.cpp QuickTool.rc Execute\Help_en.txt readme.txt

[2026-10-23] 修"鼠标移到门把手窗口出不来"：运行中实例主窗口被外部自动化工具注入 WS_EX_LAYERED(alpha=1)+WS_EX_NOACTIVATE+WS_EX_TRANSPARENT（实测 GetLayeredWindowAttributes 返回 alpha=1，唤出逻辑本身正常执行、主窗口实际已显示但肉眼全透明，随后被判"鼠标离开"收回，表象即"把手在、窗口永远不出来"）；外部清位治愈当前实例后加固代码——OnTimer 自愈掩码由仅 WS_EX_TRANSPARENT 扩为 WS_EX_TRANSPARENT|WS_EX_LAYERED|WS_EX_NOACTIVATE（工程自身从不使用这三者），并改为无条件摘除（注入可发生在收缩隐藏期间，原 IsWindowVisible 条件会漏）+ OnEdgeBarNotify 唤出入口先摘再唤出，保证唤出即刻可见可交互；同步版本号 v1.0.42 - QuickToolDlg.cpp stdafx.h QuickTool.rc readme.txt
[2026-10-23] 关于窗顶部两角圆角化：模板 DS_MODALFRAME 残留的 WS_EX_DLGMODALFRAME 直角外框包着 12px 圆角自绘卡片，观感突兀——OnInitDialog 新增 SetWindowRgn 窗口圆角外形，顶部两角与 OnPaint 卡片圆角裁剪同半径贴合（半径抽为共用常量 s_iAboutRound，两处永不脱节；区域高度外扩一个半径，底部两角保持直角外框不变）；同步版本号 v1.0.43 - QuickToolDlg.cpp stdafx.h QuickTool.rc readme.txt
