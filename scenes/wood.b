//wood texture using Perlin noise

angle: 30.0
from: 100 400 1340
at: 0 0 140
up: 0 1 0
ambient: .2 .2 .2
maxdepth: 10
minweight: 0.5
//background: .2 .2 .2
sky:

//distribution:
//jitter: 0.0001
//montecarlo: 41
//aperture: 20 1680 17
montecarlo: 25
adaptive: 0.1

light: -300 2000 400
light-color: .1 .1 .1
light-area: 100 2

light: 2000 1000 100
light-color: .3 .3 .3
light-area: 200 2

light: -200 400 1000
light-color: .96 .96 .96
light-area: 20 2

//table top
surf: .7 .7 .1
surf-specular: .2 .2 .2
surf-perlin: 1.2 .7 0  .5 0 0  164 34 34
surf-pattern: 3
//box: T__RF_  0 -20 0   900 40  900
box: _TR_F_  0 -20 0   900 40  900

//wood grain box
trans-translate: -110 50 160
trans-rotate: 0 -20 0
surf: 0 0 0
surf-perlin: 1 .8 .5  .8 .6 .4  134 14 14
surf-pattern: 3
box: LTRBFB  0 0 0   300 50 200
trans-pop: 2

//translucent box
trans-translate: 180 55.001 280
trans-rotate: 0 -48 0
surf: .2 .2 0
surf-transmit: 0 0 0
surf-translucent: .03 2
surf-iors: 1.52 1.0
surf-pattern: 0
box: LTRBFB
0 0 0
180 100  20
trans-pop: 2

