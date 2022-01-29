# CA-Liquid
Rather simple liquid simulator using cellular automaton model. Everything is rendered using PixelGameEngine.
This project helped me improve my C++ skills and develop more understanding in cellular automata algorithms.

## Table of Contents
* [General info](#general-info)
* [How to use](#how-to-use)
* [Technologies](#technologies)
* [Setup](#setup)
* [Sources](#sources)

## General info
Compiled project consist of one one .json file and one .exe file that displays interactive window. Within the window user can create water and solid blocks that properly interact with each other,
creating fluid simulation. Window can be adjusted using option in config.json file so, no recompiling is needed after every change. Blocks are actually decals,
type of sprite that lives in GPU memory and is fully controlled by GPU. That means no CPU power is needed to render visuals aspects. Therefore CPU can be fully focused on calculations.

## How to use
Left mouse button - Adds water blocks

Right mouse button - Adds solid blocks

Middle mouse button - Deletes blocks
<br />
<br />
R - Resets the area to its original state

P - Resets parameters to their original values
<br />
<br />
↑ - Przełącza aktualnie wybrany parametr o jedną pozycję wyżej

↓ - Switches the currently selected parameter one position lower 

→ - Increases the value of the currently selected parameter

← - Decreases the value of the currently selected parameter


### Interface

*Compression* - Simulation model, to properly visualize flow of watter, assumes that the water is compressible. This parameter controls how much. The larger it is, the more water is able to fit into the one water cell that is pressed by other water cells.

*Flow divider* - Parameter controllig how fast the water flows to the next cells.
The larger it is, the slower it does.

*Steps per frame* - The number of iterations over all cells, calculating their successive states, before
rendering the frame. The larger the parameter is, the smoother the animation but at the same time
the more loaded processor, which may result in a significant FPS drop.

*Brush size* - The length of the side of a square which is the field of currently added / removed
blocks. For example, when a parameter is 2, blocks of water added with single click is a 2x2 square.

*Draw lines* - Solid blocks drawing mode. When it's off, solid block are drawn in the same way as water block, i.e they are added at the point of mouse click. When the mode is turned on, the first click decides of the starting point - A. The seconds click leads a line of block from A to the currently clicked position.

## Technologies
 1. C++17
 2. [json.hpp](https://github.com/nlohmann/json) 3.10.5
 3. [olc::PixelGameEngine](https://github.com/OneLoneCoder/olcPixelGameEngine) 2.16
 
 ## Setup
  ### Compiled file
   After configuration proper settings in config.json file just open Simulator.exe
   
## Sources
1. [Overall cellular automaton model idea for such simulations](https://w-shadow.com/blog/2009/09/01/simple-fluid-simulation)
2. [More developed simulation model on unity engine](http://www.jgallant.com/2d-liquid-simulator-with-cellular-automaton-in-unity)
3. [OneLoneCoder / javidx9](https://github.com/OneLoneCoder)'s video ["olc::PixelGameEngine 2.0"](https://www.youtube.com/watch?v=8OfgGUGP4Vc)
