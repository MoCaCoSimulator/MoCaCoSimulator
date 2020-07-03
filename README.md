Description:
MoCaCo is a framework to simulate, evaluate and compare motion capture systems in a virtual environment. Arbitrary tracking methods and pose reconstruction algorithms can be implemented through provided interfaces. For the simulation characters and animations are loaded from the local drive . The user can place trackers on the characters skin, which are then altered by the selected tracking method. After the simulation an analysis components can be implemented to calculate deviations between the in- output animations.

Controls for 3D scene:
- Alt + Left Mouse: Orbit Camera
- Alt + Middle Mouse: Move Camera
- Alt + Right Mouse: Zoom Camera
- Spacebar: Place trackers

Environment setup:
Download and include any Collade character model by recreating the following folder structure:

```bash
framework_root
|-- assets
|   |   |-- Characters
|   |   |   |-- <character name>
|   |   |   |   |-- animations
|   |   |   |   |   |-- <character_animation_file_1.dae>
|   |   |   |   |   |-- <character_animation_file_2.dae>
|   |   |   |   |   |-- (...)
|   |   |   |   |-- <character_model_file.dae>
|   |-- dev_snmp6
|   |-- Models
|   |-- Shaders
|-- src
|-- (...)
```

Mixamo offers a great database for character models and animations. We recommend this script https://gist.github.com/gnuton/ec2c3c2097f7aeaea8bb7d1256e4b212 for downloading large amounts of animations.

Steps for simulations:
0. Go through the environmental setup
1. Choose and load a character from the top left
2. Select an IK kernel on the top right
3. Attach markers to the characters skin
4. Select tracking virtualizer for every marker
5. Select target IK slot for every marker
6. Add any desired error metric on the bottom right
7. Choose at least one animation from the lower left
8. Run simulation process

Required/Used Third Party Software:
- Assimp 5.0.1 https://github.com/assimp/assimp
- Glew 2.1 https://github.com/nigels-com/glew
- TinyXML2 https://github.com/leethomason/tinyxml2
- FreeImage https://freeimage.sourceforge.io/download.html
- Python >3.0 64 bit
- IMUSim (we used and suggest the Python 3 port from https://github.com/Shmuma/imusim/tree/py3)
- **Qt 5.14.x (open source) if you want to use the provided UI**

