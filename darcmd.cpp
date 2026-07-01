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

std::string input_dialog(const char* title, const char prev[128] = "");
std::string input_dialog(const char* title, const char prev[128]) {
  int h = 5, w = 40;
  int y = LINES/2 - h/2, x = COLS/2 - w/2;
  WINDOW* win = newwin(h, w, y, x);
  box(win, 0, 0);
  mvwprintw(win, 0, 2, " %s ", title);
  mvwprintw(win, 2, 2, "> ");
  wrefresh(win);

  char buf[256] = {};
  strcpy(buf, prev);
  int i = strlen(buf);
  mvwprintw(win, 2, 4, "%-36s", buf);
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

void popup_message(const char* title, const char* text) {
  int h = 5, w = 40;
  int y = LINES/2 - h/2, x = COLS/2 - w/2;
  WINDOW* win = newwin(h, w, y, x);
  box(win, 0, 0);
  mvwprintw(win, 0, 2, " %s ", title);
  mvwprintw(win, 1, 1, "%s", text);
  wrefresh(win);
  int ch;
  while ((ch = getch()) != '\n')  {
    box(win, 0, 0);
    mvwprintw(win, 0, 2, " %s ", title);
    mvwprintw(win, 1, 1, "%s", text);
    wrefresh(win);
  }
  delwin(win);
}

bool is_junction(const std::string& path) {
  DWORD attr = GetFileAttributesA(path.c_str());
  return (attr != INVALID_FILE_ATTRIBUTES) && 
         (attr & FILE_ATTRIBUTE_REPARSE_POINT);
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

void cd_up();
void draw_half(int *cur, char (*fils)[64], char *pth, bool half) {
  int start = (COLS/2) * half;
  mvprintw(1, start, "|          File Name            |  Size  |");
  mvprintw(0, start, "|%s", pth);
  char name[64];
  char entr[43];
  memset(entr, ' ', 26);
  entr[0 ] = '|';
  entr[32] = '|';
  entr[41] = '|';
  entr[42] = '\0';
  int j = 2, cfile = 0;
  bool er = 0;
  try {
    for (auto& entry : fs::directory_iterator(pth, fs::directory_options::skip_permission_denied)) {
      if (cfile >= 64) { er = 1; break; }
      memset(entr, ' ', 42);
      entr[0 ] = '|';
      entr[42] = '\0';
      for(int i = 0; i < 64; i++) name[i] = ' ';
      std::string filename = utf8_to_cp1251(entry.path().filename().string());
      snprintf(name, sizeof(name), "%s", filename.c_str());
      for(int i = strlen(name); i < 31; i++) name[i] = ' ';
      std::string size = is_junction(std::string(entry.path().string())) ? "Link" : (entry.is_directory() ? "Folder" : std::to_string(entry.file_size()));
      memcpy(entr + 1, name, 31);
      memcpy(fils[cfile], name, 63);
      fils[cfile][63] = '\0';
      entr[32] = '|';
      memcpy(entr + 33, size.c_str(), std::min((int)size.size(), 7));  // размер без \0
      entr[41] = '|';
      bool sel = (cfile == *cur) && (page == half);
      if(sel) attron(A_REVERSE);
      mvprintw(j, start, entr);
      if(sel) attroff(A_REVERSE);
      j++; cfile++;
    }
    if(er) {
      popup_message("Error", "File overflow"); cd_up();
    }
    refresh();
  } catch (const fs::filesystem_error& e) {
    popup_message("Error", "Access denied");
    cd_up();
  }
}

void redraw() {
  clear();
  std::string title;
  title = "Dar commander - " + std::string(page ? files2[current2] : files[current]);
  SetConsoleTitleA(title.c_str());
  draw_half(&current , files , path , 0);
  draw_half(&current2, files2, path2, 1);
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
  if (strlen(pth) == 2 && pth[1] == ':') strcat(pth, "/");
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
  redraw(); // do NOT touch 
  std::string s(pth);
  std::string f = cp1251_to_utf8(std::string(fils[*cur]));
  while (!f.empty() && (f.back() == ' ' || f.back() == '\0')) f.pop_back();
  std::string full = s + (s.back() == '/' ? "" : "/") + f;
  int ch = getch();
  if (ch == '\n') return 0;
  if (ch == KEY_LEFT) {
    cd_up();
    memset(fils, 0, sizeof(char[64][64]));
    *cur = 0;
  }
  if (ch == KEY_RIGHT) {
    //log("KEY_RIGHT full='%s' isdir=%d", full.c_str(), fs::is_directory(full));
    if (fs::is_directory(full)) {
      full += "/";
      strcpy(pth, full.c_str());
      //log("full=%s", full.c_str());
      *cur = 0;
      memset(fils, 0, sizeof(files));
    } else {
      int wlen = MultiByteToWideChar(CP_UTF8, 0, full.c_str(), -1, nullptr, 0);
      std::wstring wfull(wlen, 0);
      MultiByteToWideChar(CP_UTF8, 0, full.c_str(), -1, &wfull[0], wlen);
      ShellExecuteW(NULL, L"open", wfull.c_str(), NULL, NULL, SW_SHOW);
    }
  }
  if (ch == KEY_UP   && *cur > 0 ) (*cur)--;
  if (ch == KEY_DOWN && *cur < 63) (*cur)++;
  if (ch == '\t') page = !page;
  if (ch == 'd') {
    fs::remove_all(full);
  }
  if (ch == 'r') {
    fs::rename(full, s + (s.back() == '/' ? "" : "/") + input_dialog("New file name:", f.c_str()));
  }
  if (ch == 'm') {
    std::string s2(path2);
    std::string s1(path);
    if(page) {
      fs::rename(s2 + (s2.back() == '/' ? "" : "/") + f, s1 + (s1.back() == '/' ? "" : "/") + input_dialog("Move as:", f.c_str()));
    } else {
      fs::rename(s1 + (s1.back() == '/' ? "" : "/") + f, s2 + (s2.back() == '/' ? "" : "/") + input_dialog("Move as:", f.c_str()));
    }
  }
  if (ch == 'c') {
    fs::copy(full, s + (s.back() == '/' ? "" : "/") + input_dialog("Copy as:", f.c_str()), fs::copy_options::recursive);
  }
  if (ch == 'h') {
    popup_message("Dar commander Alpha v0.3", pth);
  }
  if (ch == 'g') {
    strcpy(pth, input_dialog("Go to:", pth).c_str());
    memset(fils, 0, sizeof(files));
    *cur = 0;
  }
  if (ch == 'n') {
    fs::create_directory(s + (s.back() == '/' ? "" : "/") + input_dialog("New folder:"));
  }
  if (ch == 's') {
    std::string rq = input_dialog("Search for:");
    int found = 0;
    for(int i = 0; i < 64; i++) {
      std::string file = std::string(fils[i]);
      if(file.find(rq) != std::string::npos) {
        popup_message("Found", file.c_str()); found++; //cnueonti;
      }
    }
    char ff[40];
    sprintf(ff, "Found %d results", found);
    popup_message(ff, "Press enter to exit");
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