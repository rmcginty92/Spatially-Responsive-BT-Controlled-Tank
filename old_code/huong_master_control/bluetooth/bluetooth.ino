#include <SoftwareSerial.h>  

int bluetoothTx = 7;  // TX-O pin of bluetooth mate, Arduino D2
int bluetoothRx = 6;  // RX-I pin of bluetooth mate, Arduino D3

SoftwareSerial bluetooth(bluetoothTx, bluetoothRx);

void setup()
{
    Serial.begin(9600);  // Begin the serial monitor at 9600bps

    bluetooth.begin(115200);  // The Bluetooth Mate defaults to 115200bps
    bluetooth.print("$");  // Print three times individually
    bluetooth.print("$");
    bluetooth.print("$");  // Enter command mode
    delay(100);  // Short delay, wait for the Mate to send back CMD
    bluetooth.println("U,9600,N");  // Temporarily Change the baudrate to 9600, no parity
    bluetooth.begin(9600);  // Start bluetooth serial at 9600
    
    // bluetooth.print("$");  // Print three times individually
    // bluetooth.print("$");
    // bluetooth.print("$");  // Enter command mode
    // bluetooth.println("C,0006664E9AD0");
    // delay(180000);
}

void loop()
{
    if(bluetooth.available())  // If the bluetooth sent any characters
    {   
        // Send any characters the bluetooth prints to the serial monitor
        Serial.print((char)bluetooth.read());  
    }
    if(Serial.available())  // If stuff was typed in the serial monitor
    {
        // Send any characters the Serial monitor prints to the bluetooth
        bluetooth.print((char)Serial.read());
    }
}