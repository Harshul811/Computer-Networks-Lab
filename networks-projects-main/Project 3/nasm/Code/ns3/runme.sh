#!/bin/bash

./ns3 run "scratch/code --protocol=TcpHighSpeed --for_loop=40 --simultaneously=0 --offset=2 --run_time=1 --packetsize=1024"
./ns3 run "scratch/code --protocol=TcpHighSpeed --for_loop=40 --simultaneously=1 --offset=0 --run_time=1 --packetsize=1024"
./ns3 run "scratch/code --protocol=TcpHighSpeed --for_loop=40 --simultaneously=1 --offset=2 --run_time=1 --packetsize=1024"
./ns3 run "scratch/code --protocol=TcpScalable --for_loop=40 --simultaneously=0 --offset=2 --run_time=1 --packetsize=1024"
./ns3 run "scratch/code --protocol=TcpScalable --for_loop=40 --simultaneously=1 --offset=0 --run_time=1 --packetsize=1024"
./ns3 run "scratch/code --protocol=TcpScalable --for_loop=40 --simultaneously=1 --offset=2 --run_time=1 --packetsize=1024"
./ns3 run "scratch/code --protocol=TcpVegas --for_loop=40 --simultaneously=0 --offset=2 --run_time=1 --packetsize=1024"
./ns3 run "scratch/code --protocol=TcpVegas --for_loop=40 --simultaneously=1 --offset=0 --run_time=1 --packetsize=1024"
./ns3 run "scratch/code --protocol=TcpVegas --for_loop=40 --simultaneously=1 --offset=2 --run_time=1 --packetsize=1024"

gnuplot TcpHighSpeed_delay_Seperate.plt                                                                            
gnuplot TcpHighSpeed_delay_Simultaneous_Same_Start.plt
gnuplot TcpHighSpeed_delay_Simultaneous_Different_Start.plt
gnuplot TcpHighSpeed_throughput_Seperate.plt
gnuplot TcpHighSpeed_throughput_Simultaneous_Different_Start.plt
gnuplot TcpHighSpeed_throughput_Simultaneous_Same_Start.plt
gnuplot TcpScalable_delay_Seperate.plt
gnuplot TcpScalable_delay_Simultaneous_Different_Start.plt
gnuplot TcpScalable_delay_Simultaneous_Same_Start.plt
gnuplot TcpScalable_throughput_Seperate.plt
gnuplot TcpScalable_throughput_Simultaneous_Different_Start.plt
gnuplot TcpScalable_throughput_Simultaneous_Same_Start.plt
gnuplot TcpVegas_delay_Seperate.plt
gnuplot TcpVegas_delay_Simultaneous_Different_Start.plt
gnuplot TcpVegas_delay_Simultaneous_Same_Start.plt
gnuplot TcpVegas_throughput_Seperate.plt
gnuplot TcpVegas_throughput_Simultaneous_Different_Start.plt
gnuplot TcpVegas_throughput_Simultaneous_Same_Start.plt 