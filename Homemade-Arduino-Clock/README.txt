----------------------
Homemade Arduino Clock
----------------------

The Homemade Arduino Clock is, as the name suggest, a self-made clock based on Arduino hardware.

The hardware used when creating this contains:
- 1 Arduino UNO
- 1 I2C LCD Module v1.2
- RTC Shield by Velleman (KA07)
- 4 LEDs
- 4 Buttons

When recreating this project, do not forget to add resitors in front of your LEDs. In order to get a clean output from a button it is advised to connect a resitor to a ground pin parallel to your button output pin.

If you were to alter this code or recreate it yourself do note that the Arduino UNO's dynamical memory is limited. I ran into this problem myself, therefore I had to make it more memory efficient.
Running into this problem can cause a lot of headaches, one way I used to reduce the amount of memory drainage is by using as few global variables as possible whilst keeping those who are used in multiple functions still global. Global variables and local variables count to the same maximum storage. Local variables are cleared when a function ends whereas a global variable will be stored as long as the program runs. Because of the amount of variables used in this code I was required to have as much dynamical memory left as possible when running the program.
One way I did this is by limitting the amount of strings used, by trying to keep variables within functions instead of global and by having functions return integers, booleans etc. instead of strings.

In order to run the .ino file, it must sit in a map with the same name.

Created by
Senne Van den Broeck