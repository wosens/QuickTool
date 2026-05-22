# QuickTool Project Coding Rules

## Naming Conventions (Hungarian Notation + CamelCase)

- **Local variables**: Type prefix (lowercase) + PascalCase
  - `int iAge`, `string sName`, `bool bEnabled`, `float fScore`, `double dValue`
  - `HWND hWnd`, `DWORD dwFlags`, `LPCTSTR lpszText`, `BYTE byData`
- **Global variables**: `g_` prefix + Hungarian prefix
  - `int g_iInstanceCount`, `string g_sAppName`
- **Class member variables**: `m_` prefix + Hungarian prefix
  - `int m_iAge`, `string m_sName`
- **Function parameters**: `a_` prefix + Hungarian prefix
  - `int a_iCount`, `string a_sFileName`
- **Static member variables**: `s_` prefix + Hungarian prefix
  - `int s_iRefCount`, `string s_sClassName`
- **Arrays**: `a` prefix (lowercase) + PascalCase
  - `vector<int> aItemList`, `int aAge[10]`
- **Class names**: `C` prefix + PascalCase
  - `class CQuickToolDlg`, `class CMyDocument`

## Format

- **Brace style (K&R)**: Opening brace on the same line as the control statement
  ```cpp
  if (condition) {
      DoSomething();
  } else {
      DoOther();
  }

  void CMyClass::MyMethod() {
      // body
  }
  ```
