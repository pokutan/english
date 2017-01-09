#include "stdafx.h"

#include "EnglishTraining.h"
#include "EnglishTrainingDlg.h"
#include "afxdialogex.h"
#include "SysToolsDyn.h"
#include "KDWinInet.h"

// TODO: these would be removed
#include "UT_String.h"
#pragma comment(lib, "SysTools")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

OPTIONS::OPTIONS() {
    wchar_t opt_file[MAX_PATH]={};
    SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, SHGFP_TYPE_CURRENT, opt_file);
    wcscat_s(opt_file, MAX_PATH, OPT_FILENAME);
    OutputDebugStringW(opt_file);
    _cfg_file = CreateFileW(opt_file, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    DWORD d = sizeof(_static_data);
    if(_cfg_file != INVALID_HANDLE_VALUE && GetLastError() == ERROR_ALREADY_EXISTS)
        if(ReadFile(_cfg_file,&_static_data,d,&d,NULL))
            SetFilePointer(_cfg_file,0,NULL,FILE_BEGIN);
#ifdef _DEBUG
    _timeouts.insert(std::pair<int,wstring>(1*ONE_SEC, L"1"));
#endif
    _timeouts.insert(std::pair<int,wstring>(5*ONE_SEC, L"5"));
    _timeouts.insert(std::pair<int,wstring>(10*ONE_SEC, L"10"));
    _timeouts.insert(std::pair<int,wstring>(30*ONE_SEC, L"30"));
    _timeouts.insert(std::pair<int,wstring>(60*ONE_SEC, L"60"));
    _timeouts.insert(std::pair<int,wstring>(120*ONE_SEC, L"120"));
    _timeouts.insert(std::pair<int,wstring>(180*ONE_SEC, L"180"));
    _timeouts.insert(std::pair<int,wstring>(300*ONE_SEC, L"300"));
}

OPTIONS::~OPTIONS(){
    if(_cfg_file != INVALID_HANDLE_VALUE){
        DWORD d = sizeof(_static_data);
        WriteFile(_cfg_file, &_static_data, sizeof(_static_data), &d, NULL);
        CloseHandle(_cfg_file);
    }
}

void OPTIONS::set_show_timeout(wchar_t const* to_){
    if(!to_[0])
        return;
    for(auto it : _timeouts)
        if(it.second == to_)
            _static_data._to = it.first;
}

CEnglishTrainingDlg::CEnglishTrainingDlg(CWnd* pParent /*=NULL*/) : CDialogEx(CEnglishTrainingDlg::IDD,pParent){
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    _rnd = new gen_random<int>;
//    _urls[url_vocab] = "http://www.classes.ru/dictionary-english-russian-Apresyan.htm";
    _urls[url_vocab] = "http://www.classes.ru/dictionary-english-russian-Mueller.htm";
    _urls[url_prononce] = "http://howjsay.com/pronunciation-of-";
    _urls[url_webster] = "http://www.merriam-webster.com/dictionary/";
    _urls[url_examples] = "http://www.macmillandictionary.com/dictionary/british/";
    _urls[url_synonym] = "http://www.thesaurus.com/browse/";
}

void CEnglishTrainingDlg::DoDataExchange(CDataExchange* pDX){
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX,ID_STAT_SORCE_WORD,SourceWord);
    DDX_Control(pDX,ID_COMBO_TRANSLATE,Translations);
    DDX_Control(pDX,ID_STAT_RES,Stat_Result);
    DDX_Control(pDX,ID_COMBO_TO,ComboTO);
    DDX_Control(pDX,ID_STAT_PREV,PrevTranslation);
    DDX_Control(pDX,IDC_RADIO1,RadioLearn);
    DDX_Control(pDX,IDC_RADIO2,RadioChoose);
    DDX_Control(pDX,ID_CHECK_FROM_ENG,CheckTranslateFromEng);
    DDX_Control(pDX,ID_CHECK_ONTOP,CheckOnTop);
}

