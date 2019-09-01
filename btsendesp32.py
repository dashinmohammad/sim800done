from bluetooth import *
 
BTsocket=BluetoothSocket( RFCOMM )
 
BTsocket.connect(("CC:50:E3:A1:42:D6", 1))
 
BTsocket.send("1")
 
BTsocket.close()
