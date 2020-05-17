% Building A Markdown Site
% Michal
% 16/5/2020

# Building a Markdown Site

#### A really really basic one

## Introduction

This article will take you though the process of creating a simple markdown site using a couple of preexisting libraries and some C++ glue.

The site I build is the very one you using right now. Ideal for a minimalist blog/page that's easy to update and maintain without having to run a big CMS like Wordpress.

You could achieve something very similar with HTML and a bit of CSS on a purely static site, but that's:

1. Boring and lame
2. Annoying to edit

I would like to dump a bunch of markdown files into a folder and have them served up as HTML.

## Other Options

While I was always going to be building my own thing, you might settle for someone else's project.

The idea of a static site based on markdown is nothing new. There are a bunch of online tools that do this already:

1. [blot.im](https://blot.im/) - *a very nice minimalist blog made up of just folders and markdown*
1.  [smallvictori.es](https://www.smallvictori.es) - *another blot, with less focus on markdown*

Both "blot" and "smallvictori" charge around $4 a month for the site. I can take the same $4, get a low spec server and build my own solution (that also gives me something to write about straight away).

## The Plan

I want to use C++ which is a language I currently work with. This is because I am weary of just how powerful a box I can rent for $4. I don't like the standard library too much, but for now my code will be using it because this is a personal project and I'm lazy.

I'm keeping this CSS and HTML only. No JavaScript. This is because I want a very fast, minimalist site with no bloat. This ruled out even the most lightweight JavaScript frameworks.

With C++ I get to choose fast and simple dependencies. I want a straight forward, no-bullshit HTTP server (NO FRAMEWORKS!!!) and something that can take markdown and make it into HTML (ideally with very little input from me).

For the markdown compiler I went with [Discount Markdown](http://www.pell.portland.or.us/~orc/Code/discount/). Its a C library with no external dependencies. It seems fast and well maintained. The documentation features a very encouraging level of cynicism. This, to me, is always a sign of talent and quality.

For the HTTP server I went with [cpp-httplib](https://github.com/yhirose/cpp-httplib). Its simple to use, handles static files really easily and doesn't seem particularly bloated. I don't trust it. So I will be shielding it behind an [NGINX](https://www.nginx.com/) instance on the production environment. I worked with a developer who told me *"If you're doing security, stop doing security"*. NGINX will do the security.

## Functionality

The articles on pages will be written in markdown, but some parts won't change. These are the header and the footer of pages along with the html `<head>` section etc. The code should, therefore, inject our HTML content (compiled from markdown) into designated parts of a template.

I will need to write some CSS so the template will need pull in a static file. The server needs to serve static files from a designated directory (this also lets me embed images). This could alternatively be done though NGINX (which probably does it better). We will see...

I want the website to feel dynamic. If I push a markdown article and it should be instantly accessible.

At the same time, I cannot be generating the html every time the user asks for a webpage (inefficient and vulnerable). The best solution is to generate the page on demand and then cache it. Whenever I update the page, I will need to reload the server to clear the cache. I could write an obscure endpoint for this, but would need to protect it somehow. Since this is only a personal website I can just restart the process for now to reload the changes.

It would be relatively trivial to get this more "RESTful" in a "stateless" sense by storing the compiled pages on the file system. There could then be a separate tool that generates the html files and the server just deals them out. Other fun ideas include change detection for reloading the cache automatically.

## The Solution

The whole thing turned out around 100 lines of C++ so I did not bother with OOP and kept everything in the same file for now. Some important bits:

The markdown library is a C library so use `extern` to import the header. Or you will get weird and mysterious linking errors (thanks gcc!).

```cpp
extern "C" {
#include <mkdio.h>
}
```

The code for generating a page from markdown.

```cpp
std::ifstream page_file(resources_path+std::string("/pages/")+file_name);
if(!page_file) return false; // doesn't exist

std::stringstream contents;
contents << page_file.rdbuf(); // read the full file

std::string file_contents = contents.str();

// This is the markdown compiler bit
MMIOT* md = mkd_string(file_contents.c_str(), file_contents.size(), MKD_FENCEDCODE);
mkd_compile(md, MKD_FENCEDCODE);
char* html_contents;
int size = mkd_document(md, &html_contents);

std::string html_page(page_template); // loaded template earlier. Create a copy

// Replace all occurances of title tags with titles and insert contents.
while(html_page.find(TITLE_TAG, strlen(TITLE_TAG)) != html_page.npos) {
  html_page.replace(html_page.find(TITLE_TAG), strlen(TITLE_TAG), mkd_doc_title(md));
}
html_page.replace(html_page.find(CONTENT_TAG), strlen(CONTENT_TAG), html_contents);

// Store our generated page in "cache"
pages.insert(std::make_pair(page_name, html_page));
```

The `pages` cache is currently just a `std::map<std::string,std::string>`. Could probably be a little smarter with preallocating the size of this or using some kind of static container. It will do for now.

This is how we avoiding generating the page for every request. This bit is quite important.
Otherwise somebody could just refresh the page over and over and drive our server crazy! You can't trust people!

```cpp
std::string get_page(const std::string& page_name) {
  if (!exists(page_name))
    if (!generate_page(page_name)) return get_page("not_found");
  return pages.at(page_name);
}
```

The one thing to note about the above logic is that the `README` states that there must be a `not_found` page. If it can't find the "not_found" page it will recurse infinitely and crash.

Finally, the HTTP server bit. Defines a bunch of endpoints and starts the server.

```cpp
httplib::Server server;

// endpoints
server.Get(
    R"(/)",
    [&](const httplib::Request& req, httplib::Response& res) {
    res.set_content(
        get_page("home"), // home page
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

server.listen("localhost", 4138); // 4138 is arbitrary. nginx will forward requests from HTTP ports to this.
```

The styling and template are up to you so I won't cover those here. All the code along with the build system and the contents and styling of my version (the one you're using) is on my Github:

[Dachckol/markdown_site](https://github.com/Dachckol/markdown_website)

Copy, edit, use, whatever... All the "Michal" related things are in `Resources` so will be easy to edit me out.



