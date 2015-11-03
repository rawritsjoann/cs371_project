//example of cones and cylinders

angle: 30
from: 10 400 1200
at: 0 80 0

up: 0 1 0
maxdepth: 10
//montecarlo: 14
//adaptive: 0.01
//aperture: 200 1090 14

call: setup_lighting
call: floor
call: yellow_ball
call: hollow_cylinder
call: red_cone
call: green_cone
call: rings_around_cone
//call: haze
call: haze_layers

function: haze
haze: .9 .9 .9 .004
haze-center: 0 0 0
haze-reach: 20000 200 20000
endfunction:

function: haze_layers
let: r == .9
let: g == .9
let: b == .5
let: dens == .02
let: x == 0
let: y == 10
let: z == 0
let: height == 5
do: 40
	haze: r g b dens
	haze-center: x y z
	haze-reach: 200000 height 200000
	let: y += 10
	let: dens *= .78
	let: b += .05
	if: b >= .9
		let: b == .9
	endif:
	let: g *= .95
enddo:
endfunction:



function: setup_lighting
background: 0 0 0
ambient: .4 .4 .4
light: -400 1000 400
light-color: .6 .6 .6
//light-spec: 1
light-area: 20 4
endfunction:

function: floor
checker:  80 0 80  .8 .9 .9  .3 .6 .3
checker-grout:  .3 .3 .3  .05
surf: .1 .1 .1
surf-specular: .2 .2 .2
surf-pattern: 1
trans-translate: 0 -5 0
#trans-rotate: 0 .1 0
box: _T__F_  0 0 0  900 10 1500
trans-pop: 1
endfunction:

function: yellow_ball
surf: 1 1 0
surf-specular: .2 .2 .2
trans-translate: 210 100 0
sphere: 0 0 0 100
trans-pop: 1
endfunction:


function: hollow_cylinder
trans-translate: -200 1 0
surf: .5 .4 .1
surf-specular: .1 .1 .1
surf-perlin: .3 .2 .1  .8 .6 .3   80 20 80
surf-pattern: 3
light: 20 80 10
light-color: .9 .7 .3
cylinder: 80  100
cylinder: 100  100
ring: 0 100 0    0 1 0   80 100
surf: .4 .6 1
cone: 0 0 0  40  0 100
trans-pop: 1
endfunction:

function: red_cone
surf: 1 0 0
surf-specular: .2 .2 .2
cone: 40 0 0  100  0 200
endfunction:

function: green_cone
trans-translate: -80 0 120
surf: 0 1 0
surf-specular: .2 .2 .2
cone: 0 0 0 40 0 300
endfunction:


function: rings_around_cone

let: r == 45
let: o == 50
let: y == 18
let: i == 2
do: 10
	trans-translate: 0 y 0
	surf: 1 .7 .2
	surf-specular: .3 .3 .3
	cylinder:  o 8
	cylinder:  r 8
	ring: 0 0 0  0 -1 0  r o
	ring: 0 8 0  0  1 0  r o
	let: y += i
	let: i *= 0.5
	let: r *= 0.92
	let: o *= 0.92
enddo:

trans-pop: 10
endfunction:


