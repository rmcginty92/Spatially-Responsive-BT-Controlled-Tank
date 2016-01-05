<h1>Spatially Responsive Bluetooth-Controlled Motorvehicle System</h1> 

<h3>Functionality</h3>
<h4>Specified Behavior of System</h4>
<p>For our final project we have constructed a spatially responsive motorvehicle system, incorporating bluetooth communication, motor control and distance sensor technology. All inputs to the system are received in one of two ways 1) 4x distance sensors - each positioned on a side of our rectangular box vehicle - each of which send information about the proximity of objects in the pointed direction, and 2) an external bluetooth module which is read through a serial port.</p>

<p>In order to start communicating with the bot, we have to send commands to the bluetooth module. The easiest way to achieve this is through a serial terminal program, such as CoolTerm. In addition, we have also incorporated an external remote controller which uses an accelerometer and an Arduino microcontroller to process a user's natural gestures and translate that into the movement of the bot. The remote controller includes a bluetooth module, which replaces the serial terminal and automatically transmits the characters based on the position of the remote controller. When the remote controller is on, the user needs only to tilt the remote controller based on the desired motion and the bot will respond accordingly. There is also a LCD screen, used to show the user what the current motion is, and whether the sensor has detected something in its path. When the Arduino code is running, we can also use its serial terminal to transmit to the Beaglebone's bluetooth if desired.</p>

<p>The system has two modes, a NORMAL mode from which the device can fed directional commands and an AVERSION mode, which simply moves away from objects as they come closer to the vehicle. In both modes the same analysis of spatially recognize objects is done, however different responses are evoked when objects come too close to the vehicle. in NORMAL mode, the vehicle stops moving if an object appears in the the direction it is commanded to move. The AVERSION mode, simply moves away from object if possible (as long as the direction away from one object isn't towards another object). Below is a list of commands that the system recognizes. </p> 
<h4>Commands (Modes Available)</h4>
<p>*<code>f</code> / tilt controller towards you   --> move forward (N)</p>
<p>*<code>b</code> / tilt controller away from you --> move backward (N)</p>
<p>*<code>l</code> / tilt controller to the left   --> move left (N)</p>
<p>*<code>r</code> / tilt controller to the right  --> move right (N)</p>
<p>*<code>v</code> --> switch to AVERSION mode (NA)</p>
<p>*<code>n</code> --> switch to NORMAL mode (NA)</p>
<p>*<code>q</code> --> quit (NA)</p>

<h3>Lab Components</h3>
<h4>ADC</h4>
<p>The ADC code collects data from the ADC pins on the Beaglebone board in a buffer, in which it then writes to the FIFO file at timed intervals using a timer interrupt. All timing is calculated using two values: 1) Buffer length (<code>BUFF_LEN</code>) and 2) Frequency Sampling Rate (<code>Fs</code>). The process reads ADC pin values at <code>1/Fs</code> second intervals. The Frequency of FIFO writing happens at a fraction of the <code>Fs</code>, namely <code>Fs/BUFF_LEN</code> Hz.</p>
<h4>H-Bridge</h4>
<p>The H-Bridge code is responsible for setting up all the GPIO pins required to initialize the H-Bridge breakout board, as well as sending out the appropriate signals to command to the motors. There are several modes of the motor programmed into the H-Bridge, and these modes are called within the interrupt handlers in order to execute the desired motion. The modes are as following:
<p><code>moveCW</code>: move a specified motor clockwise</p>
<p><code>moveCCW</code>: move a specified motor counterclockwise</p>
<p><code>stop</code>: stops a motor from moving</p>
<p><code>standBy</code>: puts the system on standby</p>
</p>
Within each interrupt handler, the aforementioned functions are called in order to move the bot according to the interrupt signals that was sent in from the master code. 
<p><code>left_handler</code>: move left wheel clockwise, and stops the right wheel</p>
<p><code>right_handler</code>: move right wheel clockwise, and stops the left wheel</p>
<p><code>forward_handler</code>: move left wheel counterclockwise, and move right wheel clockwise</p>
<p><code>backward_handler</code>: move left wheel clockwise, and move right wheel counterclockwise</p>
<p><code>standby_handler</code>: calls standby function and places the bot to sleep mode</p>
<h4>Master Code</h4>
<p>All communication of information is done through the master code. The master code is in charge of reading ADC values from the FIFO files as well as receiving and analyzing BT commands received from bluetooth module. This code also sends the appropriate signal  interrupt commands to H-Bridge process for motor control. Master code has a specified sample rate that should be equivalent to ADC component - though the lowest sample period is around 300 ms (due to FIFO reading delays). The master code houses all control logic; analyzing the adc values and bluetooth control to determine what type of motor control is necessary. It sends a corresponding signal interrupt based on the type of desired movement (left, right, forward, backward, standby). </p>



<h3>Compiling the Code</h3>
<p>Within the directory <code>./final_version/</code>, all code can be found (C and Arduino). The Arduino files are in separate folders, one for the whole remote controller module (<code>/accelerometer/accelerometer.ino</code>), and one for setting up the bluetooth connection (<code>/bluetooth/bluetooth.ino</code>). There is a <code>Makefile</code> file that compiles all three C programs corectly. For the Arduino controller code, we used the Arduino IDE to compile and program the microcontroller directly.</p>

<h3>Running the Code</h3>
<p>To run the entire program (inclusive of both compiling and running the programs in the correct order, one can simply run a bash called <code>run_tank</code>, which can also be found in the same folder as the rest of the code. To remove all processes simply call the same bash file with any number of input arguments. e.g. <code>./run_tank 1 </code> </p>
<p>If the remote control is also connected (Arduino w/ accelerometer), you only need to boot the control up and tilt the control in the direction you want the device to go. However, in order to setup the bluetooth connection between the Beaglebone and the Arduino, it needs to be initiated through the Arduino's serial monitor. To do this, place the Arduino's bluetooth module in command mode by typing <code>$$$</code> to the serial monitor. When the LEDs of the bluetooth module starts to flash in quick succession, type in <code>C,addressofdesiredbluetooth</code> to start the connection.</p>