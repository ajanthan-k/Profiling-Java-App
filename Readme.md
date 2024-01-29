# System Performance Engineering Coursework 1: Profiling & Scale-up optimization

## Objective

The objective of this exercise is to practice your performance engineering analysis and tuning skills on a large, complex system supporting an unfamiliar application. We do not focus on a micro-architectural analysis but on the identification of pieces of code that are important for performance at an application level and the extraction of a meaningful microbenchmark for a Java application. This vastly increases the number of analysis tools at your disposal: VTune in hotspots mode, VisualVM, JProfiler, Java Mission Control and many more. With the help of your favorite analysis tools, you should identify the most performance-critical pieces of code of the application and focus your attention on those.

## The application

I have written a java application that is representative for the kinds of data-science applications many non-performance-experts write in frameworks like Python Pandas or R Dataframes. The application analyzes Covid-19 case numbers (downloaded from “Our World in Data” to determine the per-day hotspot (i.e., the country with the highest caseload normalized by population). As Vtune support for R and Python is flaky at best, I implemented the application in Java using a framework called TableSaw ([https://jtablesaw.github.io/tablesaw/](https://jtablesaw.github.io/tablesaw/)).

# Setting up the environment

Log in to your favorite **vertex lab machine** and run the instructions there (you are, of course, free to work through the assignment on your own machine but the instructions are written for and tested on one of the vertex machines).

As you may have experienced, VTune is a bit fickle and that includes the java plugin: some versions of the jdk work without bugs, others don't. To make you life a little easier, we have created a container you can (and should) run the code in.

## Getting Started

You should have received an invitation to a Github Classroom. You will be submitting the various your coursework through Github Classroom, so please, accept the invitation with your github account (create one if necessary).

To get started, simply clone your Github repository: `git clone ${your repository url}`. Throughout the instructions, I will assume that your clone is located in `~/Projects/tablesawcoursework`. If you clone it somewhere else, you, of course, have to adapt the instructions.

You also want to download the data

```bash
cd ~/Projects/tablesawcoursework/TargetApplication
curl -L -o owid-covid-data.csv \
'https://github.com/owid/covid-19-data/blob/master/public/data/owid-covid-data.csv?raw=true'
```

## Downloading the container

While we built the environment as a docker container, CSG limits the use of docker on lab machines (for good reason). However, the image can be downloaded and run using singularity. As the image is fairly big, you may not want to keep it in your home directory but use the `/data` directory (note that this is machine-local an not backed up so don't keep your important data there). Here is how you download the image:

```bash
mkdir -p /data/$USER/m2
mkdir /data/$USER/cmake-downloads
cd /data/$USER
/vol/linux/apps/singularity/bin/singularity pull -F docker://ghcr.io/little-big-h/spe-image:main
```

## Properly setting up ssh (including authorized_keys)

VTune requires the setup of password-less logins when using ssh. For our purpose, that means using the authorized_keys mechanism. For that purpose, you need to make sure that you have a pair of keys in your `.ssh`-directory. If (and only if!) you don't have a file called `id_rsa` in your `~/.ssh`, do you need to run `ssh-keygen`. The program asks for a passphrase but, in my opinion, it is safe to leave that empty.

Afterwards you want to append the public(!) ssh-key of the machine you will have the vtune-gui running on to `~/.ssh/authorized_keys`. If you plan to run vtune directly on the vertex machine, you have to append  the local public ssh-key to the local authorized keys like this:

```bash
cat ~/.ssh/id_rsa.pub >> ~/.ssh/authorized_keys
```

If you run into issues, you might want to read up on ssh key-handling (it is just one of those things people will assume you know, despite no-one ever explicitly teaching you about it :-): [https://www.howtouselinux.com/post/ssh-authorized_keys-file](https://www.howtouselinux.com/post/ssh-authorized_keys-file).

## Entering the container

By default Singularity mounts virtually every directory into the container. To avoid that from happening, we want to disable that and only selectively mount what is required.

```bash
/vol/linux/apps/singularity/bin/singularity shell -e -H /home/$USER --pwd /home --no-home \
--no-mount $PWD  -B $HOME/Projects/tablesawcoursework:/home/$USER/tablesawcoursework,\
/data/$USER/m2/:/home/$USER/.m2,/data/$USER/cmake-downloads:/home/$USER/.cmake-downloads,\
$HOME/.ssh/authorized_keys:/home/$USER/.ssh/authorized_keys,\
$HOME/.ssh/id_rsa:/home/$USER/.ssh/id_rsa,/vol/intel:/vol/intel,/tmp:/tmp \
/data/$USER/spe-image_main.sif
```

## Building the application

Once in the container, build the application

```other
cd /home/*/tablesawcoursework/TargetApplication
mvn compile
```

## Running it

Like this

```other
mvn exec:java -Dexec.mainClass=uk.ac.ic.doc.spe.covidsaw.App
```

This should give you an output like this (plus a bunch of java/maven-related cruft around it):

```other
date     |  iso_code  |       smoothed        |
---------------------------------------------------
 2020-02-24  |       KOR  |    99.52599999999998  |
 2020-02-25  |       KOR  |   107.90199999999997  |
 2020-02-26  |       KOR  |   116.89599999999997  |
 2020-02-27  |       KOR  |   126.64199999999997  |
 2020-02-28  |       ISL  |              134.083  |
 2020-02-29  |       SMR  |              801.424  |
 2020-03-01  |       SMR  |             1098.248  |
 2020-03-02  |       SMR  |   1454.4370000000001  |
 2020-03-03  |       SMR  |   1780.9430000000002  |
 2020-03-04  |       SMR  |   1780.9430000000002  |
        ...  |       ...  |                  ...  |
 2020-02-10  |       CHN  |               40.348  |
 2020-02-11  |       CHN  |   38.468999999999994  |
 2020-02-12  |       CHN  |    35.99099999999999  |
 2020-01-06  |       LVA  |                 0.54  |
 2020-01-18  |       LVA  |                 0.54  |
 2020-01-16  |       TWN  |                0.084  |
 2020-01-17  |       TWN  |                0.084  |
 2020-01-19  |       TWN  |                0.168  |
 2020-01-20  |       TWN  |  0.29400000000000004  |
 2020-01-21  |       TWN  |  0.29400000000000004  |
```

# Your Tasks

### Task 1: Profile

The program uses a data processing library called TableSaw. While the library is only moderate in size as systems go, it constitutes a fairly large codebase with many opportunities for optimization. In the course of this exercise, we want to restrict ourselves to "local" improvements and treat the rest of the system as a black box.

To identify sections of code that are critical to the provided Covid Hotspot Finder program, spin up a profiler of your choice and launch the application in it. Unfortunately, you cannot run vtune (with GUI) in the container and you cannot easily attach to an application running inside a container from the outside. However, you can run an ssh daemon inside the container and use vtune's remote profiling feature to execute applications inside the container.

You launch an ssh server like this (I am using 2222 as the port here, you might need to run on a different port if the port is already in use):

```bash
/usr/sbin/sshd -p 2222 -o UsePAM=no -h /home/*/.ssh/id_rsa
```

When you are done, please make sure that you shut down your ssh server like this: `kill `pgrep -f "sshd -p 2222"``

You now have a running container that is ready for profiling. You can spin up vtune ***outside the container***:

```bash
. /vol/intel/oneapi/vtune/2022.2.0/amplxe-vars.sh
vtune-gui
```

If you run the GUI remotely on a vertex machine, you need to either run a vnc server on the vertex (`vncserver`) or use `ssh -Y` to forward X11 connections.

To run the application inside the container, you want to configure vtune like this:

![Vtune Configuration](https://secure-res.craft.do/v2/8RTgJ6iEpE7GNtPLnXhJPoPQiJEUj5Hx1jsyGECTVEgWcZuD6y7vAbmC1xgDZHAQvUFxBAAV1fbv3u1gVL2nJYSWag48DRsKqa1FyPW1YK3bduZUVbuZAsoYUokVodaBxB4Rmjqbj4UjAbjfy7LpAprWEfrfEtsak4VsggaQBSm9dHnLPvtTBZ1qiAwwji8qL1T7HGWyikiqvqGj74CA7zZCUiW6hZbTGdPgotbeoqug621BLzbumyxgm4GPjxTa4JWysrfo6Lp2nWD6Pby1FyDWr6zrckEemXYifYys8eDv1xMGVS/Image.jpg)

To complete the task, analyze the application and provide a breakdown of the runtime of the relevant section of the program (note that maven runs a lot of setup-code before the actual execution of the program starts)

. The program is a typical data science pipeline: it performs a number of processing steps including loading, smoothing, aggregation (tablesaw calls it summarization), filtering and output. Exclude loading and output from your analysis but provide a breakdown and report those in the Breakdown.txt file. **The file shall be no longer than 300 words.**

*30 marks are awarded for Task 1*

### Task 2: Extract Microbenchmark

Your second task is to extract a microbenchmark for the most expensive part of the aggregation. Determine the most expensive function and base your work of that. If you are in doubt if you have identified the right function, the containing class's name has 23 characters and the function 7. You will probably want to attach a debugger to the application to determine where/how the function is called from the application.

You shall implement the microbenchmark in the Java Microbenchmark Harness (JMH) framework. JMH is similar to google benchmark. You can find a good tutorial at [http://tutorials.jenkov.com/java-performance/jmh.html](http://tutorials.jenkov.com/java-performance/jmh.html) while the official page is [https://openjdk.java.net/projects/code-tools/jmh/](https://openjdk.java.net/projects/code-tools/jmh/). For your convenience, I have pre-generated a benchmark project in `JavaBenchmark` in your LabTS repository and added TableSaw as a dependency. You can compile and run it, changing into the directory and running (JMH has a lot of options to influence how it runs the benchmark but this combination works well for our case):

```other
mvn install && java -jar target/benchmarks.jar -w 1 -wi 1 -r 1 -f 1 -i 5 -bm avgt -tu ns
```

To create a meaningful microbenchmark, feel free to either copy pieces of the relevant code into your microbenchmark program or call into the framework. The point of this part of the task is not for you to be creative but to reproduce the application behavior accurately in your microbenchmark. Take care that you properly isolate the relevant part of the code and don’t include setup overhead in your benchmarking numbers. 

Like that of most applications, your will find that this program’s performance depends heavily on characteristics of the data. Form a hypothesis about one those characteristics and establish how it might impact execution time of the hot code section (no need to implement the parts of the code that have not practical costs). State your hypothesis (e.g., in the form of a simple characteristic equation) in the Report.txt file. You ensure not having your microbenchmark depend on external resources like files or network servers (you should be familiar with that approach from the way you write unit tests). This means that you must implement a (simple) data generator that captures the relevant data characteristics.

Develop a microbenchmark (i.e., data generator, code, etc.) that sweeps meaningful values for your parameter (see [https://github.com/openjdk/jmh/blob/master/jmh-samples/src/main/java/org/openjdk/jmh/samples/JMHSample_27_Params.java](https://github.com/openjdk/jmh/blob/master/jmh-samples/src/main/java/org/openjdk/jmh/samples/JMHSample_27_Params.java) for an example illustrating how to sweep parameters). To validate or refute your hypothesis. Note that it is fine if you end up refuting your hypothesis. It is better the refute an interesting hypothesis than validate a boring one (there is an opportunity to score up to 10 bonus marks for interesting hypothesis).

*30 marks are awarded for Task 2*

### Task 3: Establish Baseline

Your next task is to contrast the performance of the Java implementation of the critical section to one in C++ (or C). For that purpose, the LabTS project contains a project in the directory `CppBenchmark`. Build and the code like this:

```other
cd CppBenchmark
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

Run it like this

```other
./Benchmarks
```

To complete the task, reimplement your java microbenchmark in the file `Benchmark/Benchmarks.cpp` using the google benchmark framework you know (and love: [https://github.com/google/benchmark](https://github.com/google/benchmark)). Place your the code you are going to benchmark (i.e., the re-implementation of the TableSaw code) in `Source/CppBenchmark.hpp` and/or `Source/CppBenchmark.cpp` (similar to separating tests and implementation). In this section, you may lose marks for bad code. You do not have to adhere to any C/C++ coding guideline but you want to avoid things that are bad in any language: unclear or spaghetti code, uninitialized variables, poor structure, etc. Remember: there are people on the other side that have to read your code and you do not want to upset them. Note that even in C, you should structure data in `structs`.

Note that the purpose of this task is to establish a baseline for the runtime of the task, not to develop the fastest or most memory-efficient implementation. Consequently, you are free to use C++ standard library classes and functions as well as their templates. However, you want to make sure that your C++ implementation is representative for the java implementation (you will probably have to dig into the TableSaw implementation a bit to make sure you understand what it does). If you implement things in C, you can be sure that you are implementing the same fundamental algorithm. If you use the C++ standard library code, you will need to justify (i.e., comment in the code) why the algorithms and data structures perform like their java counterparts.

In the course of your work, you will encounter a library called "roaring bitmaps". It is a well-tuned library that is outside the scope of this coursework (i.e., you do not have to faithfully re-implement it). However, you should aim to capture its essence: it is a run-length-encoded bitmap. In you C(++) implementation you can replace it with your own implementation (you may not include a third-party roaring bitmap implementation). If there are simplifications you would like to make and are unsure about, ask me (Holger) after a lecture.

Evaluate the microbenchmark and note down your findings in Report.txt. Focus on a comparison of the performance of the Java and C++ implementation.

*40 marks are awarded for Task 3*

## Bonus Task: Testing

To ensure correctness of your implementation you should also write at least three meaningful test for your C++ implementation in Test/CppBenchmarkTests.cpp.

You can earn up to 10 bonus marks for this task. These marks can help you make up for marks lost in other tasks (naturally, you cannot achieve more than 100% of the marks in total).

## Submission


**The length of the breakdown (Task 1) must not exceed 300 words, the report (Tasks 2 and 3 combined) must not exceed 500 words. You will lose marks (and goodwill) if you exceed the limit. Also note that brevity is a virtue: no marks are awarded for length and a succinct description might even earn you some discretionary bonus marks.**

Happy coding!

