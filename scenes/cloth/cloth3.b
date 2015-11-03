//table cloth on table.
//press key 'c' to run this animation.
//Gimp - open first image, import as layers on all others.
//       save as gif, check animation box

//table cloth falling on a table
cloth-center: -1 95 1
cloth-width:  81
cloth-height: 81
cloth-step: 2.7
cloth-stiffness: 0.29
cloth-stretchiness: 2.0
cloth-gravity: -0.009
cloth-damping: 0.98
cloth-spring-steps: 1
cloth-texture: scenes/cloth/tablecloth.png
cloth-tex-tile: 4 4
//cloth-grabbed: 0
//cloth-grabbed: 1
//cloth-grabbed: 39
//cloth-grabbed: 40
//cloth-smoothing:

nframes: 1500
frameStart: 20
frameStop:  900
frameMod: 22
//saveFiles:
directory: anim_3

angle: 45

//high looking down
from: 0 740 .1
at: 0 0 0

from: 10 160 240
at:   0 60 0

from: 10 10 240
at: 0 40 0

up: 0 1 0
ambient: .2 .2 .2
maxdepth: 10
minweight: 0.1
background: 0 0 0
sky:
//montecarlo: 9
//adaptive: 0.01

//    color, density
haze: .3 .3 1  0.0004
haze-center: 0 0 -20000
haze-reach: 2000 600 20000


light: 400 500 500
light-color: .8 .8 .6
//light-area: 100 4

light: -400 200 200
light-color: .3 .2 .1
//light-area: 60 4

checker:  80 0 80  .9 .9 .9  .7 .7 .6
checker-grout: .5 .5 .5 .05

surf: .01 .01 .01
surf-specular: .2 .2 .2
surf-pattern: 1

//floor
trans-translate: 0 -.1 0
box: _T__F_  0 0 0  10000 .01 20000
trans-pop:

//table top
/trans-translate: 0 80 0
/surf: .7 .7 .1
/surf-specular: .2 .2 .2
/surf-perlin: 1.2 .7 0  .5 .2 .2  40 10 10
/surf-pattern: 3
/cylinder:  70 15
/trans-translate: 0 15 0
/ring: 0 0 0  0 1 0  0 70
/trans-pop: 2

//table stand
trans-translate: 0 0 0
surf: .1 .1 .1
surf-specular: .8 .8 .8
cylinder:  5 80
ring: 0 5 0  0 1 0  0 24
cylinder:  24 5
trans-pop: 1


























