//glass box

from: -30 0 15
from: -12 20 45
from: 32 42 52

//from: 32 19 52
//aperture: 1 62 12

from: -18 32 82
//aperture: 4 86 21

at: -8 0 0
angle: 20.0
up: 0 1 0
maxdepth: 8
minweight: 0.1
ambient: .1 .1 .1
background: .2 .2 .2
sky:

call: setup_lights
call: floor
call: glass_box
call: glass_ball


function: setup_lights
light: -1000 1000 1000
light-color: .5 .5 .5

light: 1000 2000 500
light-color: .3 .3 .3

light: 1000 1000 -500
light-color: .1 .1 .1

//light: -1000 1000 1000
//light-color: .2 .2 .2
//light: -1000 1000 -1000
//light-color: .2 .2 .2
//light:  1000 1000 -1000
//light-color: .2 .2 .2
//light:  1000 1000 1000
//light-color: .2 .2 .2
endfunction:

function: floor
checker: 4 4 4  1 1 1  .8 .8 .6
checker-grout: .6 .5 .2 .08
trans-rotate: 0 10 0
surf:
//surf-perlin: .4 .4 .9  .86 .86 .6   32 2 2
surf-pattern: 1
box: _T____   0 -6.5001 0  400 1 400
trans-pop:
endfunction:

//surf: 1 0 0
//surf-specular: .1 .1 .1
//sphere: 0 -1.5 -11 5

//trans-translate: 0 -1 0
//trans-scale: .5 .5 .5

function: glass_box
surf: .1 .1 .1
surf-specular: .1 .1 .1
surf-transmit: 1 1 1
surf-iors: 1.4914 1.0

box: _T____   0  1  0  10 10 10
box: ___B__   0 -1  0  10 10 10
box: L_____  -1  0  0  10 10 10
box: __R___   1  0  0  10 10 10
box: ____F_   0  0  1  10 10 10
box: _____B   0  0 -1  10 10 10

//surf: 1 1 1
//sphere: 0 0 0 2

//front right
trans-translate: 5 -5 5
clip: 0 0 0  -1 0 0
clip: 0 0 0  0 0 -1
/surf: 1 0 0
surf: .1 .1 .1
surf-specular: .1 .1 .1
surf-transmit: 1 1 1
surf-iors: 1.4914 1.0
cylinder: 1 10
/surf: 2 1 0
clip: 0 10 0  0 -1 0
sphere: 0 10 0 1
clip-pop: 3
trans-pop:


//front left
trans-translate: -5 -5 5
clip: 0 0 0  1 0 0
clip: 0 0 0  0 0 -1
/surf: 1 0 0
surf: .1 .1 .1
surf-specular: .1 .1 .1
surf-transmit: 1 1 1
surf-iors: 1.4914 1.0
cylinder: 1 10
/surf: 2 2 0
clip: 0 10 0  0 -1 0
sphere: 0 10 0 1
clip-pop: 3
trans-pop: 1

//front bottom
trans-rotate: 0 0 90
trans-translate: -5 -5 5
clip: 0 0 0  1 0 0
clip: 0 0 0  0 0 -1
/surf: 1 0 0
surf: .1 .1 .1
surf-specular: .1 .1 .1
surf-transmit: 1 1 1
surf-iors: 1.4914 1.0
cylinder: 1 10
/surf: 2 2 0
clip: 0 10 0  0 -1 0
sphere: 0 10 0 1
clip-pop:
clip: 0 0 0  0 0 -1
clip: 0 0 0  0 1 0
sphere: 0 0 0 1
clip-pop: 4
trans-pop: 2

//front top
trans-rotate: 0 0 90
trans-translate: 5 -5 5
/surf: 1 0 0
surf: .1 .1 .1
surf-specular: .1 .1 .1
surf-transmit: 1 1 1
surf-iors: 1.4914 1.0
clip: 0 0 0  0 0 -1
clip: 0 0 0  -1 0 0
cylinder: 1 10
clip-pop: 2
trans-pop: 2

