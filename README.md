Library for Monte Carlo simulation of photons movement in scintillator of rectangular shape
===========================================================================================


Compiling
=========

If you have your git repository with cmake project you can add needed repositories as submodules:

	git submodule add https://github.com/alexkernphysiker/math_h.git
	git submodule add https://github.com/alexkernphysiker/RectangularScintillator.git
	git submodule update --init --recursive

Then add to CMakeLists.txt the following lines

	add_definitions(--std=c++17) #the most recommended compiler mode
	set(GTEST ON) #for compiling unit-tests. Requires gtest
	add_subdirectory(math_h)
	add_subdirectory(RectangularScintillator)
	include_directories(${MATH_H_INC})
	include_directories(${RECTSCIN_INC})


General information
===================

This library implements model of rectangular shaped scintillator for Monte Carlo simulation.
This model considers the scintillator to have constant refraction parameter.
The distribution of photon emission time and emitted photons wavelengths are given.
The absorption coefficient is defined as a function depending on photon wavelength.

Photosensitive surfaces can be optically glued to different surfaces of the scintillator. 
The parameter of glue efficiency that means decreasing of photon's reflecting-back probability in the place of contact can be changed.

Photosensitive surfaces may have registration probability depending on photon wavelength.
Photons that are registered can be processed by signal former algorithms that can be connected to this photosensor.
The implemented algorithms can process signal time (depending on the time of photons registration) and amplitude (depending on photons count).

The signals can be further processed with combinations of algorithms implemented in this library or by user.

All classes are declared in namespace RectangularScintillator.



Scintillator
============
Class Scintillator defined in rectscin.h implements generating emitted photons and tracing them inside the scintillator.
For creating the instance of this class one should give the geometry of the scintillator as vector < pair < double,double > >
where each pair corresponds to a different dimension and the elements in the pair correspond to minimum and maximum coordinates occupied by the scintillator.
The distribution of photon's emission time and wavelength are given as RandomValueGenerator < double >  that is declared in math_h/randomfunc.h (in submodule).
Refraction parameter is given as double. 
Absorption coefficient is given as function < double (double) > because it depends on photon wavelength.

Method {Surface(dimension,side)} returns the object that represents corresponding rectangular surface and allows to connect scintillator with photosensor or anything else that is designed for this.
The {dimension} parameters means that the surface is defined by fixing corresponding coordinate to it's minimum value for left side and maximum value for right side.
surface geometry is given the same way but excluding the dimension that has fixed coordinate.



Photosensor
===========

Class PhotoSensitiveSurface defined in sensitive.h contains implementation of photosensor.
It requires surface geometry (see above how it's defined), glue efficiency parameter given as double between 0 and 1 
and quantum efficiency defined as function < double (double) > that depends on photon wavelength and returns value between 0 and 1.
It's recommended to use Photosensor function for creating the instances of this class.
Here's the example of using photosensors:

	Scintillator scintillator({...geometry...},...other parameters...);
	auto sensor=Photosensor({...surface geometry...},...other parameters...);
	scintillator.Surface(dimension,RectDimensions::Left)>>sensor;



Signal formers
==============

All classes declared in photon2signal.h inheriting PhotonTimeAcceptor and SignalProducent are used for modeling signals created by photosensors.
PhotonTimeAcceptor is an interface for accepting registered photons and SignalProducent implements transfering signal (defined as double value) for further analysis.
After creating and configuring such object can be connected to PhotoSensitiveSurface instance:

	auto timesignal=TimeSignal({make_pair(0,0.5),make_pair(1,0.5)});//average of 1st and 2nd photon times
	scintillator.Surface(...) >> ( sensor >> timesignal );



Signal analyse
==============


File signal_statistics.h contains classes inheriting {SignalAcceptor} and using for analysis of simulated events.
Example:

	auto statistics=make_shared<SignalStatictics>();
	scintillator.Surface(...) >> ( sensor >> ( timesignal >> statistics ) );

or such way:

	scintillator.Surface(...)
	>> ( sensor1 >> ( make_shared < SignalStatictics > () >> statistics1 ) )
	>> ( sensor2 >> ( make_shared < SignalStatictics > () >> statistics2 ) );
	
or another way:

	scintillator.Surface(...one...) >> ( sensor1 >> (make_shared < SignalStatictics > () >> statistics1 ) );
	scintillator.Surface(...another...) >> ( sensor2 >> ( make_shared < SignalStatictics > () >> statistics2 ) );

Here template "make_shared" must have type of used signal analysing class as a parameter.



More complicated systems
========================

File signal_processing.h contains declarations of classes used for more complicated signals processing.
Classes for processing one signal inherit SignalAcceptor and SignalProducent at the same time.
So they can be connected to another SignalProducent and another SignalAcceptor instances can be connected to this classes.
There are also interfaces AbstractMultiInput and AbstractMultiOutput that allow to create classes that allow to process several signals at the same time. 
You can see sources of example applications to see how they can be used.
