//cube for 371 animation demo
//press e to run anim

call: setup_animation
call: setup_camera
call: setup_lighting

function: setup_animation
	nframes: 20
	//frameStart: 1
	//frameStop:  40
	//frameMod: 20
	//saveFiles:
	directory: anim1
endfunction:

function: setup_camera
from: 0 1.2 2.5
at: 0 0 0
angle: 45.0
up: 0 1 0
endfunction:

//antialias
//montecarlo: 60
//adaptive: 0.1

function: setup_lighting
background: .1 .1 .1
ambient: .4 .4 .4
light: 60 20 60
light-color: .7 .7 .7
endfunction:

surf: 1 1 0
//box: LTRBFB 0 0 0  1 1 1

