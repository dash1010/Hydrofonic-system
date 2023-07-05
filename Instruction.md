#Instructions
#### Setup:
Connect your hydroponic system to a water tank equipped with a pump. This will enable water circulation within the system.
#### Sensors Installation:
Install the control system within the water tank. This system includes pH, EC, temperature, and water level sensors.
#### Control Elements:
Outside the water tank, position your SHT31 sensor for humidity and temperature and an OLED screen for real-time monitoring. These components will provide essential data about your system’s conditions.
#### Controller:
Connect all these components, including the three peristaltic pumps and the relay module, to the ESP32, which will serve as the system's controller.
#### Peristaltic Pumps:
Attach an acidic pH solution, a basic pH solution, and a liquid fertilizer to the respective peristaltic pumps. Ensure that the tubes from these liquid containers are correctly linked to the water tank.
#### Coding:
Navigate to the provided GitHub page and download the corresponding code. Upload this code to your ESP32 controller.
#### Power Supply:
Connect the ESP32 controller to an electricity source using a USB Type-C cable. Simultaneously, connect the peristaltic pumps and the relay module to a separate 12V power supply.
#### Operation:
Upon powering up the system, all monitored parameters (pH, EC, temperature, etc.) should display on the OLED screen. The control system, driven by the uploaded code, will automatically regulate the pH and EC values, maintaining an optimal environment for your plants.
#### Disclaimer:
Before operation, it is crucial to check the integrity of all components and ensure all sensors are properly calibrated. Incorrect data or malfunctioning components can lead to less-than-optimal plant growth or even damage to the system. Regular maintenance and monitoring are vital for the effective operation of your automated hydroponic system.
