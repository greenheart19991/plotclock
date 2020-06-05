# Plotclock

A robotic clock that writes current time with a marker.  

The way it functions is simple: the robot draws the time
on every new minute. At the beginning of a minute previously
drawn time becomes erased and new current one appears
on the board again.

<p align="center">
  <img width="500" src="https://raw.githubusercontent.com/greenheart19991/plotclock/assets/cover.jpg">
</p>

## Demo

<p align="center">
  <img width="500" src="https://raw.githubusercontent.com/greenheart19991/plotclock/assets/demo_video.gif">
</p>

## Table of Contents

* [Prerequisites](#prerequisites)
    - [Software](#software)
    - [Hardware & Supplies](#hardware--supplies)
* [Getting Components Ready](#getting-components-ready)
* [Assembling](#assembling)
    - [Mechanics](#mechanics)
    - [Electronics](#electronics)
        + [Prepare RTC Module](#prepare-rtc-module)
        + [Setup Voltage Regulator](#setup-voltage-regulator)
        + [Wiring Diagram](#wiring-diagram)
    - [Assembled Project Example](#assembled-project-example)
* [Firmware](#firmware)
    - [Installation](#installation)
    - [Calibration](#calibration)
* [Operating Details](#operating-details)
    - [Positioning To a Specific Point](#positioning-to-a-specific-point)
    - [Drawing Chars](#drawing-chars)
    - [Boundaries](#boundaries)
* [Customization](#customization)
* [Known Issues](#known-issues)
* [Acknowledgments](#acknowledgments)

## Prerequisites

### Software
* Arduino IDE;
* KOMPAS-3D v18 in case you want to customize components.

### Hardware & Supplies
* 3d-printer to print the components;
* Arduino UNO (any Rev);
* SG90 servo motors - x3;
* LM2596S step-down voltage regulator;
* ChronoDot V2.0 real time clock module;
* 4.7k Ohm resistors - x2 (for ChronoDot module);
* CR2016 battery;
* 5.5 x 2.1mm DC Female to 2 Male power splitter cable;
* 5.5 x 2.1mm power jack socket;
* DC power supply 12V 2A;
* Whiteboard marker (dry-wipe)
* M3 x 12mm pan head screws - x8; 
* M3 x 16mm pan head screws - x1;
* M2.5 x 16mm pan head screws - x7; 
* M3 nuts - x9;
* M2.5 nuts - x7;
* M3 washers - x7;
* M2.5 washers - x1;
* SG90 default screws;
* Jumper wires;
* Soldering iron, solder, flux;
* Some glue;
* Some screws and nuts to fix the circuits.

## Getting Components Ready

It is designed to print all of the robot components with 3d-printer.
But you are free to choose another way to produce them.

You can find 3d-models of that components under **plotclock_details**
directory. There are both separate files for each component
and **all_in_one** file that contains all of them.
Both exported to STL format and are ready to print.

## Assembling

### Mechanics

To assemble the robot from printed components use the pictures below.
They provide details about the main stages of robot assembling.

<p align="center">
  <img width="500" src="https://raw.githubusercontent.com/greenheart19991/plotclock/assets/assembling/mechanics/1.jpg">
</p>

<p align="center">
  <img width="500" src="https://raw.githubusercontent.com/greenheart19991/plotclock/assets/assembling/mechanics/2.jpg">
</p>

<p align="center">
  <img width="500" src="https://raw.githubusercontent.com/greenheart19991/plotclock/assets/assembling/mechanics/3.jpg">
</p>

Before joining default servo levers to servos themselves
(on the pictures above they have already been joined), set
servo shafts in positions according to the following list:
* servo bottom - all the way to the end of min angle;
* servo right - all the way to the end of min angle;
* servo left - all the way to the end of max angle.  

Next, without rotating shafts, attach servo levers to servos
exactly as on the pictures below. After that you can rotate servo
shafts as you want and join printed levers.

<p align="center">
  <img width="500" src="https://raw.githubusercontent.com/greenheart19991/plotclock/assets/assembling/mechanics/4.jpg">
</p>

<p align="center">
  <img width="500" src="https://raw.githubusercontent.com/greenheart19991/plotclock/assets/assembling/mechanics/5.jpg">
</p>

When joining printed levers using screws and nuts, glue nuts to
levers to prevent them from loosening.

<p align="center">
  <img width="500" src="https://raw.githubusercontent.com/greenheart19991/plotclock/assets/assembling/mechanics/6.jpg">
</p>

<p align="center">
  <img width="500" src="https://raw.githubusercontent.com/greenheart19991/plotclock/assets/assembling/mechanics/7.jpg">
</p>

<p align="center">
  <img width="500" src="https://raw.githubusercontent.com/greenheart19991/plotclock/assets/assembling/mechanics/8.jpg">
</p>

<p align="center">
  <img width="500" src="https://raw.githubusercontent.com/greenheart19991/plotclock/assets/assembling/mechanics/9.jpg">
</p>

<p align="center">
  <img width="500" src="https://raw.githubusercontent.com/greenheart19991/plotclock/assets/assembling/mechanics/10.jpg">
</p>

<p align="center">
  <img width="500" src="https://raw.githubusercontent.com/greenheart19991/plotclock/assets/assembling/mechanics/11.jpg">
</p>

<p align="center">
  <img width="500" src="https://raw.githubusercontent.com/greenheart19991/plotclock/assets/assembling/mechanics/12.jpg">
</p>

Glue a piece of either cloth or cotton to eraser.

<p align="center">
  <img width="500" src="https://raw.githubusercontent.com/greenheart19991/plotclock/assets/assembling/mechanics/13.jpg">
</p>

<p align="center">
  <img width="500" src="https://raw.githubusercontent.com/greenheart19991/plotclock/assets/assembling/mechanics/14.jpg">
</p>

### Electronics

#### Prepare RTC Module

1. Solder both of pin headers into ChronoDot v2.0 RTC module.
1. Solder both of resistors into the module.
1. Install CR2016 battery on the module.

<p align="center">
  <img width="500" src="https://raw.githubusercontent.com/greenheart19991/plotclock/assets/assembling/electronics/prepared_rtc_module.jpg">
</p>

#### Setup Voltage Regulator

Put 12V into the LM2596S voltage regulator and adjust it down
to make an output voltage 5.5V.  

#### Wiring Diagram

Connect all the modules together according to the wiring diagram
below.

<p align="center">
  <img width="500" src="https://raw.githubusercontent.com/greenheart19991/plotclock/assets/assembling/electronics/wiring_diagram.jpg">
</p>

To prevent PCBs from moving make something kind of platform or
box for them and fix PCBs on it with screws and nuts.

### Assembled Project Example

<p align="center">
  <img width="500" src="https://raw.githubusercontent.com/greenheart19991/plotclock/assets/assembling/assembled_project_example.jpg">
</p>

## Firmware

Sketch and its dependencies are located under **plotclock_code**
directory.

### Installation

To install dependencies copy all libraries from
**plotclock_code/libraries** directory to your machine's Arduino
libraries directory. Now you are ready to compile and upload
the sketch.

### Calibration 

For correct marker positioning you should calibrate servo values
for predefined positions. To do this, set appropriate values for
next macros of the sketch:
* `SERVO_LEFT_ANGLE_180` - left servo value for position when
its lever rotated 180 degrees and is parallel to the board;
* `SERVO_LEFT_ANGLE_90` - left servo value for position when
its lever rotated 90 degrees and is perpendicular to the board;
* `SERVO_RIGHT_ANGLE_0` - right servo value for position when
its lever rotated 0 degrees and is parallel to the board.

This lever positions are shown in pictures below.

<p align="center">
  <img width="500" src="https://raw.githubusercontent.com/greenheart19991/plotclock/assets/firmware/1.jpg">
</p>

<p align="center">
  <img width="500" src="https://raw.githubusercontent.com/greenheart19991/plotclock/assets/firmware/2.jpg">
</p>

To calibrate vertical marker positioning, change the following
macros of bottom servo values:
* `SERVO_BOTTOM_PV_WR_BOTTOM` - position of marker when it touches
the board and chars becomes written;
* `SERVO_BOTTOM_PV_WR_UP` - position of marker to move it between
chars without writing a line;
* `SERVO_BOTTOM_PV_ERS_BOTTOM` - marker is in the eraser;
* `SERVO_BOTTOM_PV_FULLUP` - marker is above the eraser. 

After calibrating and uploading the sketch the robot can be
considered fully assembled and good to go. Since RTC module contains
the battery you don't have to reupload the sketch whenever
you turn the power off to setup the actual time - the module
keeps track of it by itself.

## Operating Details

This section describes provided solution for drawing figures
and intended to help you to deal with sketch and customize
it if necessary.

### Positioning To a Specific Point

To move marker to a specific point of a drawing plane
you have to rotate servo shafts through a specific angle.
In order to draw a char you have to perform a series
of movements to a specific points, and for each of them
you have to have rotation angles for both servo left
and right shafts.

To provide an ability to move the marker specifying
position of a particular point and not the rotation angles,
a virtual coordinate plane was defined. The origin of the
plane is located relative to the servo shafts axes
and it's position is configurable.

As a number lines unit, millimeter was taken. It thus
allows you to understand the physical location of particular
point without additional units conversion.

This coordinate plane is shown on picture below.
Blue, red and green colors indicate the corresponding
parts of the 'arms' lever system (top view) for one
of the possible marker positions. Point D here is a marker
position, and points A and B indicate servo shafts axes.
The AK distance is equal to the length of the AF lever.
Distances OL and LN are customizable and added to provide
ability to configure origin position.

<p align="center">
  <img width="500" src="https://raw.githubusercontent.com/greenheart19991/plotclock/assets/operation_details/coordinate_plane.jpg">
</p>

Since all of the valuable distances between points are known
and distances MD and DN can be calculated from provided x and y
coordinates, it is possible to calculate the angles FAB and ABG
that represent either angles of the servos or those from which
they can be obtained (servo left angle = FAB, servo right
angle = 180 - ABG).

Algorithm to find these values is implemented by `getServoAngles`
function. To move the marker `moveTo` function is used and uses
`getServoAngles` within itself.

### Drawing Chars

Since coordinate plane is defined, you can describe any figure
you want on it with a function. And, as soon as you define a
function, you can draw this figure by iterating over axis values,
passing them to a function and thus getting the coordinates
of points that figure outline consists of.

Exactly this way all chars becomes drawn. But, in order to simplify
functions, chars are described as a set of functions of several primitives
on the plane. These are lines, circles and ellipses.

For example, number '1' consists of 2 lines, while number '8' is
2 ellipses. Hence, to draw these numbers the robot sequentially
draw their primitives.

### Boundaries

Due to design, not all of the board surface area are available
for drawing. For current design all the points that the robot can
positions relative to coordinate plane is shown on picture below.

This area is indicated in red. Also, the board was added to
the picture according to its actual location. Black
rectangle indicates the max area determined for drawing chars.

<p align="center">
  <img width="500" src="https://raw.githubusercontent.com/greenheart19991/plotclock/assets/operation_details/coverage_map.png">
</p>

Take into account this information if you want to customize
chars, their position or size, or draw your own figures. 

## Customization

You can find editable models of the robot components under
**plotclock_details/kompas_models** directory. 

The project also provides a tool to show available drawing
area (described above) for a specific design setup. This
tool is located under **plotclock_design** directory and
can help you determine sizes or positions of different
components for new design and define drawing area boundaries.

## Known Issues

SG90 are pretty chip servos and cannot provide guaranteed
ability to rotate their shaft through an angle lower then
~0.75 degree under the marker weight. Therefore quite big
positioning errors occur. As a result there are jagged lines
and currently no ability of physically accurate positioning.

To solve it, more accurate and expensive servos should be
used to give an ability to rotate shafts through at
least 0.1 degree.

## Acknowledgments

Thanks to [joo](https://www.thingiverse.com/joo/about)
from [Thingiverse](https://www.thingiverse.com/) for idea
and initial design of components. See original project
[here](https://www.thingiverse.com/thing:248009#Instructions).
