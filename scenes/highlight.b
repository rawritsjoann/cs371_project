//specular highlight on glass ball

from: -90 210 800
at: 20 100 0

up: 0 1 0
angle: 30.0
ambient: .4 .4 .4
maxdepth: 18
//minweight: 0.1
background: .2 .2 .2
sky:
montecarlo: 13
adaptive: 0.01

light: 1000 1000 1000
light-color: .5 .5 .5
light-spec: 1
light-area: 100 2

surf:
surf-specular: .05 .05 .05
surf-transmit: 0 0 .01
surf-highlight: .5 .5 .5
surf-spot: 20
surf-iors: 1.5 1.0
sphere: 0 100 0  100

surf-copy:
surf-iors: 1.5 1.0
surf-insideout:
sphere: 0 100 0   95

//surf:
//surf-specular: .1 .1 .1
//surf-transmit: 1 1 1
//surf-iors: 1.0 1.0
//let: a == 200
//do: 8
//	box: TBLRFB  200 50 a   100 100 10
//	let: a -= 40
//enddo:


checker: 80 0 80  1 1 1  .8 .8 .8
surf:
surf-specular: .2 .2 .2
surf-pattern: 1
box: LTRBFB 0 -10 0  1200 20 1200


checker: 80 80 0  1 1 1  .9 .9 1
surf:
surf-pattern: 1
box: ____F_  0 0 -600   4800 4800 1


trans-translate: 140 0 -200
trans-rotate: 0 45 0
surf: 1 0 0
surf-specular: .1 .1 .1
surf-highlight: .5 .5 .5
surf-spot: 5
surf-perlin: .9 .6 .1   1 0 0   40 20 40
surf-pattern: 3
box: LTRBFB  0 150 0  80 300 80
trans-pop:


