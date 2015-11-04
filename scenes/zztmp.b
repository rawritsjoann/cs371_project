//teapot model

call: studio
call: lights
call: floor
call: teapot
//call: ambient_occlusion
//call: smoothing


function: studio
angle: 30
//
from: 4 8 15
at: 0 3 0
//
up: 0 30 0
ambient: .5 .5 .5
//
maxdepth: 10
background: 0 0 0
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
light: 1 30 5
light-color: .5 .5 .5
endfunction:


function: floor
surf: 1 0 0
//surf-specular: .2 .2 .2
surf-pattern: 1
surf-texmap: scenes/table_txtr.jpg
//surf-perlin: .1 .45 .1  .1 .5 .1  2 2 2
box: LTRBFB 0 0 0 13 0 15
endfunction:

function: teapot
trans-translate: 0 0 0
trans-rotate: 0 0 0
surf: 1 1 1
//surf-specular: .1 .1 .1
model: scenes/test1.obj
trans-pop: 2
endfunction:



from: 0.000000 0.000000 0.000000
  at: 0.000000 0.000000 100.000000
angle: 0.000000
