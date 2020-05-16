% Building A Markdown Site
% Michal
% 16/5/2020

# Building a Markdown Site

#### A really really basic one

## Why?

I'm sure you've come across high quality articles and tutorials on developer blogs. I find that the simplier the blog, the better the articles. 

So why not make one myself? I want something simple and quick. I don't want to use any frameworks or CMSes as that defies the concept of simple. Ideally a mostly static site.

At the same time, I would go insane if it was truly a static site and I was having to edit stupid HTML files everytime I want to add content.

Ideally I would like to dump a bunch or markdown files into a folder and have this site serve them up as HTML. Believe it or not, if you are reading this, the idea worked!

## Research

There might be something that does this already, but secretly I have already decided that I will build something.

The idea of a static site based on markdown is nothing new. There are a bunch of online tools that do this already:

1. [bookdown.org](https://bookdown.org/) - *seems to specialise in turning markdown into just about anything*
1. [blot.im](https://blot.im/) - *a very nice minimalist blog made up of just folders and markdown*
1.  [smallvictori.es](https://www.smallvictori.es) - *another blot, with less focus on markdown*

Both blot and smallvictori charge around £4 a month for the site. I can take the same $4 and get a server and builid my own solution.

I want to use C++ which is a language I currently work in. It's not the best tool for this kind of job, but I'm on my own time so nobody can stop me.

With C++ I get to choose fast and well suited dependencies. I want a straight forward, no-bullshit http server and something that can take markdown and make it into HTML (ideally with very little input from me).

For the markdown compiler I went with [Discount Markdown](http://www.pell.portland.or.us/~orc/Code/discount/). Its a C library with no external dependencies. It seems fast and well maintained. The documentation has a very enouraging level of cynicism and sarcasm in it. This, to me, is always a sign of a talent.

For the HTTP server I went with [cpp-httplib](https://github.com/yhirose/cpp-httplib). Its simple to use, handles static files really easily and doesn't seem particulalry bloated.

Depending on just how terrible my $4 worth of server will be, I might need to rething these dependencies.

## The Plan

Here is the idea. The content on pages will be written in markdown, but there are some parts that won't change. The header and the footer of page along with the html `<head>` section etc.

We should therefore inject our HTML content (compiled from markdown) into designated parts of a template.

We will need styling so the template will need pull in a static file. So the server needs to serve static files from a designated directory. This could alternatively be done though nginx (which probably does it better). We will see...

I want the website to feel dynamic. I push a markdown article and it should be instantly accessible. At the same time, we cannot be generating the html everytime the user asks for a webpage (in efficient and vulnerable). The best solution is to generate the page on demmand and then cache it. Whenever I update the page, I will need to reload the server to clear the cache. I could write an obscure endpoint for this, but since authentication would be too complicated to do here, it would only be "security through obscurity". Which is the shittest type of security. Maybe after "no security"...

## 

