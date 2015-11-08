//texture mapping
from: 2 12 40
at: 0 1 0
angle: 4

up: 0 1 0
ambient: 0.4 0.4 0.4
maxdepth: 10
minweight: 0.1
//background: 0.2 0.2 0.2
//sky:
montecarlo: 20
adaptive: 0.01

light: -400 1000 400
light-color: .6 .6 .4
//light-spec: 1
light-area: 600 3

checker:
1 0 1
1 1 1  .8 .8 .8

surf: .01 .01 .01
surf-specular: .2 .2 .2
surf-pattern: 1
//ring: 0 0 0   0 1 0   0 40
sphere: 0 -1000 0 1000

surf: 1 0 0
surf-specular: .2 .2 .2
surf-texmap: scenes/mug_txtr.jpg
surf-spherical:
sphere: 0 1 0 1