BEGIN_MESSAGE_MAP(CEnglishTrainingDlg,CDialogEx)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_WM_TIMER()
    ON_BN_CLICKED(ID_BTN_SUBMIT,&CEnglishTrainingDlg::OnBnClickedBtnSubmit)
    ON_CBN_SELCHANGE(ID_COMBO_TO,&CEnglishTrainingDlg::OnCbnSelchangeComboTo)
    ON_BN_CLICKED(ID_BTN_EDITFILE,&CEnglishTrainingDlg::OnBnClickedBtnEditfile)
    ON_BN_CLICKED(ID_BTN_RELOAD,&CEnglishTrainingDlg::OnBnClickedBtnReload)
    ON_BN_CLICKED(IDC_RADIO2,&CEnglishTrainingDlg::OnBnClickedRadioChoose)
    ON_BN_CLICKED(IDC_RADIO1,&CEnglishTrainingDlg::OnBnClickedRadioLearn)
    ON_BN_CLICKED(ID_BTN_PRONON,&CEnglishTrainingDlg::OnBnClickedBtnPrononce)
    ON_BN_CLICKED(ID_BTN_EXAMP,&CEnglishTrainingDlg::OnBnClickedBtnExamp)
    ON_BN_CLICKED(ID_BTN_HELP,&CEnglishTrainingDlg::OnBnClickedBtnHelp)
    ON_BN_CLICKED(ID_BTN_SET_FILE,&CEnglishTrainingDlg::OnBnClickedBtnSetFile)
    ON_WM_DESTROY()
    ON_BN_CLICKED(ID_BTN_DICT,&CEnglishTrainingDlg::OnBnClickedBtnDict)
    ON_BN_CLICKED(ID_CHECK_FROM_ENG,&CEnglishTrainingDlg::OnBnClickedCheckFromEngToRus)
    ON_STN_DBLCLK(ID_STAT_PREV,&CEnglishTrainingDlg::OnDblclkStatPrev)
    ON_STN_DBLCLK(ID_STAT_SORCE_WORD,&CEnglishTrainingDlg::OnStnDblclickStatSorceWord)
    ON_WM_ACTIVATE()
    ON_STN_DBLCLK(ID_STAT_RES,&CEnglishTrainingDlg::OnStnDblclickStatRes)
    ON_BN_CLICKED(ID_BTN_CLEAR_WORD,&CEnglishTrainingDlg::OnBnClickedBtnClearWord)
    ON_BN_CLICKED(ID_CHECK_ONTOP,&CEnglishTrainingDlg::OnBnClickedCheckOntop)
    ON_BN_CLICKED(ID_BTN_PLUS_WORD,&CEnglishTrainingDlg::OnBnClickedBtnPlusWord)
    ON_BN_CLICKED(ID_BTN_SYNS,&CEnglishTrainingDlg::OnBnClickedBtnSyns)
END_MESSAGE_MAP()

void CEnglishTrainingDlg::fill_combo(int rus_){
    for(int i = Translations.GetCount() - 1; i >= 0; i--)
        Translations.DeleteString(i);
    for(MAP_IT it = _words_map.begin(); it != _words_map.end(); ++it)
        Translations.AddString(rus_ ? it->first.c_str() : it->second.c_str());
    Translations.SetCurSel(-1);
    Translations.SetDroppedWidth(!rus_ ? 500 : 100);
}

void CEnglishTrainingDlg::fill_ui_data(_In_ bool update_prev_){
    if(!_words_map.size()){
        Stat_Result.SetWindowTextW(L"Error loading words");
        SetWindowTextA(m_hWnd, _caption.c_str());
        return;
    }
    if(_opt._static_data._vocab_from_rus2eng)
        CheckTranslateFromEng.SetCheck(BST_CHECKED);
    static gen_random<int> gr(0,2);
    wstring s;
    if(_mode_learn)
        _rus2eng_learn = gr;
//    _rus2eng_learn = 1;
    fill_combo(_mode_learn ? !_rus2eng_learn : _opt._static_data._vocab_from_rus2eng);
    if(_mode_learn){
        bool base_map = true;
        if(_most_active_words_map.size()){
            static int cnt = 0;
            if(++cnt == 5){
                cnt = 0;
                base_map = false;
            }
        }
        MAP_IT it = get_random_pair(base_map);
        SourceWord.SetWindowText(_rus2eng_learn ? it->first.c_str() : it->second.c_str());
        _curr_right_transl = !_rus2eng_learn ? it->first : it->second;
        Stat_Result.SetWindowTextW(L"Choose Translation");
        if(update_prev_ && _curr_pair.first.length()){
            s = _curr_pair.first;
            s += L" - ";
            s += _curr_pair.second;
            PrevTranslation.SetWindowTextW(s.c_str());
        }
        _curr_pair = *it;
    }else
        _curr_pair = MAP_PAIR(L"", L"");
    ActivateKeyboardLayout(_mode_learn ? (_rus2eng_learn ? _rus_kbd : _eng_kbd) : (!_opt._static_data._vocab_from_rus2eng ? _rus_kbd : _eng_kbd), KLF_SETFORPROCESS);
    string ss = _caption;
    ss += " - ";
    ss += _source_file;
    SetWindowTextA(m_hWnd, ss.c_str());
}

