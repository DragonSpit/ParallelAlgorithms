To compile on Linux do the following steps:

To update gcc to support c++17 standard:

sudo apt update
sudo apt install libtbb-dev

To compile use the following command (the order of the following arguments matters):
g++ ParallelAlgorithms.cpp -ltbb -std=c++17 -o ParallelAlgorithms