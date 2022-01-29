# CA-Liquid
Rather simple liquid simulator using cellular automaton model. Everything is rendered using PixelGameEngine.
This project helped me improve my C++ skills and develop more understanding in cellular automata algorithms.

## Table of Contents
* [General info](#general-info)
* [Technologies](#technologies)
* [Setup](#setup)
* [Sources](#sources)

## General info
Compiled project consist of one one .json file and one .exe file that displays interactive window. Within the window user can create water and solid blocks that properly interact with each other,
creating fluid simulation. Window can be adjusted using option in config.json file so, no recompiling is needed after every change. Blocks are actually decals,
type of sprite that lives in GPU memory and is fully controlled by GPU. That means no CPU power is needed to render visuals aspects. Therefore CPU can be fully focused on calculations.

## Technologies
 1. C++17
 2. [json.hpp](https://github.com/nlohmann/json) 3.10.5
 3. [olc::PixelGameEngine](https://github.com/OneLoneCoder/olcPixelGameEngine) 2.16
 
 ## Setup
  ### Compiled file
   After configuration proper settings in config.json file just open Simulator.exe
  ### Compiling file
   TODO makefile
   
## Sources
1. [Overall cellular automaton model idea for such simulations](https://w-shadow.com/blog/2009/09/01/simple-fluid-simulation)
2. [More developed simulation model on unity engine](http://www.jgallant.com/2d-liquid-simulator-with-cellular-automaton-in-unity)
3. [OneLoneCoder / javidx9](https://github.com/OneLoneCoder)'s video ["olc::PixelGameEngine 2.0"](https://www.youtube.com/watch?v=8OfgGUGP4Vc)
