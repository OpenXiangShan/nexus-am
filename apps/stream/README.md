===============================================

STREAM is the de facto industry standard benchmark
for measuring sustained memory bandwidth.

Documentation for STREAM is on the web at:
   http://www.cs.virginia.edu/stream/ref.html

===============================================

STREAM is a project of "Dr. Bandwidth":
	John D. McCalpin, Ph.D.
	john@mccalpin.com

===============================================

Ported to AM by Zhigang Liu

**Note: Please Read stream.c carefully before you do anything.**

Tunable parameters:
* DSTREAM_ARRAY_SIZE=134217728
* DSTREAM_TYPE=float
* DNTIMES=10

===============================================

STREAM Benchmarks include following micro benchmarks:

a,b,c are arrays, scalar is a scalar number.

* Copy:  c[j] = a[j];
* Scale: b[j] = scalar * c[j]
* Add:   c[j] = a[j] + b[j]
* TriAd: a[j] = b[j] + scalar * c[j]

===============================================
