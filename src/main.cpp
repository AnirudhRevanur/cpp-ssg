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
  string date;
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

// ---------- helpers ----------------------------------------------------------

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
    if (line.find("title:") == 0)
      fm.title = line.substr(6);
    else if (line.find("description:") == 0)
      fm.description = line.substr(12);
    else if (line.find("date:") == 0)
      fm.date = line.substr(5);
    else if (line.find("draft:") == 0) {
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

string replaceAll(string page, const string &needle,
                  const string &replacement) {
  size_t pos = 0;
  while ((pos = page.find(needle, pos)) != string::npos) {
    page.replace(pos, needle.size(), replacement);
    pos += replacement.size();
  }
  return page;
}

string buildNav(const string &root) {
  return "<nav class=\"site-nav tron-panel\">\n"
         "  <a href=\"" +
         root +
         "index.html\">~/home</a>\n"
         "  <a href=\"" +
         root +
         "blog/index.html\">~/blog</a>\n"
         "</nav>\n";
}

void generatePortfolioPage(const string &portfolioLayout) {
  string page = replaceAll(portfolioLayout, "{{nav}}", buildNav(""));
  ofstream out("dist/index.html");
  out << page;
}

void generateBlogIndex(const vector<Post> &posts, const string &indexLayout) {
  stringstream content;
  for (auto &p : posts) {
    content << "<div class=\"post-card tron-panel\">"
            << "<h3><a href=\"" << p.filename << "\">" << p.title << "</a></h3>"
            << (p.date.empty()
                    ? ""
                    : "<span class=\"post-date\">" + p.date + "</span>")
            << "</div>\n";
  }

  string page = replaceAll(indexLayout, "{{content}}", content.str());
  page = replaceAll(page, "{{nav}}", buildNav("../"));

  fs::create_directories("dist/blog");
  ofstream out("dist/blog/index.html");
  out << page;
}

void generatePosts(vector<Post> &posts, const string &postLayout) {
  fs::create_directories("dist/blog");

  for (size_t i = 0; i < posts.size(); i++) {
    string nav;
    if (i > 0)
      nav += "<a href=\"" + posts[i - 1].filename + "\">&#8592; " +
             posts[i - 1].title + "</a>";
    if (!nav.empty())
      nav += " | ";
    nav += "<a href=\"index.html\">Blog</a>";
    if (i + 1 < posts.size())
      nav += " | <a href=\"" + posts[i + 1].filename + "\">" +
             posts[i + 1].title + " &#8594;</a>";

    string page = postLayout;
    page = replaceAll(page, "{{content}}", posts[i].htmlContent);
    page = replaceAll(page, "{{title}}", posts[i].title);
    page = replaceAll(page, "{{navigation}}", nav);
    page = replaceAll(page, "{{toc}}", posts[i].tocHtml);
    page = replaceAll(page, "{{nav}}", buildNav("../"));

    ofstream fout("dist/blog/" + posts[i].filename);
    fout << page;
  }
}

int main() {
  vector<Post> posts;

  string postLayout = readFile("templates/layout.html");
  string indexLayout = readFile("templates/index.html");
  string portfolioLayout = readFile("templates/portfolio.html");

  for (auto &entry : fs::directory_iterator("content")) {
    if (entry.path().extension() != ".md")
      continue;

    string md = readFile(entry.path().string());
    string mdBody;
    Frontmatter fm = parseFrontmatter(md, mdBody);
    if (fm.draft)
      continue;

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
    posts.push_back({fm.title, outname, fm.date, tocHtml, html});
  }

  sort(posts.begin(), posts.end(),
       [](const Post &a, const Post &b) { return a.date > b.date; });

  generatePosts(posts, postLayout);
  generateBlogIndex(posts, indexLayout);
  generatePortfolioPage(portfolioLayout);

  cout << "Built " << posts.size() << " post(s).\n";
  return 0;
}
