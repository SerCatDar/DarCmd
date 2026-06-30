#include <curses.h>
#include <filesystem>
#include <cstring>
#include <cstdlib>
#include <locale>
#include <windows.h>
#include <cstdarg>
#include <shellapi.h>

namespace fs = std::filesystem;
char path[256] = "C:/";
char path2[256] = "C:/";

int current = 0;
int current2 = 0;
int page = 0;
char files[64][64];
char filer[8] = "hjfhjr3";
char files2[64][64];

std::string input_dialog(const char* title) {
  int h = 5, w = 40;
  int y = LINES/2 - h/2, x = COLS/2 - w/2;
  WINDOW* win = newwin(h, w, y, x);
  box(win, 0, 0);
  mvwprintw(win, 0, 2, " %s ", title);
  mvwprintw(win, 2, 2, "> ");
  wrefresh(win);

  char buf[256] = {};
  int i = 0;
  int ch;
  while ((ch = wgetch(win)) != '\n') {
    if (ch == 27) return "";  // Escape — отмена
    if ((ch == KEY_BACKSPACE || ch == 8) && i > 0) {
      buf[--i] = '\0';
    } else if (ch >= 32 && i < 254) {
      buf[i++] = ch;
      buf[i] = '\0';
    }
    mvwprintw(win, 2, 4, "%-36s", buf);
    wrefresh(win);
  }
  delwin(win);
  return std::string(buf);
}

void popup_message(const char* title) {
  int h = 5, w = 40;
  int y = LINES/2 - h/2, x = COLS/2 - w/2;
  WINDOW* win = newwin(h, w, y, x);
  box(win, 0, 0);
  mvwprintw(win, 0, 2, " %s ", title);
  wrefresh(win);

  getch();
  delwin(win);
}

std::string utf8_to_cp1251(const std::string& s) {
  int wlen = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
  std::wstring wide(wlen, 0);
  MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &wide[0], wlen);
  int len = WideCharToMultiByte(1251, 0, wide.c_str(), -1, nullptr, 0, nullptr, nullptr);
  std::string result(len, 0);
  WideCharToMultiByte(1251, 0, wide.c_str(), -1, &result[0], len, nullptr, nullptr);
  return result;
}

std::string cp1251_to_utf8(const std::string& s) {
  int wlen = MultiByteToWideChar(1251, 0, s.c_str(), -1, nullptr, 0);
  std::wstring wide(wlen, 0);
  MultiByteToWideChar(1251, 0, s.c_str(), -1, &wide[0], wlen);
  int len = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, nullptr, 0, nullptr, nullptr);
  std::string result(len, 0);
  WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, &result[0], len, nullptr, nullptr);
  return result;
}

void log(const char* fmt, ...) {
  FILE* file = fopen("darcmd.log", "a");
  va_list args;
  va_start(args, fmt);
  vfprintf(file, fmt, args);
  va_end(args);
  fprintf(file, "\n");
  fclose(file);
}

void redraw() {
  clear();
  std::string title;
  title = "Dar commander - " + std::string(page ? files[current] : files2[current2]);
  SetConsoleTitleA(title.c_str());
  mvprintw(0, 0,      "           File Name            |  Size  |");
  mvprintw(0, COLS/2, "           File Name            |  Size  |");
  //mvprintw(0, 0, "&cur=%p &files=%p", &current, files);
  char name[64];
  char entr[43];
  memset(entr, ' ', 26);  // заполнить пробелами
  entr[0 ] = '|';
  entr[32] = '|';
  entr[41] = '|';
  entr[42] = '\0';
  int j = 1;
  for (auto& entry : fs::directory_iterator(path, fs::directory_options::skip_permission_denied)) {
    memset(entr, ' ', 42);
    entr[42] = '\0';
    for(int i = 0; i < 64; i++) name[i] = ' ';
    std::string filename = utf8_to_cp1251(entry.path().filename().string());
    snprintf(name, sizeof(name), "%s", filename.c_str());
    for(int i = strlen(name); i < 31; i++) name[i] = ' ';
    std::string size = entry.is_directory() ? "Folder" : std::to_string(entry.file_size());
    memcpy(entr + 1, name, 31);
    memcpy(files[j - 1], name, 63);
    files[j - 1][63] = '\0';
    entr[32] = '|';
    memcpy(entr + 33, size.c_str(), std::min((int)size.size(), 7));  // размер без \0
    entr[41] = '|';
    bool sel = (j - 1 == current) && !page;
    if(sel) attron(A_REVERSE);
    mvprintw(j, 0, entr);
    if(sel) attroff(A_REVERSE);
    j++;
  }
  refresh();
  memset(entr, ' ', 26);  // заполнить пробелами
  entr[0 ] = '|';
  entr[32] = '|';
  entr[41] = '|';
  entr[42] = '\0';
  j = 1;
  for (auto& entry : fs::directory_iterator(path2, fs::directory_options::skip_permission_denied)) {
    memset(entr, ' ', 42);
    entr[42] = '\0';
    for(int i = 0; i < 64; i++) name[i] = ' ';
    std::string filename = utf8_to_cp1251(entry.path().filename().string());
    snprintf(name, sizeof(name), "%s", filename.c_str());
    for(int i = strlen(name); i < 31; i++) name[i] = ' ';
    std::string size = entry.is_directory() ? "Folder" : std::to_string(entry.file_size());
    memcpy(entr + 1, name, 31);
    memcpy(files2[j - 1], name, 63);
    files2[j - 1][63] = '\0';
    entr[32] = '|';
    memcpy(entr + 33, size.c_str(), std::min((int)size.size(), 7));  // размер без \0
    entr[41] = '|';
    bool sel = (j - 1 == current2) && page;
    if(sel) attron(A_REVERSE);
    mvprintw(j, COLS/2, entr);
    if(sel) attroff(A_REVERSE);
    j++;
  }
  refresh();
}

