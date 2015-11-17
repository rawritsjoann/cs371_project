//cloth dragged across scene containing a sphere.
//press key 'b' to run this animation.
//Gimp - open first image, import as layers on all others.
//       save as gif, check animation box

call: setup_animation
call: setup_camera
call: setup_lighting
call: floor


function: setup_animation
	cloth-center: 200 200 0
	cloth-velocity: 0 0 0
	cloth-width:  41
	cloth-height: 41
	cloth-step: 6
	cloth-stiffness: 0.05
	cloth-stretchiness: 2.0
	cloth-gravity: -0.008
	cloth-damping: 0.99
	cloth-spring-steps: 1
	cloth-grabbed: 0
	cloth-grabbed: 1
	cloth-grabbed: 1640  // w * (h-1)
	cloth-grabbed: 1641
	//cloth-smoothing:
	//
	nframes: 5000
	frameStart: 500
	frameStop:  500
	//modFrames: 22
	modFrames: 40
	//saveFiles:
	directory: anim_2
endfunction:


function: setup_camera
angle: 45
//
//high looking down
from: 0 740 .1
at: 0 0 0
//looking from cloth destination
from: -600 200  0
at:      0  60  0
//looking from cloth source
from:  350  60  0
at:      0  90  0
//regular
from: 10 140 500
at: 0 90 0
//
up: 0 1 0
maxdepth: 10
minweight: 0.1
sky:
endfunction:

//    color, density
//haze: .3 .3 1  0.0001
//haze-center: 0 0 -20000
//haze-reach: 2000 600 20000

//montecarlo: 5
//adaptive: 0.01

function: setup_lighting
ambient: .3 .3 .3
background: 0 0 0

light: 200 400 400
light-color: .5 .5 .5
endfunction:


function: floor
checker:  80 0 80  .9 .9 .9  .7 .7 .6
checker-grout: .5 .5 .5 .05
//
surf: .01 .01 .01
surf-specular: .2 .2 .2
surf-pattern: 1
//
//floor
trans-translate: 0 -.1 0
box: _T__F_  0 0 0  10000 .01 20000
trans-pop:
endfunction:

//ball
//surf: .3 .6 0
//surf-specular: .2 .2 .2
//sphere: 0 50 0 50

//red triangle
//surf: .9 .1 .1
//surf-specular: .1 .1 .1
//tri:
//0 200 0
//100 200 0
//0 200 -100

