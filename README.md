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
