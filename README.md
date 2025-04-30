# Asynchronous System Exec

An alternative to LabVIEW's native `System Exec.vi` with asynchronous access to standard input, output and error.

Similar to the `.NET System Exec` from 💖[JKI](https://www.jki.net)💖 - [see comparison table for more](#comparison-with-jkis-net-system-exec)

## Features
✨ Flexible LabVIEW Event based API


✨ Cross Platform support for 32-bit/64-bit Windows and 64-bit Linux (RHEL, CentOS, openSUSE & NI-RT)*


✨ Non-blocking - multiple executables can run without blocking each other or any LabVIEW threads


✨ Regular Expression powered end-of-line matching for Standard Out and Standard Error


✨ Built for the LabVIEW Community with [LabVIEW 2020 Community Edition](https://www.ni.com/en-gb/shop/labview/select-edition/labview-community-edition.html) but supports saving for LabVIEW 2015 


✨ Available under the [Boost License v1.0](./LICENSE) - an Open Source Initiative approved license similar to the MIT/BSD licenses without the attribution requirements for binary files

\* Arm based NI-Linux-RT and Apple Mac OSs _are_ mostly supported by C++/LabVIEW code but presently there is no build tooling for these platforms*


![An animation showing a console example VI running commands with the shell on a NI-Linux-RT target](<./docs/img/example-command-line-ni-linux-rt.gif>)

---
## Version 3.0.1 Released 🎉

### Whats New?
* Boost (C++ Libraries) boost::process V1 has been deprecated [so this updates to use V2](https://www.boost.org/doc/libs/1_87_0/doc/html/boost_process/v2.html). The ability to use the "cmd" of Start Call.vi has been removed in-line with the boost::process API changes. However, this library now includes a "Lookup Executable Path.vi" which will find the full path of an executable from the system's PATH with just the executable's name to allow for similar behavior if required.

* The On-Exit event now includes any data from the Standard Output and Standard Error pipes that had not yet regex-matched and been generated as Std-Out/Std-Err events.

* Configuration has been simplified to a cluster to avoid custom configuration classes being required.

* The conversion on Windows platforms from UTF-16 can now be specified to be either UTF-8 or the ACP or OEM code page character encoding.

---

## Installation on Windows and Linux-x64
The VIPM Package can be found on [VIPM.io](https://www.vipm.io/package/serenial_lib_asynchronous-system-exec/) or a zipped copy of the VIPM contents can be found in the [releases page of this repository](<../../releases>).

## Installation on NI-Linux-RT x64
Once this library is installed on the host machine, copy `serenial.io-ase_64-3.so` from the `serenial-io.ase/ase/bin` directory (from `vi.lib/Serenial` if installed with VIPM) into the targets `usr\lib\`

See [this guide](https://knowledge.ni.com/KnowledgeArticleDetails?id=kA03q000000YMLnCAO&l=en-GB) for methods to transfer the file, alternatively, if the target is setup for `ssh` access and your host machine has the `scp` utility then this can be used as follows to copy the package file to the target's `/tmp` directory.

```sh
scp <path-to-file-on-host>/libserenial.io-ase_<X.Y.Z>_x64.ipk admin@<NI-RT-SYSTEM-IP>:/tmp
```

Alternatively, you can use a "Source Distribution" Build under a Real Time Target to deploy files across to the RT Target easily.

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
