//cloth falling on sphere
//press key 'a' to run this animation.

call: setup_animation
call: setup_camera
call: setup_lighting
call: floor
call: setup_haze

//maxdepth: 10
//minweight: 0.1

//montecarlo: 5
//adaptive: 0.01


function: setup_animation
	cloth-center: 220 120 0
	cloth-velocity: -1.3 2 0
	//cloth-width:  100
	//cloth-height: 100
	//cloth-step: 2.8
	cloth-width:  50
	cloth-height: 50
	cloth-step: 5.6
	cloth-stiffness: 0.25
	cloth-stretchiness: 1.1
	cloth-gravity: -0.009
	cloth-damping: 0.995
	cloth-spring-steps: 1
	//cloth-texture: scenes/plaid3.ppm
	//cloth-tex-tile: 2 2
	//cloth-grabbed: 0
	//cloth-grabbed: 49
	//cloth-grabbed: 2450
	//cloth-grabbed: 2499
	//cloth-smoothing:
	//
	nframes: 1600
	frameStart: 10
	frameStop:  600
	frameMod: 20
	//saveFiles:
	directory: animx
endfunction:


function: setup_camera
//from: 10 350 600
from: 0 190 400
at:   0  80   0
up: 0 1 0
angle: 45
endfunction:


function: setup_haze
//    color, density
haze: .3 .3 1  0.001
haze-center: 0 0 -20000
haze-reach: 2000 600 20090
endfunction:


function: setup_lighting
background: 0 0 0
ambient: .4 .4 .4
sky:
light: 700 1000 600
light-color: .7 .7 .7
endfunction:


function: floor
//checker:  80 0 80  .1 .1 .1  .3 .3 .1
//checker-grout: .05 .05 .05 .04
surf: .8 .8 .7
//surf-specular: .2 .2 .2
//surf-pattern: 1
//floor
trans-translate: 0 -.1 0
box: _T__F_  0 0 0  10000 .01 20000
trans-pop:
endfunction:

//detailed floor
//let: x == -80
//let: x *= 18
//let: x += 40
//do: 36
//	let: z == 80
//	let: z *= 2
//	let: z += 40
//	do: 36
//		trans-translate: x -50 z
//		surf: .9 .9 .9
//		box: T_LRF_  0 0 0  76 100 76
//		trans-pop:
//		let: z -= 160
//	enddo:
//	let: x += 160
//enddo:
//
//let: x == -80
//let: x *= 17
//let: x += 40
//do: 36
//	let: z == 80
//	let: z *= 3
//	let: z += 40
//	do: 36
//		trans-translate: x -50 z
//		surf: .9 .9 .9
//		box: T_LRF_  0 0 0  76 100 76
//		trans-pop:
//		let: z -= 160
//	enddo:
//	let: x += 160
//enddo:
//
//let: x == -80
//let: x *= 17
//let: x += 40
//do: 36
//	let: z == 80
//	let: z *= 2
//	let: z += 40
//	do: 36
//		trans-translate: x -50 z
//		surf: .7 .7 .6
//		box: T_LRF_  0 0 0  76 100 76
//		trans-pop:
//		let: z -= 160
//	enddo:
//	let: x += 160
//enddo:
//
//let: x == -80
//let: x *= 18
//let: x += 40
//do: 36
//	let: z == 80
//	let: z *= 3
//	let: z += 40
//	do: 36
//		trans-translate: x -50 z
//		surf: .7 .7 .6
//		box: T_LRF_  0 0 0  76 100 76
//		trans-pop:
//		let: z -= 160
//	enddo:
//	let: x += 160
//enddo:





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

