# -*- coding: utf-8 -*-
import sys
sys.stdout.reconfigure(encoding='utf-8')

path_backup = r'E:\wosens\Programs\VisualC++\MyProgram\QuickTool\QuickToolDlg - 副本.cpp'
path_main = r'E:\wosens\Programs\VisualC++\MyProgram\QuickTool\QuickToolDlg.cpp'

with open(path_backup, 'rb') as f:
    data = f.read()

# Detect line ending
use_crlf = b'\r\n' in data[:500]
lines = data.split(b'\n')
print(f'Backup lines: {len(lines)}, CRLF: {use_crlf}')

# Build fixes as {line_num(1-based): utf8_str}
fixes = {}

# Line 2
fixes[2] = '// QuickToolDlg.cpp : 实现文件'
# Line 21
fixes[21] = '// CAboutDlg 对话框用于应用程序"关于"菜单项'
# Line 27
fixes[27] = '// 对话框数据'
# Line 33
fixes[33] = '    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 数据交换'
# Line 35
fixes[35] = '// 实现'
# Line 53
fixes[53] = '    // 读取帮助信息并且显示在文本框中'
# Line 63
fixes[63] = '    // 异常: OCX 属性页应返回 FALSE'
# Line 88
fixes[88] = '// CQuickToolDlg 对话框'
# Line 101
fixes[101] = '    // 当配置的文件夹为空时，需要新增一个本地文件夹，并将其加入到文件夹列表 2023.4.26'
# Line 133
fixes[133] = '    // 删除托盘图标'
# Line 197
fixes[197] = '    // IDM_ABOUTBOX 系统命令在应用程序菜单的范围内。'
# Line 216
fixes[216] = '    // 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动'
# Line 217
fixes[217] = '    //  执行此操作'
# Line 218
fixes[218] = '    SetIcon(m_hIcon, TRUE);\t\t\t// 设置大图标'
# Line 219
fixes[219] = '    SetIcon(m_hIcon, FALSE);\t\t// 设置小图标'
# Line 233
fixes[233] = '    return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE'
# Line 249
fixes[249] = '// 如果向对话框添加最小化按钮，则需要下面的代码'
# Line 250
fixes[250] = '//  来绘制图标。对于使用文档/视图模型的 MFC 应用程序，'
# Line 251
fixes[251] = '//  这将由框架自动完成。'
# Line 257
fixes[257] = '        CPaintDC dc(this); // 用于绘制的设备上下文'
# Line 261
fixes[261] = '        // 使图标在工作区中居中'
# Line 269
fixes[269] = '        // 绘制图标'
# Line 278
fixes[278] = '//当主对话框是最小化时系统调用此函数获取光标'
# Line 279
fixes[279] = '//所以返回'
# Line 330
fixes[330] = '    CBrowseForFolder dlg(this->GetSafeHwnd(),_T("请选择目录"),_T(""));'
# Line 372
fixes[372] = '    // 将此功能添加到 Windows 2000 或更高版本中。'
# Line 373
fixes[373] = '    // 版本 _WIN32_WINNT 和 WINVER 的值必须 >= 0x0500。'
# Line 374
fixes[374] = '    // TODO: 在此处添加消息处理程序代码'
# Line 379
fixes[379] = '    // 新建节点'
# Line 385
fixes[385] = '    tvBoot.hInsertAfter=m_tvwFolder.InsertItem(&tvBoot);                      // 新建节点'
# Line 393
fixes[393] = '// 加载文件夹的所有文件 2022.3.17'
# Line 488
fixes[488] = '// 运行程序并且等待返回 2021.9.3'
# Line 501
fixes[501] = '//         printf(_T("HaoZipC 程序启动失败,确认%s是存在的可执行程序。"),a_sProgram));'
# Line 510
fixes[510] = '函数功能：创建一个快捷方式文件到指定路径'
# Line 511
fixes[511] = '函数2参数：'
# Line 512
fixes[512] = 'lpszFileName    目标文件，aNULL表示当前进程的EXE文件。'
# Line 513
fixes[513] = 'lpszLnkFileDir  快捷方式目录，可为NULL。'
# Line 514
fixes[514] = 'lpszLnkFileName 快捷方式名，aNULL表示EXE文件名。'
# Line 515
fixes[515] = 'wHotkey         为0表示不设置快捷键'
# Line 516
fixes[516] = 'pszDescription  备注'
# Line 517
fixes[517] = 'iShowCmd        显示方式，默认为3，最大化窗口'
# Line 525
fixes[525] = '    IShellLink     *pLink;  //IShellLink接口指针  '
# Line 526
fixes[526] = '    IPersistFile   *ppf; //IPersisFil接口指针  '
# Line 528
fixes[528] = '    //创建IShellLink接口  '
# Line 533
fixes[533] = '    //从IShellLink接口得到IPersistFile接口  '
# Line 540
fixes[540] = '    //设置图标  '
# Line 543
fixes[543] = '    //1设置工作目录  '
# Line 547
fixes[547] = '    //设置快捷键  '
# Line 551
fixes[551] = '    //设置备注  '
# Line 555
fixes[555] = '    //设置显示方式  '
# Line 559
fixes[559] = '    //快捷方式的目标路径 + 参数  '
# Line 561
fixes[561] = '//     if (lpszLnkFileName != NULL) //如果指定了快捷方式名  '
# Line 564
fixes[564] = '//         //从第二个参数取路径名作为快捷方式的目标文件  '
# Line 576
fixes[576] = '//         //保存为*.lnk  '
# Line 584
fixes[584] = '    //保存快捷方式的目标路径  '
# Line 585
fixes[585] = '//     WCHAR  wsz[MAX_PATH];  //调用Unicode版本  '
# Line 604
fixes[604] = '// 获取快捷方式文件的地址 2022.3.18'
# Line 607
fixes[607] = '    IShellLink     *pLink;  //IShellLink接口指针  '
# Line 608
fixes[608] = '    IPersistFile   *ppf; //IPersisFil接口指针  '
# Line 610
fixes[610] = '    //创建IShellLink接口  '
# Line 615
fixes[615] = '    //从IShellLink接口得到IPersistFile接口  '
# Line 656
fixes[656] = '        // 改变当前目录'
# Line 679
fixes[679] = '    // 记录右键选择的节点 2024.3.4'
# Line 696
fixes[696] = '    // 右键打开菜单，允许在菜单选择"删除"选项'
# Line 698
fixes[698] = '    //GetCursorPos(&point);//获取鼠标的所在位置'
# Line 699
fixes[699] = '    //m_tvwFolder.ScreenToClient(&point);//转换为窗口坐标系'
# Line 702
fixes[702] = '    //CurrentItem=m_tvwFolder.HitTest(point,&uFlags);//获取当前的坐标的ITEM'
# Line 722
fixes[722] = '// 获取当前选择文件的完整路径 2023.4.25'
# Line 726
fixes[726] = '    m_tvwFolder.ScreenToClient(&m_pntOldPoint);//转换为窗口坐标系'
# Line 729
fixes[729] = '    //CurrentItem = m_tvwFolder.HitTest(m_pntOldPoint, &uFlags);//获取当前的坐标的ITEM'
# Line 739
fixes[739] = '// 删除快捷键'
# Line 743
fixes[743] = '        if (AfxMessageBox(_T("确定要删除该文件吗？"), MB_YESNO) == IDYES) {'
# Line 750
fixes[750] = '// 修改快捷键的文件名称'
# Line 759
fixes[759] = '        CInputBox dlg(_T("修改文件名称"),_T("修改"),&sInput);'
# Line 766
fixes[766] = '                    AfxMessageBox(_T("该文件已存在！"));'
# Line 778
fixes[778] = '// 根据文件路径创建快捷方式'
# Line 794
fixes[794] = '            // CString 转换为 LPCOLESTR'
# Line 804
fixes[804] = '// 获取第一个路径'
# Line 816
fixes[816] = '// 添加一个文件到快捷方式'
# Line 822
fixes[822] = '// 添加一个快捷键'
# Line 834
fixes[834] = '// 添加一个文件夹'
# Line 839
fixes[839] = '// 刷新'
# Line 846
fixes[846] = '    //取得被拖动文件的数目'
# Line 854
fixes[854] = '            MessageBox(_T("给文件名分配暂存空间时失败!"), _T("警告信息"), MB_ICONERROR);'
# Line 874
fixes[874] = '    // TODO: 在此添加控件通知处理程序代码'
# Line 878
fixes[878] = '// 打开快捷方式所在的文件夹'
# Line 889
fixes[889] = '// 以快捷方式打开并返回该快捷方式指向的文件名 2025.4.28'
# Line 898
fixes[898] = '            // CString 转换为 LPCOLESTR'

# Apply fixes
fixed = 0
for line_num, new_text in fixes.items():
    idx = line_num - 1
    if idx < len(lines):
        lines[idx] = new_text.encode('utf-8')
        fixed += 1

print(f'Applied {fixed} fixes out of {len(fixes)}')

# Rejoin with consistent line endings
if use_crlf:
    content = b'\r\n'.join(lines)
else:
    content = b'\n'.join(lines)

# Write with UTF-8 BOM
with open(path_main, 'wb') as f:
    f.write(b'\xef\xbb\xbf' + content)

print(f'Written to {path_main}')
print(f'Total lines: {len(lines)}')
