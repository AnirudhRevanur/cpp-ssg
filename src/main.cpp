#include <algorithm>
#include <cctype>
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
  string tocHtml;
  string htmlContent;
};

struct Heading {
  int level;
  string text;
  string id;
};

struct Frontmatter {
  string title = "Untitled";
  string description = "";
  string date = "";
  bool draft = false;
};

string kebabThisShit(const string &s) {
  string out;
  for (char c : s) {
    if (isalnum(c))
      out += tolower(c);

    else if (c == ' ')
      out += '-';
  }

  return out;
}

vector<Heading> extractHeadings(string &markdown) {
  vector<Heading> list;
  istringstream ss(markdown);
  string line;

  while (getline(ss, line)) {
    size_t level = 0;

    while (level < line.size() && line[level] == '#')
      level++;

    if (level > 0 && line.size() > level + 1) {
      string text = line.substr(level + 1);
      string id = kebabThisShit(text);

      list.push_back({(int)level, text, id});
    }
  }

  return list;
}

string buildTOC(const vector<Heading> &list) {
  if (list.empty())
    return "";

  string html = "<div class=\"toc\">\n<h2>Table of Contents</h2>\n<ul>\n";

  int lastLevel = 0;

  for (auto &h : list) {
    while (h.level > lastLevel) {
      html += "<ul>\n";
      lastLevel++;
    }
    while (h.level < lastLevel) {
      html += "</ul>\n";
      lastLevel--;
    }

    html += "<li><a href=\"#" + h.id + "\">" + h.text + "</a></li>\n";
  }

  while (lastLevel > 0) {
    html += "</ul>\n";
    lastLevel--;
  }

  html += "</div>\n";
  return html;
}

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

void generateIndexPage(const vector<Post> &posts, const string &indexLayout) {
  stringstream content;

  for (auto &p : posts) {
    content << "<div class=\"post-card\">"
            << "<h3><a href=\"" << p.filename << "\">" << p.title << "</a></h3>"
            << "</div>\n";
  }

  string page = indexLayout;

  size_t pos = page.find("{{content}}");
  if (pos != string::npos)
    page.replace(pos, strlen("{{content}}"), content.str());

  ofstream out("dist/index.html");
  out << page;
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

    vector<Heading> tocList = extractHeadings(mdBody);
    string tocHtml = buildTOC(tocList);

    string html;
    md_html(mdBody.c_str(), mdBody.size(), md_output_callback, &html, 0, 0);

    for (auto &h : tocList) {
      string needle = "<h" + to_string(h.level) + ">";
      string replacement = "<h" + to_string(h.level) + " id=\"" + h.id + "\">";

      size_t pos = 0;
      while ((pos = html.find(needle, pos)) != string::npos) {
        html.replace(pos, needle.length(), replacement);
        pos += replacement.length();
      }
    }

    string outname = entry.path().stem().string() + ".html";

    posts.push_back({fm.title, outname, tocHtml, html});
  }

  std::sort(posts.begin(), posts.end(), [](const Post &a, const Post &b) {
    return a.filename < b.filename;
  });

  for (size_t i = 0; i < posts.size(); i++) {
    string nav;

    if (i > 0) {
      nav += "<a href=\"" + posts[i - 1].filename + "\">← " +
             posts[i - 1].title + "</a>";
    }

    if (!nav.empty())
      nav += " | ";
    nav += "<a href=\"index.html\">Home</a>";

    if (i + 1 < posts.size()) {
      nav += " | <a href=\"" + posts[i + 1].filename + "\">" +
             posts[i + 1].title + " →</a>";
    }

    string page = postLayout;

    size_t p = page.find("{{content}}");
    if (p != string::npos)
      page.replace(p, strlen("{{content}}"), posts[i].htmlContent);

    p = page.find("{{title}}");
    if (p != string::npos)
      page.replace(p, strlen("{{title}}"), posts[i].title);

    p = page.find("{{navigation}}");
    if (p != string::npos)
      page.replace(p, strlen("{{navigation}}"), nav);

    p = page.find("{{toc}}");
    if (p != string::npos)
      page.replace(p, strlen("{{toc}}"), posts[i].tocHtml);

    ofstream fout("dist/" + posts[i].filename);
    fout << page;
    fout.close();
  }

  generateIndexPage(posts, indexLayout);

  return 0;
}
