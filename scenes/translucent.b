//translucent glass

call: studio
call: lights
call: scene


function: studio
from: -280 800 2000
at: 60 0 0
angle: 30.0

//left
from: -28000 60000 200000
at: -80 160 0
angle: .3

//right
from: -28000 60000 200000
at: 80 160 0
angle: .3

up: 0 1 0
//
ambient: .1 .1 .1
//maxdepth: 16
background: .4 .4 .4
//
//jitter: 0.0009
//aperture: 10 2000 12
montecarlo: 15
adaptive: 0.01
endfunction:


function: lights
light: -3000 800 2000
light-color: .6 .6 .6
light-area: 1100 8
//
light: 3000 400 1000
light-color: .2 .2 .1
//light-area: 1340 2
endfunction:

function: scene
surf: 1 0 0
surf-specular: .2 .2 .2
sphere: -160 100 60  100

surf: .9 .9 .1
surf-specular: .2 .2 .2
sphere: 60 100 -1000  100

checker:
200 0 200   1 .8 .8   .9 .7 .7

surf: 1 .8 .8
surf-pattern: 1
ring: 0 0 0   0 1 0   0 19000

surf: .3 .3 .3
surf-specular: .2 .2 .2
surf-transmit: 0 0 0
surf-translucent: 0.1 4
surf-iors: 1.1 1.0
//center, w,h,d
box: LTRBFB   0 200.1 200  300 400 80
endfunction:

