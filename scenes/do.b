//do looping and conditional statements
angle: 45
from: 0 1 1050
at: 0 0 0
up: 0 1 0
maxdepth: 10

array: col [ 3 ] == { .4 .3 .1 }

call: setup_lighting
call: grid
//call: edge_dots
call: polygons


montecarlo: 2
//adaptive: 0.01

function: setup_lighting
//background: 0 0 0
background: col [ 0 ] col [ 1 ] col [ 2 ]
ambient: .4 .4 .4
light: -1000 1000 4000
light-color: .7 .7 .7
light-area: 660 5
endfunction:

function: grid
let: y == -225
let: n == 0
do: 9
	let: x == -225
	do: 9
		//surf: rnd .9 .1  rnd .8 .2  rnd .7 .3
		if: n >= 46
			surf: 1 0 0
		else:
			surf: 1 1 0
		endif:
		surf-specular: .1 .1 .1
		sphere: x y 40   20
		let: x += 50
		let: n += 1
	enddo:
	let: y += 50
enddo:
endfunction:


function: edge_dots
//surf: .75 0 0
//surf-specular: .1 .1 .1
//predefined variables a-z
let: n == 40
let: a == 0
let: c == 3.141592653
let: c *= 2.0
let: c /= n
do: n
	surf: .75 rnd .4 0 rnd .4 0
	surf-specular: .1 .1 .1
	let: x == cos a
	let: y == sin a
	let: a += c
	let: x *= 400.0
	let: y *= 266.666
	sphere: x y 0   10
enddo:
endfunction:


function: polygons
let: z == 0
let: n == 9
let: r == 360
do: 7
	surf: rnd 1 0  rnd 1 0  rnd 1 0
	let: m == .6666
	let: a == rnd 6.28 0
	let: c == 3.141592653
	let: c *= 2.0
	let: c /= n
	let: x == cos a
	let: y == sin a
	let: a += c
	let: x *= r
	let: t == r
	let: t *= m
	let: y *= t
	do: n
		let: u == x
		let: v == y
		let: x == cos a
		let: y == sin a
		let: a += c
		let: x *= r
		let: t == r
		let: t *= m
		let: y *= t
		tri: x y z u v z 0 0 z
	enddo:
	let: z += 50
	let: n -= 1
	let: r *= 0.6
enddo:
endfunction:

























