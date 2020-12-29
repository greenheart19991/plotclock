# Plotclock Design

A tool to show available drawing area for a specific design setup
of the Plotclock.

It can be useful in case you are customizing existing components
or creating your own design. It can help you determine sizes of
these components as well as their positions and drawing area
boundaries.

## Usage

To start the tool open index.html in browser. You'll see
all the points to that marker can be positioned for currently
set robot design.

To edit components settings, edit corresponding constants in
*plotclock.js* file and refresh the page to update the area.

In case available drawing area for some reason was not be rendered
check your design setup and see occurred errors in browser console.

### Component Settings

All the constants are named according to points and segments
described in 'Operating Details' section of main README.
Take a look on the figure provided in this section to determine
constant components.

### Display Settings

You can also change some display behaviour. Main setting here
is `pxInUn` that specifies number of display pixels per
coordinate plane unit. The larger value you set, the larger image
will be, and the higher resolution you will have. You can find this
setting under `show points on page` section in code. 

### Overlay Components 

The tool also provides a Photoshop mockup that can simplify
you putting the board outline and eraser on created image
and determining the chars drawing area. You can find this
mockup as well as example result of tool usage
(for current design setup) under **coverage_maps** directory. 

## Known Issues

Due to representation of numbers in computer, calculation
errors take place. Thus for some **correct** design settings
an error can be thrown and coverage map will not be rendered.
In such situation in console you'll see an error like this:

<p align="center">
  <img width="500" src="https://raw.githubusercontent.com/greenheart19991/plotclock/assets/plotclock_design/calc_error.jpg">
</p>

Currently, to work around this issue you should change
your design settings a little bit, e.g. for one tenth of mm.   