void CEnglishTrainingDlg::fill_to_combo(){
    int curr_to_idx = 0;
    bool to_set = false;
    for(std::map<int, wstring>::iterator i = _opt._timeouts.begin(); i != _opt._timeouts.end(); ++i){
        ComboTO.AddString(i->second.c_str());
        if(i->first == _opt.to())
            to_set = true;
        if(!to_set)
            ++curr_to_idx;
    }
    ComboTO.SetCurSel(curr_to_idx);
}

BOOL CEnglishTrainingDlg::OnInitDialog(){
    CDialogEx::OnInitDialog();
    SetIcon(m_hIcon,TRUE);
    SetIcon(m_hIcon,FALSE);
    _caption = "English Vocabulay by pokutan";
    // https://msdn.microsoft.com/en-us/library/dd318693.aspx
    _eng_kbd = LoadKeyboardLayoutW(L"00000409",KLF_SETFORPROCESS|KLF_ACTIVATE);
    _rus_kbd = LoadKeyboardLayoutW(L"00000419",KLF_SETFORPROCESS|KLF_ACTIVATE);
    DWORD d = MAX_PATH;
    GetComputerNameW(_comp_name,&d);
    if(!_wcsicmp(_comp_name,L"pokutan")){
        strcpy_s(_source_file,MAX_PATH,"F:\\Dropbox\\eng\\my_app\\words.txt");
        if(!IsFileExists(_source_file))
            strcpy_s(_source_file,MAX_PATH,"f:\\Dropbox\\eng\\my_app\\words.txt");
    } else if(!_wcsicmp(_comp_name,L"IVANZ1-WS"))
        strcpy_s(_source_file,MAX_PATH,IsFileExists("\\\\ivanz-tp\\eng_dropbox\\my_app\\words.txt") ? "\\\\ivanz-tp\\eng_dropbox\\my_app\\words.txt" : "C:\\dev_my\\EnglishTraining\\Release\\words.txt");
    else if(!_wcsicmp(_comp_name,L"IVANZ-TP"))
        strcpy_s(_source_file,MAX_PATH,"C:\\Private\\Dropbox\\eng\\my_app\\words.txt");
    if(!IsFileExists(_source_file)){
        ::GetCurrentDirectoryA(MAX_PATH,_source_file);
        strcat_s(_source_file,MAX_PATH,"\\words.txt");
    }
    OutputDebugStringA(_source_file);
    read_source_file();
    _mode_learn = (_opt.regime() == _opt.OPTIONS::A::APP_REGIME::_study_) ? true : false;
    if(_mode_learn)
        OnBnClickedRadioLearn();
    else
        fill_ui_data(true);
    fill_to_combo();
    if(!_mode_learn)
        OnBnClickedRadioChoose();
    SetWindowPos(&wndTopMost,_opt.left(),_opt.top(),0,0,SWP_NOSIZE);
    CheckOnTop.SetCheck(BST_CHECKED);
    return TRUE;
}

void CEnglishTrainingDlg::OnPaint(){
    if(IsIconic()){
        CPaintDC dc(this); // device context for painting
        SendMessage(WM_ICONERASEBKGND,reinterpret_cast<WPARAM>(dc.GetSafeHdc()),0);
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;
        dc.DrawIcon(x,y,m_hIcon);
    }else
        CDialogEx::OnPaint();
}

HCURSOR CEnglishTrainingDlg::OnQueryDragIcon(){ return static_cast<HCURSOR>(m_hIcon); }

