//array test
angle: 45
from: 0 0 80
at: 0 0 0
up: 0 1 0

array: col [ 25 ] == { 1 .9 .5 }
let: col [ 3 ] == .9
let: col [ 4 ] == .5
let: col [ 5 ] == 0

let: c == .7
array: lcolor [ 3 ] == { c c c }

background: col [ 0 ] col [ 1 ] col [ 2 ]
ambient: .4 .4 .4
light: 100 100 100
light-color: lcolor [ 0 ] lcolor [ 0 ] lcolor [ 0 ]
//light-area: 10 12
surf: 1 0 0
surf: col [ 3 ] col [ 4 ] col [ 5 ]
sphere: 0 0 0  20

























