//teapot model

call: studio
call: lights
call: floor
call: teapot
call: ambient_occlusion
call: smoothing


function: studio
angle: 30
//
from: -10 8 -5
at: 0 1 0
//
up: 0 5 0
ambient: .9 .9 .9
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
light: -10 100 10
light-color: 1.0 1.0 1.0
endfunction:


function: floor
surf: .1 .5 .1
//surf-specular: .1 .1 .1
surf-perlin: .1 .45 .1  .1 .5 .1  2 2 2
surf-pattern: 2
box: LTRBFB 0 -1 0  100 2 100
endfunction:

function: teapot
trans-translate: 0 0 0
trans-rotate: 0 0 0
surf: 1 1 1
//surf-specular: .1 .1 .1
model: scenes/mug3.obj
trans-pop: 2
endfunction:



from: -10.000000 8.000000 -5.000000
  at: 54.498266 -53.370152 43.903375
angle: 15.000000
