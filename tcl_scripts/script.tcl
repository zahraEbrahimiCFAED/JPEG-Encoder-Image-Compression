############################################################
## This file is generated automatically by Vivado HLS.
## Please DO NOT edit it.
## Copyright (C) 1986-2019 Xilinx, Inc. All Rights Reserved.
############################################################
open_project JPEG_Encoder
set_top encode_top_level
add_files JPEG_Encoder/src/jpeg.cpp
add_files JPEG_Encoder/src/jpeg.hpp
add_files JPEG_Encoder/src/toplevel.cpp
add_files JPEG_Encoder/src/toplevel.hpp
add_files -tb JPEG_Encoder/tb/tb.cpp
add_files -tb JPEG_Encoder/files
open_solution "solution1"
set_part {xc7vx485t-ffg1157-1}
create_clock -period 2 -name default
#source "./JPEG_Encoder/solution1/directives.tcl"
csim_design
csynth_design
cosim_design
export_design -format ip_catalog
