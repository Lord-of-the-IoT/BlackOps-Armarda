# Remote Access Rootkit
a Linux kernel module with [remote access](#remote-access-server) for post exploitation and surveillence. encrypts all networking traffic. hides server and requires port knocking to add IP adress to view server. when client connects [shell]() between client and rootkit initiated

> **Warning**: the code is still in development and is currently not stable for use

## **Rootkit functionalities**

### obscurity
- [hide files/directories](#hide-filesdirectories)
- hide real content of file
- hide the module
- hide processes
- hide open ports
- hide memory
- hide IPv4/Ipv6 packets
- hide sockets
- cleanse logs
- avoid kernel auditing

### surveillence
- record syscalls
- record key presses and mouse
- record network traffic
- record location/elevation
- live camera(s)/screen(s) feed to client
- live mic(s)/speaker(s) feed to client

### security
- encrypts communication with AES-256
- derives session key with MQV-like algorithm
- server hidden, and only visible to certain Ip adresses if correct port knocking sequence used


## **LKM Build and Installation**
### Build
to build the LKM, install the src directory, and run the Makefile with the `make` command. a number of files will be created, but the rootkit.ko is the important one.
### installation
if built on a different computer, copy the rootkit.ko file to the target machine, then run `insmod rootkit.ko`. normally requires root priveleges to use `insmod`
> **Warning**: if compiled on a different OS or kernel version, the rootkit.ko file may not work, as it is OS and Version specific

## **Rootkit functionalities**

### Remote Access Server
when the rootkit is installed, a TCP server will be set up, but will hide the port. If a host at a certain IP adress 'port knocks' the correct pre-defined sequence, all clients at the IP adress are able to view the port. communication with the rootkit is easiest to be done through the [BlackOps Armarda C&C server](https://github.com/ArtemisesAngel/BlackOps-Armarda/tree/main/CommandServer), however writing custom tools for communication is possible (but not advised). If extra or different functionality is required, create a python extension for the server (see [here](https://github.com/ArtemisesAngel/BlackOps-Armarda/tree/main/CommandServer/docs/extensions.md))