void CEnglishTrainingDlg::read_source_file(){
    if(!IsFileExists(_source_file))
        if(!DownloadFileSynch("http://www.pokutan.com/tmp/words.txt",_source_file))
            return;
    std::stringstream ss;
    std::ifstream ifs(_source_file);
    _words_map.clear();
    _most_active_words_map.clear();
    _last_eng_word.clear();
    if(ifs.is_open()){
        ss << ifs.rdbuf();
        string s = ss.str();
        if(!(s.size() % sizeof(wchar_t))){
            _words_map.clear();
            wstring ws;
            ws.resize(s.size() / sizeof(wchar_t));
            std::memcpy(&ws[0],isalpha(s[0]) ? s.c_str() : s.c_str() + sizeof(wchar_t),isalpha(s[0]) ?  s.size() : s.size()-1);
            for(;;){
                bool f = true;
                size_t seprtr_idx = ws.find(L';');
                size_t new_line_idx = ws.find(L"\r\n");
                if(new_line_idx == wstring::npos){
                    new_line_idx = ws.length();
                    f = false;
                }
                if(ws[0] != L'#')
                    parse_and_insert_str(ws, new_line_idx, false);
                if(!f)
                    break;
                ws = &ws[new_line_idx + 2];
            }
//            fill_ui_data(false);
        }
        ifs.close();
    }
    _rnd->set_range(0,_words_map.size());
}

void CEnglishTrainingDlg::OnTimer(UINT_PTR nIDEvent){
    KillTimer(_my_timer);
    fill_ui_data(true);
    ShowWindow(SW_SHOW);
    CDialogEx::OnTimer(nIDEvent);
}

void CEnglishTrainingDlg::OnBnClickedBtnSubmit(){
    wchar_t curr_translation[MAX_PATH]={};
    Translations.GetWindowTextW(curr_translation,MAX_PATH);
    if(!curr_translation[0])
        return;
    MAP_IT it;
    Stat_Result.SetWindowTextW(_mode_learn ? L"Choose Translation" : L"Choose Word");
    bool is_substr = false, suggest_found = false;
    wstring compare_to, substr_full_transl;
    int rus_to_eng = _mode_learn ? !_rus2eng_learn : _opt._static_data._vocab_from_rus2eng;
    auto f = [&](WORDS_MAP& map_){ for(it = _words_map.begin(); it != _words_map.end(); ++it)if(it->second == curr_translation)break; };
    if(!rus_to_eng){
        // seek for Russian translation
        if(_curr_pair.second == curr_translation)
            compare_to = _curr_pair.first;
        else{
            if(!_mode_learn){
                f(_words_map);
                if(it == _words_map.end())
                    f(_most_active_words_map);
                _curr_pair = MAP_PAIR(it->first, it->second);
            }
            if(wcslen(curr_translation) >= 3 && (wcsstr(_curr_pair.second.c_str(), curr_translation) != NULL) && (_curr_pair.second != curr_translation)){
                if(!_mode_learn || (_mode_learn && _curr_pair.second == _curr_right_transl)){
                    is_substr = true;
                    substr_full_transl = _curr_pair.second;
                    compare_to = _curr_pair.first;
                    suggest_found = true;
                }
            }
        }
    }else{
        // find English keyword
        if((it = _words_map.find(curr_translation)) == _words_map.end())
            // look for a substring
            for(it = _words_map.begin(); it != _words_map.end(); ++it)
                if((is_substr = (wcslen(curr_translation) >= 3 && (wcsstr(it->first.c_str(),curr_translation) != NULL) && (it->first != curr_translation)))){
                    compare_to = it->second;
                    substr_full_transl = it->first;
                    suggest_found = true;
                    break;
                }
        if(_mode_learn){
            if(it != _words_map.end())
                compare_to = it->second;
        }else
            _curr_pair = MAP_PAIR(it->first, it->second);
    }
    // after 3 wrong tries we start to suggest translation
    static int try_counter = 1;
    static size_t i = 0;

    if(_mode_learn && !compare_to.length() && !suggest_found){
        if(++try_counter <= 3)
            _text_err = L"Wrong! Try Again: ";
        else if(i < _curr_right_transl.length())
            _text_err += _curr_right_transl[i++];
        Stat_Result.SetWindowTextW(_text_err.c_str());
        return;
    }
    if(is_substr && substr_full_transl.length() && suggest_found){
        Translations.SetCurSel(Translations.FindString(0,substr_full_transl.c_str()));
        if(_mode_learn)
            return;
    }
    if(compare_to.length())
        _last_eng_word = _curr_pair.first;
    if(!_mode_learn){
        wstring s = _curr_pair.first + L" - " + _curr_pair.second;
        Stat_Result.SetWindowTextW(s.c_str());
        return;
    }
    STRINGW src;
    SourceWord.GetWindowTextW(&src[0],MAX_PATH);
    if(src == compare_to.c_str()){
        ShowWindow(SW_HIDE);
        src += L" - ";
        src += curr_translation;
        PrevTranslation.SetWindowTextW(src.Str());
        if(_opt.to() != -1)
            _my_timer = SetTimer(1,_opt.to(),NULL);
        try_counter = 1;
        i = 0;
    }else{
        if(++try_counter <= 3)
            _text_err = L"Wrong! Try Again: ";
        else if(i < _curr_right_transl.length())
            _text_err += _curr_right_transl[i++];
        Stat_Result.SetWindowTextW(_text_err.c_str());
    }
}