//top left
trans-translate: -5 5 5
trans-rotate: -90 0 0
/surf: 0 0 1
surf: .1 .1 .1
surf-specular: .1 .1 .1
surf-transmit: 1 1 1
surf-iors: 1.4914 1.0
clip: 0 0 0  0 0 -1
clip: 0 0 0  1 0 0
cylinder: 1 10
clip: 0 10 0  0 -1 0
/surf: 0 2 2
sphere: 0 10 0 1
clip-pop: 3
trans-pop: 2

//bottom left
trans-translate: -5 -5 5
trans-rotate: -90 0 0
/surf: 0 0 1
surf: .1 .1 .1
surf-specular: .1 .1 .1
surf-transmit: 1 1 1
surf-iors: 1.4914 1.0
clip: 0 0 0  0 0 1
clip: 0 0 0  1 0 0
cylinder: 1 10
clip: 0 10 0  0 -1 0
/surf: 0 2 2
sphere: 0 10 0 1
clip-pop: 3
trans-pop: 2

//top right
trans-translate: 5 5 5
trans-rotate: -90 0 0
/surf: 0 0 1
surf: .1 .1 .1
surf-specular: .1 .1 .1
surf-transmit: 1 1 1
surf-iors: 1.4914 1.0
clip: 0 0 0  0 0 -1
clip: 0 0 0  -1 0 0
cylinder: 1 10
clip: 0 10 0  0 -1 0
/surf: 0 2 2
sphere: 0 10 0 1
clip-pop: 2
trans-pop: 2

//bottom right
trans-translate: 5 -5 5
trans-rotate: -90 0 0
/surf: 0 0 1
surf: .1 .1 .1
surf-specular: .1 .1 .1
surf-transmit: 1 1 1
surf-iors: 1.4914 1.0
clip: 0 0 0  0 0 1
clip: 0 0 0  -1 0 0
cylinder: 1 10
clip: 0 10 0  0 -1 0
/surf: 2 2 0
sphere: 0 10 0 1
clip-pop: 2
trans-pop: 2

//back right
trans-translate: 5 -5 -5
clip: 0 0 0  -1 0 0
clip: 0 0 0  0 0 1
/surf: 0 1 0
surf: .1 .1 .1
surf-specular: .1 .1 .1
surf-transmit: 1 1 1
surf-iors: 1.4914 1.0
cylinder: 1 10
clip-pop: 2
trans-pop: 1

//back left
trans-translate: -5 -5 -5
clip: 0 0 0  1 0 0
clip: 0 0 0  0 0 1
/surf: 0 1 0
surf: .1 .1 .1
surf-specular: .1 .1 .1
surf-transmit: 1 1 1
surf-iors: 1.4914 1.0
cylinder: 1 10
clip-pop: 2
trans-pop: 1

//back top
trans-rotate: 0 0 90
trans-translate: 5 -5 -5
/surf: 0 1 0
surf: .1 .1 .1
surf-specular: .1 .1 .1
surf-transmit: 1 1 1
surf-iors: 1.4914 1.0
clip: 0 0 0  0 0 1
clip: 0 0 0  -1 0 0
cylinder: 1 10
clip-pop: 2
trans-pop: 2

//back bottom
trans-rotate: 0 0 90
trans-translate: -5 -5 -5
/surf: 0 1 0
surf: .1 .1 .1
surf-specular: .1 .1 .1
surf-transmit: 1 1 1
surf-iors: 1.4914 1.0
clip: 0 0 0  0 0 1
clip: 0 0 0  1 0 0
cylinder: 1 10
clip-pop: 2
trans-pop: 2

trans-pop:
endfunction:


function: glass_ball
trans-translate: -15 0 0
sphere: 0 0 0 8

surf-copy:
surf-iors: 1.4914 1.0
surf-insideout:
sphere: 0 0 0   7

trans-pop:
endfunction:













