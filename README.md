# Asynchronous System Exec

An alternative to LabVIEW's native `System Exec.vi` with asynchronous access to standard input, output and error.

Similar to the `.NET System Exec` from 💖[JKI](https://www.jki.net)💖 - [see comparison table for more](#comparison-with-jkis-net-system-exec)

## Features
✨ Flexible LabVIEW Event based API


✨ Cross Platform support for 32-bit Windows and 64-bit Linux (RHEL, CentOS, openSUSE & NI-RT)*


✨ Non-blocking - multiple executables can run without blocking each other or any LabVIEW threads


✨ Regular Expression powered end-of-line matching for Standard Out and Standard Error


✨ Built for the LabVIEW Community with [LabVIEW 2020 Community Edition](https://www.ni.com/en-gb/shop/labview/select-edition/labview-community-edition.html) but supports saving for LabVIEW 2015 


✨ Available under the [Boost License v1.0](./LICENSE) - an Open Source Initiative approved license similar to the MIT/BSD licenses without the attribution requirements for binary files

\* *Support for 64-bit Windows if building from source. Arm base NI-Linux-RT and Apple Mac OSs are supported by C++/LabVIEW code but presently there is no build tooling for these platforms*


![An animation showing a console example VI running commands with the shell on a NI-Linux-RT target](<./docs/img/example-command-line-ni-linux-rt.gif>)

---

## Installation
A package based distribution (VIPM etc) for this library is still under-development.

In the mean time, check the releases of this repository for downloadable library builds.

Unzip the distribution into your project directory. The library has no dependencies except LabVIEW's built-in libraries.

When first opening/mass-compiling this library, LabVIEW will attempt to resolve the new location of shared library file `serenial-io.ase.<bitness>.<extension>`. Resolution should happen automatically but might lead to warnings which can be safely ignored.

![LabVIEW Open Loading Warning](<./docs/img/loading-warning.png>)

## Installation on NI-Linux-RT x64
Once installed on the host machine, copy the `libserenial.io-ase_X.Y.Z_x64.ipk` from the `packages` directory to your Real-Time target.

See [this guide](https://knowledge.ni.com/KnowledgeArticleDetails?id=kA03q000000YMLnCAO&l=en-GB) for methods to transfer the file, alternatively, if the target is setup for `ssh` access and your host machine has the `scp` utility then this can be used as follows to copy the package file to the target's `/tmp` directory.

```sh
scp <path-to-file-on-host>/libserenial.io-ase_<X.Y.Z>_x64.ipk admin@<NI-RT-SYSTEM-IP>:/tmp
```

With the package file transferred to the Real-Time target, install it using `opkg` as the `admin` user 

```sh
opkg install /tmp/libserenial.io-ase_<X.Y.Z>_x64.ipk
```
---
## Getting Started

Please see the [using the Asynchronous System Exec documentation](./docs/using-the-asynchronous-system-exec.adoc) for getting started using the library.

If you would like to build the binaries for your platform or develop either the LabVIEW or the C++ code, please see the [building from source documentation and developing documentation](./docs/building-from-source-and-developing.adoc).

For information on contributions, look no further than the [contributing document](./CONTRIBUTING.adoc).

---
## Comparison with JKI's .NET System Exec

| Feature | [`.NET System Exec`](https://www.vipm.io/package/jki_lib_dotnet_system_exec/) | This Library |
|---------|-------------------|--------------|
| Write To Std In  | Yes | Yes|
| Std Out & Error User Events| Yes | Yes |
| Exit User Event | Yes | Yes (with exit code) |
| Send Terminate Signal | No | Yes |
| Max Simultaneous Instances |  Limited by System Resources  | Limited by System Resources|
| OS Support | Windows | Windows (32 and 64-bit), Linux and NI-Linux-RT (64-bit) |
| Made by a LabVIEW Community [OG](https://www.merriam-webster.com/dictionary/OG) | Most Definitely | 😰 |

---
## Contributors
[John Medland](https://twitter.com/j_medland) 