#include <fstream>
#include <iostream>
#include <md4c-html.h>
#include <sstream>
#include <string>

using namespace std;

static void md_output_callback(const char *text, MD_SIZE size, void *userdata) {
  string *out = static_cast<string *>(userdata);
  out->append(text, static_cast<size_t>(size));
}

string readFile(const string &path) {
  ifstream in(path, ios::in | ios::binary);
  if (!in)
    throw runtime_error("Failed to open file: " + path);
  ostringstream ss;
  ss << in.rdbuf();
  return ss.str();
}

int main(int argc, char **argv) {
  if (argc < 2) {
    cerr << "Usage: md2html <markdown-file>\n";
    return 1;
  }

  string md = readFile(argv[1]);
  string html;

  int ret = md_html(md.c_str(), md.size(), md_output_callback, &html, 0, 0);
  (void)ret;

  cout << html;

  return 0;
}
