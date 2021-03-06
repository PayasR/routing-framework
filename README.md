# Routing Framework

This repository contains the C++14 source code used in the following publications/submissions:

* Valentin Buchhold, Peter Sanders, and Dorothea Wagner. Real-Time Traffic Assignment Using Fast
  Queries in Customizable Contraction Hierarchies. In Gianlorenzo D'Angelo, editor, *Proceedings of
  the 17th International Symposium on Experimental Algorithms (SEA'18)*, volume 103 of *Leibniz
  International Proceedings in Informatics (LIPIcs)*, pages 27:1–27:15. Schloss Dagstuhl, 2018.
  [doi:10.4230/LIPIcs.SEA.2018.27](http://dx.doi.org/10.4230/LIPIcs.SEA.2018.27).
* Valentin Buchhold, Peter Sanders, and Dorothea Wagner. Real-Time Traffic Assignment Using
  Engineered Customizable Contraction Hierarchies. Submitted to the ACM Journal of Experimental
  Algorithmics.
* Valentin Buchhold, Peter Sanders, and Dorothea Wagner. Efficient Calculation of Microscopic Travel
  Demand Data with Low Calibration Effort. Submitted to the 27th ACM SIGSPATIAL International
  Conference on Advances in Geographic Information Systems (SIGSPATIAL'19).

## Prerequisites

You need to have some tools and libraries installed. On Debian and its derivatives (such as Ubuntu)
the `apt-get` tool can be used:

```
$ sudo apt-get install build-essential
$ sudo apt-get install scons
$ sudo apt-get install python3
$ sudo apt-get install libboost-all-dev
$ sudo apt-get install libcairo2-dev
$ sudo apt-get install libproj-dev
$ sudo apt-get install zlib1g-dev
```

Next, you need to clone, build and install the libraries in the `External` subdirectory. To do so,
type the following commands at the top-level directory of the framework:

```
$ git submodule update --init
$ cd External
$ cd fast-cpp-csv-parser && sudo cp *.h /usr/local/include && cd ..
$ cd randomc && sudo mkdir /usr/local/include/randomc && sudo cp *.cpp *.h $_ && cd ..
$ cd RoutingKit && make && sudo cp -r include lib /usr/local && cd ..
$ cd stocc && sudo mkdir /usr/local/include/stocc && sudo cp *.cpp *.h $_ && cd ..
$ cd vectorclass && sudo mkdir /usr/local/include/vectorclass && sudo cp *.h special/* $_ && cd ..
```

## Building

Once you installed the packages, simply type `scons` at the top-level directory of the framework:

```
$ scons
```

## Experiments in *Real-Time Traffic Assignment Using Fast Queries in Customizable Contraction Hierarchies*

To get the version of the source code used in the publication, check out the `SEA18` tag:

```
$ git checkout SEA18
```

To run the experiments presented in the publication, enter the following commands at the top-level
directory of the framework:

```
$ cd Publications/FastTA/SEA18
$ ./PrepareP2P <path-to-xatf-data> ~
$ ./PrepareTA <path-to-visum-data> <path-to-mobitopp-data> ~
$ ./ConductP2P ~
$ ./ConductTA ~
```

*Note: This repository contains the full source code used in the publication. However,
unfortunately the data used in the experiments is proprietary and not publicly available.*

## Experiments in *Real-Time Traffic Assignment Using Engineered Customizable Contraction Hierarchies*

To get the version of the source code used in the publication, check out the `JEA18` tag:

```
$ git checkout JEA18
```

To run the experiments presented in the publication, enter the following commands at the top-level
directory of the framework:

```
$ cd Publications/FastTA/JEA
$ ./PrepareP2P <path-to-stuttgart-data> <path-to-london-data> <path-to-xatf-data>     ~
$ ./PrepareTA  <path-to-stuttgart-data> <path-to-london-data> <path-to-mobitopp-data> ~
$ ./ConductP2P ~
$ ./ConductTA  ~
```

*Note: This repository contains the full source code used in the publication. However,
unfortunately the data used in the experiments is proprietary and not publicly available.*

## Experiments in *Efficient Calculation of Microscopic Travel Demand Data with Low Calibration Effort*

To get the version of the source code used in the publication, check out the `SIGSPATIAL19` tag:

```
$ git checkout SIGSPATIAL19
```

To run the experiments presented in the publication, create the following directory structure in a
freely chosen base directory `<base-dir>` and fill the directories with the respective raw data:

```
<base-dir>/Inputs/RawData/GermanCensus2011/
<base-dir>/Inputs/RawData/XATF/
<base-dir>/Inputs/RawData/GEOSTAT2011/
<base-dir>/Inputs/RawData/mobiTopp/
<base-dir>/Inputs/RawData/Visum/London/
<base-dir>/Inputs/RawData/Visum/Stuttgart/
```

*Note: This repository contains the full source code used in the publication. However,
unfortunately most data used in the experiments is proprietary and not publicly available.
Exceptions are the [German population grid](https://www.zensus2011.de/DE/Home/Aktuelles/DemografischeGrunddaten.html)
and the [European population grid](https://ec.europa.eu/eurostat/web/gisco/geodata/reference-data/population-distribution-demography/geostat).*

Now, enter the following command at the top-level directory of the framework:

```
$ Publications/DemandCalculation/SIGSPATIAL19/RunExperiments <base-dir>
```

## SCons Integration for Eclipse

The plugin SConsolidator provides tool integration for SCons in Eclipse.
Install it via its update site `http://www.sconsolidator.com/update`.

At the time of writing, there is a bug in the SConsolidator sources that you have to fix by hand.
If you are lucky, the bug will have been fixed in the official sources at the time you read this.
If not, try the following:

1. Find out where Eclipse installed the SConsolidator sources.
2. Open file `ch.hsr.ifs.sconsolidator.core/scons_files/BuildInfoCollector.py` with a text editor.
3. Change the definition of the function `get_compiler_flags` from

```
def get_compiler_flags(lang, environ):
    return (environ['CXXFLAGS'] if lang == 'c++' else environ['CCFLAGS'])
```

to

```
def get_compiler_flags(lang, environ):
    return [environ['CXXFLAGS'] if lang == 'c++' else environ['CCFLAGS']]
```

To import the framework as a SCons project into Eclipse, follow these steps:

1. Choose the menu `File`, `Import...`.
2. Select `C/C++`, `New SCons project from existing source`.
3. Choose the top-level directory of the framework.
4. Choose the menu `Project`, `Properties`, `C/C++ Build`, `Settings`, `Binary Parsers`.
5. Enable `Elf Parser`.
