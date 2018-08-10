Quick Start
==========================

Installation
--------------------------

### Lagopus vswitch installation

Install necessary packages

```
	$ sudo apt-get install build-essential libgmp-dev \
	  libssl-dev libpcap-dev byacc flex git
```

Compile and Install

```
	$ cd utils/
	$ ./configure
	$ make
    $ sudo make install
```

Development
==========================
Lagopus has the developer's mode build to proceed unit tests in Lagopus.
Details can be found docs/how-to-write-tests-in-lagopus.md.
