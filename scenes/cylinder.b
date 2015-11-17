//cylinder

from: 0 30 100
at: 0 10 0
//from: 0 100 1
//at: 0 0 0
angle: 45

up: 0 1 0
ambient: .4 .4 .4
background: 0 0 0

//light: 1000 1000 1000
//light-color: .6 .6 .6

checker:  5 0 5  .9 .9 .9  .1 .1 .1
surf: .01 .01 .01
surf-specular: .1 .1 .1
surf-pattern: 1
ring: 0 0 0  0 1 0  0 1000


trans-rotate: 0 30 0
trans-translate: 5 0 20
let: x == -18
do: 7
	light: x 5 -3
	light-color: .2 .2 .2
	light-spec: 1
	//light-area: 12 2
	//surf: 1 0 0
	//sphere: x 5 -13  1
	let: x += 6
enddo:
surf: .9 .9 .9
box: LTRBFB  0 0 0  40 10 4
trans-pop: 2


//a cone
trans-translate: -13 0 0
surf: .5 .4 .1
surf-specular: .2 .2 .2
cone: 0 0 0   8   0 15
trans-pop: 1

//a ball
trans-translate: 0 0 0
surf: .5 .4 .1
surf-specular: .2 .2 .2
sphere: 0 8 0   8
trans-pop: 1

//a cylinder
trans-translate: 14 0 0
surf: .5 .4 .1
surf-specular: .2 .2 .2
cylinder: 7 12
ring: 0 12 0  0 1 0  0 7
trans-pop: 1




