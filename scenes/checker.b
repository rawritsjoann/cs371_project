//checker board example

call: setup_studio
call: setup_lighting
//call: checker_test
call: checker_red_yellow


function: setup_studio
from: 10 350 700
at: 0 0 0
angle: 30
up: 0 1 0
maxdepth: 10
minweight: 0.1
//montecarlo: 9
//adaptive: 0.01
endfunction:


function: setup_lighting
light: .1000 1000 .1000
light-color: .6 .6 .6
ambient: .4 .4 .4
background: 0 0 0
endfunction:


function: checker_test
checker:
80 0 80
.8 .8 .8  .6 .5 .4
checker-grout: .6 .4 .1 .03
//
surf: .01 .01 .01
surf-specular: .2 .2 .2
surf-pattern: 1
//
trans-translate: 0 -5.01 0
box: _T__F_  0 0 0  900 10 1500
trans-pop:
endfunction:


function: checker_red_yellow
from: 0 0 9.656854249
at: 0 0 0
angle: 45
up: 0 1 0
ambient: 1 1 1
background: 1 1 1
//
checker:
4 4 0
1 0 0  1 1 0
//checker-grout: .6 .4 .1 .03
surf: 1 1 1
surf-pattern: 1
//ring: 0 0 0  0 0 1   0 10
//box: TB__F_   0  0 0   8 8 .0001
box: _T_BF_   0  0 0   8 8 .0001
endfunction:

