---
title: Writing my own Static Site Generator in C++
date: '13-12-2025'
language: 'en'
tags: ['Blog', 'C++', 'SSG']
draft: false
description: How I replaced my NextJS SSG with a simple C++ SSG
---

Static Site Generators are simple tools that do what they say. They build a static website.
Meaning it shows information that the server decides to show, and limits the user interactivity.

It does seem counter-intuitive in this age and era of having over-the-top websites for blogs and portfolio websites, but it doesn't make sense to include all that interactivity on a blog website.

While my previous SSG worked very well for me, over a period of time it became more of a problem than a solution.
Constant emails from Vercel warning me about outdated versions of Node and React, slow build and deploy times, and so many more issues.

In this article I'll explain how I replaced a 200-file NextJS based SSG with a simple 250-line C++ code that does the same

# Requirements
Before I did anything about this project, I asked myself,

> "Is this even necessary?"

I decided to ignore the voices in my head and did it anyway.

At its core, the problem was simple: take a Markdown file and render it as an HTML on a browser.
Now that the main problem statement was defined, the question was, how do we do it?

The first thought that comes to mind is, is there a JS module that does this?
The answer is Yes. Doesn't matter, there's a JS module for everything, but I decided that I wanted a break from JS and I decided to do stuff in C++.

Now the main requirements were:
- Read a Markdown File
- Follow a template
- Generate HTML files
- Deploy the HTML file somewhere so users can see it

There was no need for any fancy JS magic. Since it is a blog website, it's more of a one way communication. I put out what I want to, and you consume it whether you like it or not.

# Why C++?
I'm known for doing weird stuff in weird languages.

Oh what's that? You have to use TypeScript for making websites? Nahhhh, I choose to make an [unsafe implementation of RSA](https://github.com/AnirudhRevanur/rsa-tool) in TypeScript.
So just like that I said, "Hey, what could be a weird language that I can use here?". Since I was learning C++, I decided to make a C++ project, which ended up to be this Static Site Generator.

Apart from me being me, C++ also has its own advantages over JS/TS.
- C++ is **fast**, and I mean fast
- Build times are very low
- A single binary output can do everything
- It has the same behaviour every run (Looking at you React Hydration)

# Project Structure
The project has a very simple structure and it looks like this

```
.
├── content
│   └── <all-blogs>.md
├── Makefile
├── README.md
├── src
│   └── main.cpp
├── static
│   └── images
│       └── <Blog Name>
│           └── <images-needed>.png
└── templates
    ├── index.html
    ├── layout.html
    └── styles.css
```

- `content/` directory contains all the blogs that you see.
- `Makefile` compiles and generates the compiled binary output.
- `src/` contains all the code files, which turns out to be one file with 250 lines (subject to change as I add more features).
- `static/` stores all the images that are needed for blogs.
- `templates/` contains:
  - `index.html` which is the homepage
  - `layout.html` is the template for the actual blog
  - `styles.css` is what is giving the glow effect, and the TRON Grid on the background.

# Breaking Down the Problem
So right now, we have a clearly defined problem statement, which is to convert a Markdown file into an HTML file which can be deployed.

The right thing to do is to break down this problem into smaller problems so that my goldfish brain can actually do its work.

This is what the SSG has to do:
- Read Markdown Files
- Parse the Frontmatter
- Convert Markdown to HTML
- Generate a Table of Contents
- Render on templates
- Post Navigation
- Generate Index Page

Let's go through each one of these one by one

## Reading Markdown Files

The first thing that the SSG has to do is read Markdown Files from the `content/` directory. I used C++17's `<filesystem>` library in order to walk through the directory and process only the .md files.

```c++

  for (auto &entry : fs::directory_iterator("content")) {
    if (entry.path().extension() != ".md")
      continue;

    string md = readFile(entry.path().string());

```

This loop goes through every file in the `content/` directory. If the file is not a .md, it's skipped.

Each md file is then read into memory with a small helper function (readFile)

```c++

string readFile(const string &path) {
  ifstream in(path, ios::in | ios::binary);
  if (!in)
    throw runtime_error("Failed to open file: " + path);
  ostringstream ss;
  ss << in.rdbuf();
  return ss.str();
}
```