void CEnglishTrainingDlg::OnCbnSelchangeComboTo(){
    wchar_t s[10]={};
    ComboTO.GetWindowTextW(s, 10);
    _opt.set_show_timeout(s);
}

void CEnglishTrainingDlg::OnBnClickedBtnEditfile(){
    ::ShellExecuteA(m_hWnd,"edit",_source_file,NULL,NULL,SW_SHOWNA);
}

void CEnglishTrainingDlg::OnBnClickedBtnReload(){
    read_source_file();
    fill_ui_data(false);
    Translations.SetFocus();
}

void CEnglishTrainingDlg::OnBnClickedRadioChoose(){
    CheckRadioButton(IDC_RADIO1,IDC_RADIO2,IDC_RADIO2);
    CheckTranslateFromEng.EnableWindow(TRUE);
    if(CWnd* edit_combo = Translations.GetWindow(GW_CHILD)){
        Translations.ModifyStyle(CBS_DROPDOWN, CBS_DROPDOWNLIST);
        edit_combo->SendMessage(EM_SETREADONLY ,TRUE ,0);
    }
    _mode_learn = false;
    Stat_Result.SetWindowTextW(L"Choose word from combo");
    SourceWord.SetWindowTextW(L"");
    fill_ui_data(true);
    KillTimer(_my_timer);
}

void CEnglishTrainingDlg::OnBnClickedRadioLearn(){
    CheckRadioButton(IDC_RADIO1,IDC_RADIO2,IDC_RADIO1);
    CheckTranslateFromEng.EnableWindow(FALSE);
    if(CWnd* edit_combo = Translations.GetWindow(GW_CHILD)){
        Translations.ModifyStyle(CBS_DROPDOWNLIST, CBS_DROPDOWN);
        edit_combo->SendMessage(EM_SETREADONLY, FALSE, 0);
    }
    _mode_learn = true;
    Stat_Result.SetWindowTextW(L"Choose Translation");
    SourceWord.SetWindowTextW(L"");
    fill_ui_data(true);
}

void CEnglishTrainingDlg::open_url(URLS url_index_){
    if(url_index_ < url_vocab && !_curr_pair.first.length())
        return;
    string url;
    STRINGA s;
    if(url_index_ == url_synonym){
        url = _urls[url_synonym];
        WC2MB(_curr_pair.first.c_str(), s);
        url += s.Str();
    }else if(url_index_ != url_vocab && _last_eng_word.length()){
        url = _urls[url_index_];
        WC2MB(_last_eng_word.c_str(), s);
        char* p = strchr(&s[0], ',');
        if(p)*p = '\0';
        if(p = strchr(&s[0], '('))*p = '\0';
        if(p = strchr(&s[0], ' '))*p = '\0';
        url += s.Str();
    }else
        url = _urls[url_index_];
    if(url.length())
        ShellExecuteA(m_hWnd, "open", url.c_str(), NULL, NULL, SW_SHOWNA);
}

void CEnglishTrainingDlg::OnBnClickedBtnPrononce(){ open_url(url_prononce); }

void CEnglishTrainingDlg::OnBnClickedBtnExamp(){ open_url(url_webster); }

void CEnglishTrainingDlg::OnBnClickedBtnHelp(){
    Stat_Result.SetWindowTextW(_curr_right_transl.c_str());
    Translations.SetCurSel(Translations.FindStringExact(0,_curr_right_transl.c_str()));
    wchar_t src[MAX_PATH]={};
    SourceWord.GetWindowTextW(src,MAX_PATH);
    _most_active_words_map.insert(_curr_pair);
    Translations.SetFocus();
}

void CEnglishTrainingDlg::OnBnClickedBtnSetFile(){
    CFileDialog fd(TRUE);
    fd.DoModal();
    OPENFILENAME ofn = fd.GetOFN();
    if(ofn.lpstrFile[0]){
        WideCharToMultiByte(CP_ACP,0,ofn.lpstrFile,-1,_source_file,MAX_PATH,NULL,NULL);
        OnBnClickedBtnReload();
    }
}

