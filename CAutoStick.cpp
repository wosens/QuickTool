#include "stdafx.h"
#include "CAutoStick.h"


CStickWindow::CStickWindow(){
    m_bShink=false;
    m_bShinked=false;
    m_bMoveing=false;
    m_bFirst=true;
    m_hWnd  =NULL;
    m_szOrig=CSize(0,0);
}

/// <summary>
/// CStickWindow
/// </summary>
void CStickWindow::OnTimer(){
    if(!m_hWnd) return;
    // 窗口自动收缩
    if(m_bShink && !m_bShinked){
        int iScreenWidth=GetScreenWidth();
        RECT rTemp;
        CRect r;
        GetWindowRect(m_hWnd,&rTemp);
        r=rTemp;
        CRect rClient;
        GetClientRect(m_hWnd,&rTemp);
        rClient=rTemp;
        ClientToScreen(m_hWnd,&rClient.TopLeft());
        ClientToScreen(m_hWnd,&rClient.BottomRight());
        int iEdgeX=(r.Width()-rClient.Width())/2;
        POINT pntPoint;
        GetCursorPos(&pntPoint);
        if(!r.PtInRect(pntPoint)){
            CRect rNew=r;
            if(r.left<=0){
                rNew.left=0-iEdgeX;
                rNew.right=rNew.left+iEdgeX*2+10;
            }
            if(r.right>=iScreenWidth){
                rNew.right=iScreenWidth+iEdgeX;
                rNew.left=rNew.right-(iEdgeX*2+10);
            }
            m_bMoveing=true;
            MoveWindow(m_hWnd,rNew.left,rNew.top,rNew.Width(),rNew.Height(),TRUE);
            m_bMoveing=false;
            m_rCurr=rNew;
            m_bShinked=true;
            SetLayeredWindowAttributes(m_hWnd,RGB(0,0,0),100,LWA_ALPHA);
            ::SetWindowPos(m_hWnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
        }
    }
    // 恢复窗口显示
    if(m_bShink && m_bShinked){
        RECT rTemp;
        CRect r;
        GetWindowRect(m_hWnd,&rTemp);
        r=rTemp;
        POINT pntPoint;
        GetCursorPos(&pntPoint);
        if(r.PtInRect(pntPoint) || m_bFirst){
            CRect rNew(m_rOrig.left,m_rOrig.top,m_rOrig.left+m_szOrig.cx,m_rOrig.top+m_szOrig.cy);
            m_bMoveing=true;
            MoveWindow(m_hWnd,rNew.left,rNew.top,rNew.Width(),rNew.Height(),TRUE);
            m_bMoveing=false;
            m_rCurr=rNew;
            m_bShinked=false;
            SetLayeredWindowAttributes(m_hWnd,RGB(0,0,0),255,LWA_ALPHA);
            ::SetWindowPos(m_hWnd,HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
        }
        m_bFirst=false;
    }
}

void CStickWindow::OnMove(int a_iX,int a_iY){
    if(!m_hWnd) return;
    if(m_bMoveing) return;
    int iScreenWidth=GetScreenWidth();
    int iScreenHeight=GetScreenHeight();
    CRect r,rNew;
    ::GetWindowRect(m_hWnd,r);
    CRect rClient;
    ::GetClientRect(m_hWnd,rClient);
    ::ClientToScreen(m_hWnd,&rClient.TopLeft());
    ::ClientToScreen(m_hWnd,&rClient.BottomRight());
    int iEdgeX=(r.Width()-rClient.Width())/2;
    rNew=r;
    bool bShink=false;
    if(r.left<0){
        rNew.left=0-iEdgeX;
        rNew.right=m_szOrig.cx;
        bShink=true;
    }
    if(r.top<0){
        rNew.top=0;
        rNew.bottom=r.Height();
        bShink=true;
    }
    if(r.right>iScreenWidth){
        rNew.right=iScreenWidth+iEdgeX;
        rNew.left=rNew.right-m_szOrig.cx;
        bShink=true;
    }
    if(r.bottom>iScreenHeight){
        rNew.bottom=iScreenHeight;
        rNew.top=iScreenHeight-r.Height();
        bShink=true;
    }
    if(bShink){
        m_bMoveing=true;
        rNew.right=rNew.left+m_szOrig.cx;
        rNew.bottom=rNew.top+m_szOrig.cy;
        ::MoveWindow(m_hWnd,rNew.left,rNew.top,rNew.Width(),rNew.Height(),TRUE);
        m_bMoveing=false;
    } else{
    }
    m_bShink=bShink;
    m_rOrig=rNew;
}

void CStickWindow::OnSize(UINT nType,int cx,int cy){
    if(!m_hWnd) return;
    CRect r,rMain;
    GetClientRect(m_hWnd,&r);
    rMain.left=5;
    rMain.top=5;
    rMain.right=r.Width()-5;
    rMain.bottom=r.Height()-5;
    if(!m_bMoveing){
        GetWindowRect(m_hWnd,&r);
        m_szOrig.cx=r.Width();
        m_szOrig.cy=r.Height();
    }
    if(m_bMoveing || m_bShink) return ;
}

void CStickWindow::LoadConfig(CIniFile* a_pConfig){
    if(!m_hWnd) return;
    int iWindowMax=0;
    CString sGroupName=Format(_T("Window%dPos"),GetID());
    CRect r;
    r.left  =a_pConfig->GetInt(sGroupName,_T("Left"),0);
    r.top   =a_pConfig->GetInt(sGroupName,_T("Top"),0);
    r.right =r.left+a_pConfig->GetInt(sGroupName,_T("Width"),0);
    r.bottom=r.top+a_pConfig->GetInt(sGroupName,_T("Height"),0);
    iWindowMax=a_pConfig->GetInt(sGroupName,_T("WindowMax"),0);
    if(r.Width()>10 && r.Height()>10){
        m_szOrig=r.Size();
        m_bMoveing=true;
        ::MoveWindow(m_hWnd,r.left,r.top,r.Width(),r.Height(),FALSE);
        m_bMoveing=false;
        OnMove(0,0);
        if(iWindowMax>0){
            ::ShowWindow(m_hWnd,SW_SHOWMAXIMIZED);
        }
    }
    if(m_szOrig.cx==0 || m_szOrig.cy==0){
        GetWindowRect(m_hWnd,&r);
        m_szOrig=r.Size();
    }

    SetWindowLong(m_hWnd,GWL_EXSTYLE,GetWindowLong(m_hWnd,GWL_EXSTYLE)|WS_EX_LAYERED);
    ::SetLayeredWindowAttributes(m_hWnd,RGB(0,0,0),255,LWA_ALPHA);
    m_bShinked=(a_pConfig->GetInt(sGroupName,_T("Shink"),0)==1);
}

void CStickWindow::SaveConfig(CIniFile* a_pConfig){
    if(!m_hWnd) return;
    CRect r;
    ::GetWindowRect(m_hWnd,&r);
    if(!IsIconic(m_hWnd)){
        if(m_bShink && m_bShinked){
        } else{
        }
        CString sGroupName=Format(_T("Window%dPos"),GetID());
        a_pConfig->SetVarInt(sGroupName,_T("WindowMax"),0);   // 不保存最大化的状态，因为windows恢复的窗口大小和位置不正确 2018.12.28
        if(r.Width()>0 && r.Height()>0){
            a_pConfig->SetVarInt(sGroupName,_T("Left"),r.left);
            a_pConfig->SetVarInt(sGroupName,_T("Top"),r.top);
            if(m_bShink && m_bShinked){
                if(r.Width()<m_szOrig.cx) r.right =r.left+m_szOrig.cx;
                if(r.Height()<m_szOrig.cy) r.bottom=r.top+m_szOrig.cy;
            }
            a_pConfig->SetVarInt(sGroupName,_T("Width"),r.Width());
            a_pConfig->SetVarInt(sGroupName,_T("Height"),r.Height());
            a_pConfig->SetVarInt(sGroupName,_T("WindowMax"),0);
            a_pConfig->SetVarInt(sGroupName,_T("Shink"),m_bShink?1:0);
        }
    }
}

void CStickWindow::SetWindow(HWND a_hWnd){
    m_hWnd=a_hWnd;
}

CAutoStick::CAutoStick(){

}

CAutoStick::~CAutoStick(){

}

/// <summary>
/// CAutoStick
/// </summary>
/// <param name="a_pWindow"></param>

void CAutoStick::AddWindow(CStickWindow* a_pWindow){
    m_aWindows.AddItem(a_pWindow);
}

void CAutoStick::OnTimer(){
    for(int i=0;i<m_aWindows.GetItemCount();i++){
        CStickWindow* pWindow=(CStickWindow*)m_aWindows.GetValue(i);
        pWindow->OnTimer();
    }
}

void CAutoStick::OnMove(int a_iX,int a_iY){
    for(int i=0;i<m_aWindows.GetItemCount();i++){
        CStickWindow* pWindow=(CStickWindow*)m_aWindows.GetValue(i);
        pWindow->OnMove(a_iX,a_iY);
    }
}

void CAutoStick::OnSize(UINT nType,int cx,int cy){
    for(int i=0;i<m_aWindows.GetItemCount();i++){
        CStickWindow* pWindow=(CStickWindow*)m_aWindows.GetValue(i);
        pWindow->OnSize(nType,cx,cy);
    }
}

void CAutoStick::LoadConfig(CString a_sConfigFile){
    CIniFile iniFile;
    iniFile.Load(a_sConfigFile);
    for(int i=0;i<m_aWindows.GetItemCount();i++){
        CStickWindow* pWindow=(CStickWindow*)m_aWindows.GetValue(i);
        pWindow->LoadConfig(&iniFile);
    }
}

void CAutoStick::SaveConfig(CString a_sConfigFile){
    CIniFile iniFile;
    iniFile.Load(a_sConfigFile);
    //iniFile.SetVarInt(_T("General"),_T("WindowCount"),m_aWindows.GetItemCount());
    for(int i=0;i<m_aWindows.GetItemCount();i++){
        CStickWindow* pWindow=(CStickWindow*)m_aWindows.GetValue(i);
        pWindow->SaveConfig(&iniFile);
    }
    iniFile.Save();
}
