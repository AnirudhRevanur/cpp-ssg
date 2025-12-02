#include <cstddef>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <md4c-html.h>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
namespace fs = std::filesystem;

struct Post {
  string title;
  string filename;
};

struct Frontmatter {
  string title = "Untitled";
  string description = "";
  string date = "";
  bool draft = false;
};

Frontmatter parseFrontmatter(const string &text, string &bodyOut) {
  Frontmatter fm;

  if (text.rfind("---", 0) != 0) {
    bodyOut = text;
    return fm;
  }

  size_t end = text.find("\n---", 3);
  if (end == string::npos) {
    bodyOut = text;
    return fm;
  }

  string fmText = text.substr(3, end - 3);
  bodyOut = text.substr(end + 4);

  istringstream ss(fmText);
  string line;

  while (getline(ss, line)) {
    if (line.find("title:") == 0) {
      fm.title = line.substr(6);
    } else if (line.find("description:") == 0) {
      fm.description = line.substr(12);
    } else if (line.find("date:") == 0) {
      fm.date = line.substr(5);
    } else if (line.find("draft:") == 0) {
      string v = line.substr(6);
      fm.draft = (v.find("true") != string::npos);
    }
  }

  return fm;
}

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
  vector<Post> posts;
  string postLayout = readFile("templates/layout.html");
  string indexLayout = readFile("templates/index.html");

  for (auto &entry : fs::directory_iterator("content")) {
    if (entry.path().extension() != ".md")
      continue;

    string md = readFile(entry.path().string());
    string mdBody;
    Frontmatter fm = parseFrontmatter(md, mdBody);

    string html;
    md_html(mdBody.c_str(), mdBody.size(), md_output_callback, &html, 0, 0);

    string page = postLayout;

    size_t p = page.find("{{content}}");
    if (p != string::npos)
      page.replace(p, strlen("{{content}}"), html);

    p = page.find("{{title}}");
    if (p != string::npos)
      page.replace(p, strlen("{{title}}"), fm.title);

    string outname = entry.path().stem().string() + ".html";
    ofstream fout("dist/" + outname);
    fout << page;
    fout.close();

    posts.push_back({fm.title, outname});
  }

  stringstream indexContent;
  indexContent << "<ul>\n";
  for (auto &p : posts) {
    indexContent << "<li><a href=\"" << p.filename << "\">" << p.title
                 << "</a></li>\n";
  }

  indexContent << "</ul>\n";

  string indexPage = indexLayout;

  size_t a = indexPage.find("{{content}}");
  if (a != string::npos)
    indexPage.replace(a, strlen("{{content}}"), indexContent.str());

  ofstream indexOut("dist/index.html");
  indexOut << indexPage;
  indexOut.close();

  return 0;
}
