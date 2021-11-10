// a simple application which lists input args on stdout and stderr
// and will then echo a line of stdin to stdout
// exit by writing 'quit\n' to stdin
// exit code is the number of input args provided
//          Copyright serenial.io and contributors.
// Distributed under the Boost Software License, Version 1.0.
//    (See https://www.boost.org/LICENSE_1_0.txt)

// This application is used for LabVIEW Testing routines
#include <string>
#include <iostream>
#include <chrono>
#include <thread>


int main(int argc, char *argv[]) 
{
  // print args to stdout
  for (int i=1; i<argc;i++){
      std::cout << argv[i] << std::endl;
  }

  // print args to stderr
  for (int i=1; i<argc;i++){
      std::cerr << argv[i] << std::endl;
  }

  std::string input("");

try{
  while(std::getline (std::cin,input)) {
    if(input == "exit") break;
    std::cout << input << std::endl;
  }
}
catch(...){
  return -123546;
}
  
  return argc-1;
}