#!/usr/bin/python

import serial,time

port1 = serial.Serial("/dev/ttyO1",baudrate=9600,timeout=3.0)
port4 = serial.Serial("/dev/ttyO4",baudrate=9600,timeout=3.0)

rcv = "blah"
while 1:
	# get keyboard input
	input = raw_input(">> ")
        # Python 3 users
        # input = input(">> ")
	if input == 'exit':
		port1.close()
		exit()
	else:
		# send the character to the device
		# (note that I happend a \r\n carriage return and line feed to the characters - this is requested by my device)
		port1.write(input + '\r\n')
		out = ''
		# let's wait one second before reading output (let's give device time to answer)
		time.sleep(1)
		while port1.inWaiting() > 0:
			out += port1.read(1)
			
		if out != '':
			print ">>" + out

