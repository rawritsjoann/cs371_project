//teapot model

call: studio
call: lights
call: floor
call: teapot
call: ambient_occlusion
call: smoothing


function: studio
angle: 30
from: 0 7 14
at: 0 2.5 0
//
from: 0 7 15
at: 0 0.90 0
//
up: 0 1 0
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
light: -200 100 100
light-color: .31 .31 .31
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
surf: .8 .6 .1
//surf-specular: .1 .1 .1
model: scenes/teapot.obj
trans-pop: 2
endfunction:



