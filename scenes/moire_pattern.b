//checker board showing moire patterns

from: 0 10 100
at: 0 0 0

angle: 30
up: 0 1 0
ambient: .5 .5 .5
background: 0 0 0

//jitter: .1

montecarlo: 20
//adaptive: 0.1

// aperture: radius, aperture.focal_length, nsamples
aperture: .6 60  20

light: 10 100 100
light-color: .5 .5 .5
//light-area: 10 10

trans-rotate: 0 12 0
checker:
2 0 2
1 1 1   0 0 0
surf: .01 .01 .01
surf-pattern: 1
trans-pop: 1
ring: 0 0 0   0 1 0   0 1000

checker:
1.4 0 1.4
1 0 0   1 1 1
surf: 0 0 1
surf-pattern: 1
//ring: 0 0 -300   0 0 1   0 1000





