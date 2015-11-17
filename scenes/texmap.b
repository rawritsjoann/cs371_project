//demonstration of a texture map

angle: 45.0
from: 40 200 600
at: 0 80 0

up: 0 1 0
ambient: 0.5 0.5 0.5
maxdepth: 10
minweight: 0.1
background: 0.2 0.2 0.2
//sky:
//montecarlo: 19
//adaptive: 0.1

light: -400 1000 400
light-color: .5 .5 .3
light-spec: 1
light-area: 60 4




//textured surfaces
//  +---------+
//  |\        |
//  | \       |
//  |  \      |
//  |   \     |
//  |    \    |
//  |     \   |
//  |      \  |
//  |       \ |
//  |        \|
//  +---------+
surf:
surf-specular: .1 .1 .1
//surf-texmap: scenes/teapot.ppm
surf-texmap: scenes/table_txtr.ppm
tri:
-100   0  0
-100 200  0
 100   0  0
tri-texcoord: 0 1  0 .3  1 1
tri:
 100   0  0
-100 200  0
 100 200  0
tri-texcoord: 1 1  0 .3  1 .3


