I figured I'd make this its own function since I'd be reading from a file very frequently in this project.

## Parsing Frontmatter

Now you don't see this in the blog, but at the top of every md file, I write a little information about the file and blog. So the name of the Blog, a mini description, the date of writing the blog etc.

In order to use this data, I have to parse this Frontmatter. The frontmatter looks something like this

```md
---
Name: <name-of-blog>
Date: DD/MM/YYYY
Tags: ['Depends', 'on', 'what']
Description: A simple one-liner of the blog
---
```

Now in order to parse this, I wrote a small helper function that separates the Frontmatter from the rest of the blog

```c++
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

... Clipped
```

This searches for the Frontmatter that is present at the top of the file. If the Frontmatter doesn't exist, then it treats the entire file as a simple Markdown.

## Converting Markdown to HTML

The biggest and core part of this project resides here. Like any sane person would, instead of writing my own parser, I decided to use **md4c**, which is a fast and minimal Markdown Parsing Library for C/C++.

```c++
md_html(
  mdBody.c_str(),
  mdBody.size(),
  md_output_callback,
  &html,
  0,
  0
);
```

The callback function appends the generated HTML directly into a string, avoiding unnecessary buffers and memory usage.

## Generating Table of Contents

So at the top of this page, you see a Table of Contents with all the heading and subheading right? Yea, I wrote a function that parses through the Markdown to extract headings.
I made sure that code snippets with comments in them are **not** considered as headings

These headings that are parsed are then used to build a Table of Contents in a hierarchical manner. Each heading is given a URL-friendly ID with kebab-case, allowing clicking on a ToC link to directly jump you there

```c++
    vector<Heading> tocList = extractHeadings(mdBody);
    string tocHtml = buildTOC(tocList);


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
```

These two functions combined, help generate the Table of Contents so it's easier for all users to access stuff.

## Rendering Templates

HTML templates are kept very simple. They use placeholders that can be easily identified and are unique.

```c++
  size_t pos = page.find("{{content}}");
  if (pos != string::npos)
    page.replace(pos, strlen("{{content}}"), content.str());
```

This avoids the need of a full template engine, and keeps the entire code easy to understand and modify.

## Post Navigation

Once all the blog posts are generated, it feels nice to be able to move between them without going back to the homepage every single time.

Since the sites are generated at build time, post navigation has to be done at build time as well. The idea for this is simple: once all posts are collected, sort them, then link each post to the previous and the next one.

```c++
  std::sort(posts.begin(), posts.end(), [](const Post &a, const Post &b) {
    return a.filename < b.filename;
  });
...

...
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
```

This generates a simple previous/next navigation along with a link to the homepage as well. Since all of this is static, there is no runtime magic. It's all HTML links

## Generating the Index Page

The index page is what ties up everything together. You can go anywhere in the website from here.

Instead of hardcoding the page, it's generated dynamically with using another template

```c++
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
```

Each page is rendered as a simple card that links to the corresponding HTML file. This keeps the homepage clean and makes adding new posts as easy as making a new markdown file in the `content/` directory.

# Putting Everything Together

Once all these small individual pieces are done, the main function ties it together, and it does the following in this order:
1. Read all the Markdown Files
2. Parse Frontmatter and Content
3. Convert Markdown to HTML
4. Generate a Table of Contents
5. Render Templates
6. Add navigation
7. Generate Index Page

Since we are using an external library for this and need to link them during compile, I decided to make a simple `Makefile` that compiles the code for me, then runs the compiled output, and generate all the html files inside the `dist/` directory which can then be uploaded on a server.

# What I Learnt

Over the course of the few days that I did this project I learnt a few things. I learnt that C++ is an insanely fun language to work with and a lot of stuff works, but after messing around a lot.

I still use NextJS, but only for when I want to build a full website with interactivity and styles and all that good stuff. For this blog I felt that a more chill, simple, HTML file works perfectly fine.

So I have now managed to successfully build another tool in a language that it _generally_ shouldn't be written in, but that's me. I do stuff in ways that it shouldn't be done in. It's not the most optimal thing, but I do it my way.
