# INTRODUCTION

A very quick and simple web blog that uses markdown and generates web pages.

## Building

You will need boost asio for Simple Web Server to build. Otherwise build libmarkdown first:


```
git submodule update --init
cd discount
./configure.sh
make
```

Then to build michal\_vs\_computers, run the following in top level directory:

```
cmake .
make
```

Executables are placed in `build`. The binary takes a path to the resources a command line argument. You can edit the `main.cpp` to change ip and port.

## Content

`Resources` folder should contain:

* `public` which is where the static files should live.
* `pages` which is where your markdown pages go. You need `home.md` and `not_found.md` at least. Url of these pages is the filename. Pages should start with pandoc style headers. These are the first 3 lines of the doc. They should look like this:
* `template.html` which is a page template. String `%{TITLE}%` will be replaced by the title of the page. String `%{CONTENT}%` will be populated with compiled markdown.

```
% Page Title
% Autho
% Date

... the rest of your markdown ...
```

### Editing

Style changes will be pulled in straight away upon a refresh.

Content changes will require a server restart. It caches pages instead of recompiling the markdown everytime.