void CEnglishTrainingDlg::OnDestroy(){
    CDialogEx::OnDestroy();
    if(_rnd)delete _rnd;
    //UnloadKeyboardLayout(_eng_kbd);
    //UnloadKeyboardLayout(_rus_kbd);
    RECT rc;
    GetWindowRect(&rc);
    _opt._static_data._left = rc.left;
    _opt._static_data._top = rc.top;
    _opt._static_data._regime = _mode_learn ? OPTIONS::A::APP_REGIME::_study_ : OPTIONS::A::APP_REGIME::_vocab_;
}

void CEnglishTrainingDlg::OnBnClickedBtnDict(){ open_url(url_vocab); }

void CEnglishTrainingDlg::OnBnClickedCheckFromEngToRus(){
    _opt._static_data._vocab_from_rus2eng = (CheckTranslateFromEng.GetCheck() == BST_CHECKED) ? 1 : 0;
    fill_ui_data(true);
}

void CEnglishTrainingDlg::OnDblclkStatPrev(){ open_url(url_examples); }

void CEnglishTrainingDlg::OnStnDblclickStatRes(){ open_url(url_examples); }

void CEnglishTrainingDlg::OnStnDblclickStatSorceWord(){ /*open_url(url_examples);*/ }

void CEnglishTrainingDlg::OnActivate(UINT nState,CWnd* pWndOther,BOOL bMinimized){
    CDialogEx::OnActivate(nState,pWndOther,bMinimized);
    Translations.SetFocus();
}

void CEnglishTrainingDlg::OnBnClickedBtnClearWord(){
    MAP_IT it = _words_map.find(_curr_pair.first);
    if(it != _words_map.end())
        _words_map.erase(it);
    it = _most_active_words_map.find(_curr_pair.first);
    if(it != _most_active_words_map.end())
        _most_active_words_map.erase(it);
    fill_ui_data(false);
    Translations.SetFocus();
}

void CEnglishTrainingDlg::OnBnClickedCheckOntop(){
    SetWindowPos(CheckOnTop.GetCheck() == BST_CHECKED ? &wndTopMost : &wndNoTopMost,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE);
}

void CEnglishTrainingDlg::OnBnClickedBtnPlusWord(){
    CString s;
    Translations.GetWindowText(s);
    parse_and_insert_str(wstring(s.GetBuffer()), s.GetLength(), true);
    fill_ui_data(false);
}

void CEnglishTrainingDlg::parse_and_insert_str(wstring& ws, size_t end_ofLine_idx_, bool also_help_map_){
    if(ws.length()){
        size_t seprtr_idx = ws.find(L';');
        wstring s1 = ws.substr(0, seprtr_idx);
        wstring s2 = ws.substr(seprtr_idx + 1, end_ofLine_idx_ - seprtr_idx - 1);
        std::transform(s1.begin(), s1.end(), s1.begin(), ::tolower);
        std::transform(s2.begin(), s2.end(), s2.begin(), ::tolower);
        if(s1.length() && s2.length()){
            // check for English synonyms
            if((seprtr_idx = s1.find(L'(')) != wstring::npos){
                // check that it's not inside transcription
                size_t close_bracket = s1.find(L'}');
                if(close_bracket == wstring::npos || close_bracket < seprtr_idx){
                    size_t end_brace = s1.find(L')');
                    if(end_brace != wstring::npos){
                        wstring syns = s1.substr(seprtr_idx + 1, end_brace - seprtr_idx - 1);
                        s1 = s1.substr(0, seprtr_idx);
                        _syns.insert(MAP_PAIR(s1, syns));
                    }
                }
            }
            _words_map.insert(MAP_PAIR(s1, s2));
            if(also_help_map_)
                _most_active_words_map.insert(MAP_PAIR(s1, s2));
//            fill_ui_data(false);
        }
    }
}

void CEnglishTrainingDlg::OnBnClickedBtnSyns(){
    if(!_curr_pair.first.length())
        return;
    wstring syns;
    MAP_IT it = _syns.find(_curr_pair.first);
    if(it != _syns.end()){
        syns = it->second;
        syns += L"\nCheck synonyms online?";
        if(MessageBox(syns.c_str(),_curr_pair.first.c_str(),MB_YESNO) == IDYES)
            open_url(url_synonym);
    }
}
