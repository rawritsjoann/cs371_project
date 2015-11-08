//teapot model

call: studio
call: lights
call: floor
call: rim
call: handle
//call: ambient_occlusion
//call: smoothing


function: studio
angle: 30
//
from: 5 15 20
at: 0 3 0
//
up: 0 30 0
ambient: .5 .5 .5
//
maxdepth: 10
background: 1 1 1
sky:
endfunction:

function: ambient_occlusion
ambient-occlusion: 5 1.5
endfunction:


function: smoothing
//aperture: .5  15.85  17
montecarlo: 5
adaptive: 0.01
endfunction:


function: lights
light: 10 30 0
light-color: .5 .5 .5
//light-area: 5 10
endfunction:

function: rim
trans-translate: 0 0 0
trans-rotate: 0 0 0
surf: 1 1 1
//surf-specular: .1 .1 .1
model: scenes/rim.obj
trans-pop: 2
endfunction:

function: handle
trans-translate: 2.9 3 0
trans-rotate: 0 180 0
surf: 1 1 1
//surf-specular: .1 .1 .1
model: scenes/handle.obj
trans-pop: 2
endfunction:

//a cylinder
trans-translate: 0 0.1 0
surf: 1 1 1
//surf-specular: .2 .2 .2
surf-texmap: scenes/mug_txtr.jpg
surf-cylindrical:
cylinder: 2.99 5.9
ring: 0 0 0  0 1 0  0 0
trans-pop: 1

//a cylinder
trans-translate: 0 0.1 0
surf: 1 1 1
//surf-specular: .2 .2 .2
cylinder: 2.85 5.9
ring: 0 0.1 0  0 1 0  0 2.81
trans-pop: 1

function: floor
surf: 1 0 0
//surf-specular: .2 .2 .2
//surf-pattern: 1
surf-texmap: scenes/table_txtr.jpg
//surf-perlin: .1 .45 .1  .1 .5 .1  2 2 2
//box: LTRBFB 0 0 0 13 0 15

tri:
-10 0  10
-10 0 -10
 10 0 -10
tri-texcoord: 0 .3  0 1  1 1
tri:
-10 0  10
 10 0 -10
 10 0  10
tri-texcoord: 0 .3 1 1  1 .3
endfunction:


