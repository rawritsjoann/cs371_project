//mug shot

call: studio
call: lights
call: floor
call: rim
call: handle
call: pencil
//call: ambient_occlusion
//call: smoothing

function: studio
angle: 30
//
from: 0 35 25
at: 0 0 0
//
up: 0 30 0
ambient: .4 .4 .4
//
maxdepth: 10
background: .5 .5 .5
endfunction:

function: ambient_occlusion
//ambient-occlusion: 5 1.5
endfunction:


function: smoothing
//aperture: .5  15.85  17
montecarlo: 5
adaptive: 0.01
endfunction:


function: lights
light: 0 50 0
light-color: .2 .2 .2
light-spec: 10
light-area: 3 50

light: 0 50 10
light-color: .2 .2 .2
light-spec: 10
light-area: 3 50

light: -20 20 0
light-color: .3 .3 .5
light-spec: 10
light-area: 3 50
endfunction:

function: rim
trans-translate: 0 0 0
trans-rotate: 0 0 0
surf: 1 1 1
surf-specular: .1 .1 .1
model: scenes/rim.obj
trans-pop: 2
endfunction:

function: handle
trans-translate: 2.5 3.1 -1.5
trans-rotate: 0 210 0
surf: 1 1 1
surf-specular: .1 .1 .1
surf-highlight: 1 1 1
surf-spot: 80
model: scenes/handle1.obj
trans-pop: 2
endfunction:

function: pencil
trans-translate: -2 6 -.3
trans-rotate: 335 30 50
surf: 1.0 .5373 .02353
surf-specular: .1 .1 .1
model: scenes/pencil.obj
trans-pop: 2
trans-translate: -2 6 -.3
trans-rotate: 335 30 50
surf: .5 .5 .5 
surf-specular: .3 .3 .3
surf-highlight: 1 1 1
surf-spot: 30
model: scenes/top.obj
trans-pop: 2
trans-translate: -2 6 -.3
trans-rotate: 335 30 50
surf: .8 0.1 0.1
model: scenes/eraser.obj
trans-pop: 2
trans-translate: -2 6 -.3
trans-rotate: 335 30 50
surf: 1 .4 .4
model: scenes/eraser_u.obj
trans-pop: 2
endfunction:

// outside
trans-translate: 0 0.1 0
trans-rotate: 0 -15 0
surf: 1 1 1
surf-specular: .2 .2 .2
surf-texmap: scenes/mug_txtr.jpg
surf-cylindrical:
cylinder: 3 5.9
surf: .9 .9 .9
ring: 0 .5 0  0 .5 0  0 2.86
trans-pop: 2

// inside
trans-translate: 0 0.1 0
surf: 1 1 1
surf-insideout:
surf-specular: .2 .2 .2
cylinder: 2.86 5.9
surf: 1 1 1
surf-specular: 1 1 1
//ring: 0 .500001 0  0 .500001 0  2.8 2.85
trans-pop: 3

function: floor
surf: 1 0 0
//surf-specular: .2 .2 .2
//surf-pattern: 1
surf-texmap: scenes/table_txtr.jpg

tri:
-25 0  25
-25 0 -25
 25 0 -25
tri-texcoord: 0 .3  0 1  1 1
tri:
-25 0  25
 25 0 -25
 25 0  25
tri-texcoord: 0 .3 1 1  1 .3
endfunction:
from: 0.000000 35.000000 25.000000
  at: -15.790662 -37.663218 -45.318000
angle: 15.000000
