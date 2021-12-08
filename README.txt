Hi and welcome to the Reality plugin project.

This file is just an initial draft, no much here. Sorry, I'll try to add more info asa.

Reality is a project written mostly in C++. It runs as a plugin for both DAZ Studio and Poser. 

The DAZ Studio version uses the DS SDK in C++. The Poser version uses a hybrid system where the 
plugin proper is written in Python. That piece of software is very thin. Its main goal is to communicate
with Poser, given that the program only uses Python for plugin development. That is unfortunate on 
many fronts but there is realy nothing that we can do. The python layer is a broker between Poser
and Reality. 

Reality itself is broken down into two components: the UI and the working library. The UI communicates
with the plugin, either the DS or Poser one, using message passign protocol via the ZeroMQ library. 

ZMQ uses TCP/IP for message passing. See the documentation for that library for more details.

The first task that you will have to overcome is to compile Reality as it is. That is not easy.
Reality is designed to use a build system but the reality is that the build system that worked 
was the one finely tailored on my machines. Sorry again. It's not going to require rocket science 
but do not expect to run a couple of scripts and compile. You will probably need to adap things a 
bit, since the last time that I compiled the project was about 4 years ago.

Anyway, everything is based on CMake and, with a few tweaks to the CMake scripts you should be
able to get things going. 

Once you have the project compiled, you will need to run it. If it run then I suggest, as first 
task, to remove the DRM system. I will post more about how to find the components of it and howto 
disable it soon. 

See the file "HOW TO BUILD REALITY.txt" for other details.

Happy trails!

--
Paolo Ciccone
January 19th, 2020
