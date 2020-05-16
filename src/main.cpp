#include <iostream>
#include <algorithm>
#include <string>
#include <sstream>
#include <fstream>
#include <map>

#include <httplib.h>

#define TITLE_TAG "%{TITLE}%"
#define CONTENT_TAG "%{CONTENT}%"

extern "C" {
#include <mkdio.h>
}

char page_template[1000];
char* resources_path;
std::map<std::string, std::string> pages;

bool exists(const std::string& page_name) {
  return pages.find(page_name) != pages.end();
}


bool generate_page(const std::string& page_name) {
  std::string file_name = page_name + std::string(".md");
  std::ifstream page_file(resources_path+std::string("/pages/")+file_name);
  if(!page_file) return false;

  std::stringstream contents;
  contents << page_file.rdbuf();

  std::string file_contents = contents.str();
  MMIOT* md = mkd_string(file_contents.c_str(), file_contents.size(), 0);
  mkd_compile(md, 0);
  char* html_contents;
  int size = mkd_document(md, &html_contents);

  std::string html_page(page_template);
  while(html_page.find(TITLE_TAG, strlen(TITLE_TAG)) != html_page.npos) {
    html_page.replace(html_page.find(TITLE_TAG), strlen(TITLE_TAG), mkd_doc_title(md));
  }
  html_page.replace(html_page.find(CONTENT_TAG), strlen(CONTENT_TAG), html_contents);

  pages.insert(std::make_pair(page_name, html_page));
  return true;
}


std::string get_page(const std::string page_name) {
  if (!exists(page_name))
    if (!generate_page(page_name)) return get_page("not_found");
  return pages.at(page_name);
}


void initialise() {
  std::ifstream template_file(resources_path+std::string("/template.html"));
  template_file.getline(page_template, sizeof(page_template), '\0');
  pages = std::map<std::string, std::string>();
}


int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "Need resources dir as first arg" << std::endl;
    return -1;
  }
  resources_path = argv[1];
  initialise();
  httplib::Server server;


  // endpoints
  server.Get(
      R"(/)",
      [&](const httplib::Request& req, httplib::Response& res) {
    res.set_content(
        get_page("home"), 
        "text/html"
        );
  });

  server.Get(
      R"(/([^:/?#]+))",
      [&](const httplib::Request& req, httplib::Response& res) {
    auto title = req.matches[1];
    res.set_content(
        get_page(title), 
        "text/html"
        );
  });


  // static resources
  bool good_mount = server.set_mount_point(
      "/public",
      (resources_path + std::string("/public")).c_str()
      );
  if (!good_mount) throw std::runtime_error("Failed to mount static files");

  server.listen("localhost", 4138);
  return 0;
}
