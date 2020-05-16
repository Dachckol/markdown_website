#include <iostream>
#include <algorithm>
#include <string>
#include <sstream>
#include <fstream>
#include <map>

#include <httplib.h>

char page_template[1000];
std::string resources_path;
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

  pages.insert(std::make_pair(page_name, contents.str()));
  return true;
}


std::string get_page(const std::string page_name) {
  if (!exists(page_name))
    if (!generate_page(page_name)) return get_page("not_found");
  return pages.at(page_name);
}


std::string build_template(const std::string& title) {
  const char* title_tag = "%{TITLE}%";
  const char* content_tag = "%{CONTENT}%";
  std::string clone(page_template);
  clone.replace(clone.find(title_tag), 9, title);
  clone.replace(clone.find(content_tag), 11, get_page(title));
  return clone;
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
  resources_path = std::string(argv[1]);
  initialise();
  httplib::Server server;


  // endpoints
  server.Get(
      R"(/)",
      [&](const httplib::Request& req, httplib::Response& res) {
    res.set_content(
        build_template("home"), 
        "text/html"
        );
  });

  server.Get(
      R"(/([^:/?#]+))",
      [&](const httplib::Request& req, httplib::Response& res) {
    auto title = req.matches[1];
    res.set_content(
        build_template(title), 
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
