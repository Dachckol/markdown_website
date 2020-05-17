% Building and Deploying a Markdown Site
% Michal
% 16/5/2020

# Building and Deploying a Markdown Site

#### A really really basic one

# Introduction

This article will take you though the process of creating and deploying a simple markdown site using a couple of preexisting libraries and some basic C++ glue.

The site I build is the very one you using right now. Ideal for a minimalist blog/page that's easy to update and maintain without having to run a big CMS like Wordpress.

You could achieve something very similar with HTML and a bit of CSS on a purely static site, but that's:

1. Boring and lame
2. Annoying to edit

I would like to dump a bunch of markdown files into a folder and have them served up as HTML.

# Other Options

While I was always going to be building my own thing, you might settle for someone else's project.

The idea of a static site based on markdown is nothing new. There are a bunch of online tools that do this already:

1. [blot.im](https://blot.im/) - *a very nice minimalist blog made up of just folders and markdown*
1.  [smallvictori.es](https://www.smallvictori.es) - *another blot, with less focus on markdown*

Both "blot" and "smallvictori" charge around $4 a month for the site. I can take the same $4, get a low spec server and build my own solution (that also gives me something to write about straight away).

# The Plan

I want to use C++ which is a language I currently work with. This is because I am weary of just how powerful a box I can rent for $4. I don't like the standard library too much, but for now my code will be using it because this is a personal project and I'm lazy.

I'm keeping this CSS and HTML only. No JavaScript. This is because I want a very fast, minimalist site with no bloat. This ruled out even the most lightweight JavaScript frameworks.

With C++ I get to choose fast and simple dependencies. I want a straight forward, no-bullshit HTTP server (NO FRAMEWORKS!!!) and something that can take markdown and make it into HTML (ideally with very little input from me).

For the markdown compiler I went with [Discount Markdown](http://www.pell.portland.or.us/~orc/Code/discount/). Its a C library with no external dependencies. It seems fast and well maintained. The documentation features a very encouraging level of cynicism. This, to me, is always a sign of talent and quality.

For the HTTP server I went with [cpp-httplib](https://github.com/yhirose/cpp-httplib). Its simple to use, handles static files really easily and doesn't seem particularly bloated. I don't trust it. So I will be shielding it behind an [NGINX](https://www.nginx.com/) instance on the production environment. I worked with a developer who told me *"If you're doing security, stop doing security"*. NGINX will do the security.

# Functionality

The articles on pages will be written in markdown, but some parts won't change. These are the header and the footer of pages along with the html `<head>` section etc. The code should, therefore, inject our HTML content (compiled from markdown) into designated parts of a template.

I will need to write some CSS so the template will need pull in a static file. The server needs to serve static files from a designated directory (this also lets me embed images). This could alternatively be done though NGINX (which probably does it better). We will see...

I want the website to feel dynamic. If I push a markdown article and it should be instantly accessible.

At the same time, I cannot be generating the html every time the user asks for a webpage (inefficient and vulnerable). The best solution is to generate the page on demand and then cache it. Whenever I update the page, I will need to reload the server to clear the cache. I could write an obscure endpoint for this, but would need to protect it somehow. Since this is only a personal website I can just restart the process for now to reload the changes.

It would be relatively trivial to get this more "RESTful" in a "stateless" sense by storing the compiled pages on the file system. There could then be a separate tool that generates the html files and the server just deals them out. Other fun ideas include change detection for reloading the cache automatically.

# The Solution

The whole thing turned out around 100 lines of C++ so I did not bother with OOP and kept everything in the same file for now. Some important bits:

## C++ Bits

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

This is how we avoid generating the page for every request. This bit is quite important.
Otherwise somebody could just refresh the page over and over and drive our server crazy! You can't trust people!

```cpp
std::string get_page(const std::string& page_name) {
  if (!exists(page_name))
    if (!generate_page(page_name)) return get_page("not_found");
  return pages.at(page_name);
}
```

The one thing to note about the above logic is that the `README` states that there must be a `not_found` page. If it can't find the "not_found" page it will recurse infinitely and crash :(

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

// 4138 is arbitrary. nginx will forward requests from HTTP ports to this.
server.listen("localhost", 4138);
```

The styling and template are up to you so I won't cover those here. All the code along with the build system and the contents and styling of my version (the one you're using) is on my Github:

[Dachckol/markdown_site](https://github.com/Dachckol/markdown_website)

Copy, edit, use, whatever... All the "Michal" related things are in `Resources` so will be easy to edit me out.


# Cloud Deployment

Let's deploy this thing!

## The Host

I spent a great deal of time researching the hosting solution. We need a VPS or a cloud server as we want to be running out own server binary. Cloud servers are usually cheaper, but most providers are kind of overkill with the specs for our use case which adds up to more $$$. I ended up going with [Hetzner Cloud](https://console.hetzner.cloud) for the following reasons:

1. The lowest tier cloud server is $2.99 which is the cheapest I could find. The lowest tier is still more than what I need (some basic tests revealed that MarkdownWebsite runs at under 80MB of RAM). 1GB of RAM would probably be sufficient for a stripped down box running our server and NGINX.
2. Hetzner Cloud servers are carbon neutral which is not something you can expect from the likes of DigitalOcean or AWS.

My experience so far has been overwhelmingly positive.

The fact that the server is $2.99 means I have $1 to spare which lets me get a domain name too! I went with [Namecheap](https://www.namecheap.com) because they provide free WhoIs blocking (people cant see your address, email and phone number though domain lookup services) and don't do that thing GoDaddy does, where first year is $1.99 followed by $24 the next. My domain was $10 for the full year which when combined with the server totals up $3.82 a month. I will use the remaining $0.18 to make the world a better place!

I'm a Fedora-boy so that's what I'm using. However, the exact steps will likely be the same for Ubuntu or any other conventional Linux distribution. Otherwise the general steps are the same, but the exact commands etc will differ so get Ecosia/DuckDuckGo-ing.

## Domain Setup

This has been documented everywhere, but I still find myself looking it up so here we go. We want to be accessible on both `http://www.[your domain]` and `http://[your domain]`. Pick one and redirect the other. I think `www` is a little too 2002 for me so I pick the one without `www` and redirect the `www`. I'm sure you can figure out how to do it the other way. Here is the DNS records you want:

|  Type             | From | To                   |
|-------------------|:-----|:---------------------|
| "A" Record        | @    | [your server IP]     |
|Redirect(unmasked) | www  | http://[your domain] |

Can change that bottom one to `https` later.

## Setting up NGINX

We decided NGINX will do the security. Outside of some default settings we need to use SSL. NGINX will handle the SSL stuff and forward the requests to our `localhost:4138` address. This way all traffic is routed though NGINX. You can find a tutorial (like [this one](https://www.linode.com/docs/web-servers/nginx/how-to-configure-nginx/)) on setting up NGINX yourself. This is an example `server` block you can use for the markdown server (for your inspiration):

```
server {
  listen       80 default_server;
  listen       [::]:80 default_server;

  server_name [your domain name];

  location / {
    proxy_pass http://localhost:4138;
  }
}
```

For SSL we will need to have this config in an `sites-enabled` folder. So you want this server block to be in this file: `/etc/nginx/sites-enabled/[your domain]`. You can then edit your `/etc/nginx/nginx.conf` to contain this line in the `http` section:

```
include /etc/nginx/sites-enabled/*;
```

It will pull in all blocks in that `sites-enabled` directory. The `server_name [your domain]` setting in the above server block will make NGINX route all incoming traffic to that domain to that server block. We will therefore need to set the domain as a host for the servers ip. Edit `/etc/hosts` and add the following line:

```
[your server ip] [your domain name]
```

That handles NGINX for now (we still need to sort out SSL). NGINX will route the traffic to `localhost:4138`. We need to make sure our `markdown_website` project is running at all times for it to respond to this `localhost:4138` request. I decided to create a systemd service to handle this. It ensures our server gets run on startup and lets us easily check its state. More half-arsed alternatives include messing around with `tmux` or `screen` sessions.

```
[Unit]
Description=Markdown server
After=network.target

[Service]
Type=simple
ExecStart=[path to built executable] [path to resources folder]

[Install]
WantedBy=multi-user.targe
```

Place that in `/etc/systemd/services/markdown_server.service` and run `systemctl enable markdown_server.service` followed by `systemctl start markdown_server.service`. When you change something in resources run: `systemctl restart markdown_server.service` to restart the thing and reset the cache.

At this point your site should be available on the internet...

## SSL Certificate

We need this to go from `http://` to `https://`. HTTPS means that the traffic is getting encrypted between the client and server. Its a good practice to setup SSL on all your sites as it's free with LetsEncrypt and shows you care. If you don't care, you should.

Since we put our NGINX server block in `sites-enabled` earlier we can use a script to setup the SSL certificates by installing something called `certbot`. On Fedora:

`dnf install certbot certbot-nginx`

On Ubuntu, I think its the same but with `apt` instead of `dnf`.

Run `certbot` like this: ` certbot --nginx -d [your domain]`

It will ask some questions and set it all up for. I recommend redirecting all HTTP traffic to HTTPS (that's the last thing it asks).

That's you done!

Both the systemd service and the NGINX block examples are in [the repo](https://github.com/Dachckol/markdown_website) in the `deployment` folder.

Last thing to do is to load the site up in a browser. Make sure it looks good on your phone. Now for the super scientific load test. Start hard-refreshing (ctrl-shift-R) and clicking links frantically for a few minutes. Now take a look at the server graphics on your host. In my case, the CPU peaked at under just a few % while memory showed no significant change. This goes to show just how overkill this cheap box really is when running really simple sites like this.

Load times over a simulated 2G network and a VPN on the other side of the world are still snappy and feel borderline local. I went ahead and loaded the site up in `lynx` and it looks good on there too (the 0.001% of browser users will appreciate). These are the joys of HTML and CSS only.