void cd_up() {
  char* pth = page ? path2 : path;
  int len = strlen(pth);
  // do NOT touch
  if (len > 0 && (pth[len-1] == '/' || pth[len-1] == '\\')) pth[--len] = '\0';
  int last = -1;
  for (int i = 0; i < len; i++)
    if (pth[i] == '/' || pth[i] == '\\') last = i;
  if (last > 0) pth[last] = '\0';
}

int loop() {
  int* cur = page ? &current2 : &current;
  char (*fils)[64] = page ? files2 : files;
  char* pth = page ? path2 : path;
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
  int rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
  int cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
  //mvprintw(0, 0, "rows=%d cols=%d LINES=%d COLS=%d", rows, cols, LINES, COLS); refresh(); napms(2000);
  if (rows != LINES || cols != COLS) resizeterm(rows, cols);
  redraw();
  std::string s(pth);
  std::string f = cp1251_to_utf8(std::string(fils[*cur]));
  while (!f.empty() && (f.back() == ' ' || f.back() == '\0')) f.pop_back();
  std::string full = s + (s.back() == '/' ? "" : "/") + f;
  int ch = getch();
  if (ch == '\n') return 0;
  if (ch == KEY_LEFT) {
    cd_up();
  }
  if (ch == KEY_RIGHT) {
    //log("KEY_RIGHT full='%s' isdir=%d", full.c_str(), fs::is_directory(full));
    if (fs::is_directory(full)) {
      full += "/";
      strcpy(pth, full.c_str());
      //log("full=%s", full.c_str());
      *cur = 0;
    } else {
      int wlen = MultiByteToWideChar(CP_UTF8, 0, full.c_str(), -1, nullptr, 0);
      std::wstring wfull(wlen, 0);
      MultiByteToWideChar(CP_UTF8, 0, full.c_str(), -1, &wfull[0], wlen);
      ShellExecuteW(NULL, L"open", wfull.c_str(), NULL, NULL, SW_SHOW);
    }
  }
  if (ch == KEY_UP   && *cur > 0 ) (*cur)--;
  if (ch == KEY_DOWN && *cur < 31) (*cur)++;
  if (ch == '\t') page = !page;
  if (ch == 'd') {
    fs::remove_all(full);
  }
  if (ch == 'r') {
    fs::rename(full, s + (s.back() == '/' ? "" : "/") + input_dialog("New file name:"));
  }
  if (ch == 'm') {
    std::string s2(path2);
    std::string s1(path);
    if(page) {
      fs::rename(s2 + (s2.back() == '/' ? "" : "/") + f, s1 + (s1.back() == '/' ? "" : "/") + input_dialog("Move as:"));
    } else {
      fs::rename(s1 + (s1.back() == '/' ? "" : "/") + f, s2 + (s2.back() == '/' ? "" : "/") + input_dialog("Move as:"));
    }
  }
  if (ch == 'c') {
    fs::copy(full, s + (s.back() == '/' ? "" : "/") + input_dialog("Copy as:"), fs::copy_options::recursive);
  }
  if (ch == 'h') {
    popup_message("Dar commander Alpha v0.2");
  }
  //mvprintw(LINES-1, 0, "ch=%d cur=%d", ch, current); refresh(); napms(2000);
  redraw(); return 1;
}

int main() {
  setlocale(LC_ALL, "");
  SetConsoleOutputCP(1251);
  SetConsoleCP(1251);
  initscr();
  curs_set(0);
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  while(loop());
  clear();
  mvprintw(0, 0, "Press any key to close Dar Commander");
  getch();
  endwin();
  return 0;
}