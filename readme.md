# comp6841: rootkit

This project is based off of [Designing BSD Rootkits]( 
http://www.nostarch.com/rootkits.html) by Joseph Kong

There are two LKM's (Loadable Kernel Modules, aka KLD):

* Keylogger which saves characters to /tmp/arf
* mkdir which changes uid to 0 (making you root) when `mkdir abra` is given

## group members

Nate Henry  
Maxwell Lambert  
Victor Naxter  
Kaylen Payer  
Cameron Panzenbock  
Luke Cusack  

## slides

https://docs.google.com/presentation/d/1nhPdAhXu8IdGny1L_wdcAC0VnIXQ999YbyPJ-H9Vid4/edit?ts=59101fb7#slide=id.g1e1d5047a6_0_1

## usage

this project was tested on [FreeBSD 6](http://ftp-archive.freebsd.org/pub/FreeBSD-Archive/old-releases/i386/ISO-IMAGES/6.0/), download discs 1 & 2. if you're a UNSW CSE student you'll be able to get a free version of VMWare to run it on, or virtual box is also an option.

run `make` in the directories of the `.c` files, and then `sudo kldload ./<module>.ko` to load the kernel modules. `sudo kldunload <module>.ko` to unload them
