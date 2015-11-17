//lighting test

call: camera_low
//call: camera_high
call: haze
call: area_lights
call: light_bar
call: floor
call: surface
//call: csub
call: objects


function: camera_low
from: 40 250 1100
at: 0 50 0
endfunction:

function: camera_high
from: 100 1850 101
at: 100 0 100
endfunction:

angle: 30

montecarlo: 33
adaptive: 0.05

up: 0 1 0
ambient: .1 .1 .1
//maxdepth: 10
//minweight: 0.1

background: 0 0 0
//sky:

function: haze
haze: 0 0 0  0.001
haze-center: 0 0 0
haze-reach: 10000 12 100000 
endfunction:


function: area_lights
light: -3000 800 -2000
light-color: .2 .2 .1
light-area: 140 2
light: -800 400 200
light-color: .3 .3 .2
endfunction:

function: light_bar
array: col [ 25 ] == {
 .1 .1 .1
 .1 .1 .1
 .1 .1 .1
 .15 .15 .15
 .4 .2 .02
 .4 .2 .02
 .4 .2 .02 }
trans-rotate: 0 45 0
trans-translate: 0 10 0
let: a == 0
let: b == 1
let: c == 2
let: x == -150
do: 7
	light: x 10 300
	light-color: col [ a ] col [ b ] col [ c ]
	light-spec: 1
	light-area: 15 2
	let: x += 50
	let: a += 3
	let: b += 3
	let: c += 3
enddo:
//the bar
surf: .2 .1 .01
trans-translate: 0 10 320
box: LTRBFB  0 0 0  300 30 10
trans-pop:
trans-pop: 2
endfunction:

function: floor
checker:  80 0 80  1 1 1  .8 .8 .8
surf: .01 .01 .01
surf-specular: .2 .2 .2
surf-pattern: 1
ring: 0 0 0   0 1 0   0 100000
endfunction:


function: csub
surf: .7 .7 .7
surf-specular: .4 .4 .4
surf-highlight: .62 .62 .62
surf-spot: 200
trans-scale: 200 200 200
//model: /home/gordon/Desktop/371material/models/csub.obj
model: /home/gordon/Desktop/371material/models/csubBevel3.obj
trans-pop:
endfunction:

function: surface
surf: .7 .7 .7
surf-specular: .4 .4 .4
surf-highlight: .62 .62 .62
surf-spot: 200
endfunction:

function: objects
sphere: 10 100 0 103
cone:     -140 0 0   80   0 200
trans-translate: 170 0 0
surf-copy:
cylinder:  60   140
trans-pop:
ring:      170 140 0   0 1 0   0 60
endfunction:


