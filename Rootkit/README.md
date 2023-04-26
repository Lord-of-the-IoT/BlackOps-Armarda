# Remote Access Rootkit
a Linux kernel module with [remote access](#remote-access-server) for post exploitation and surveillence. encrypts all networking traffic. hides server and requires port knocking to add IP adress to view server. when client connects [shell]() between client and rootkit initiated

> **Warning**: the code is still in development and is currently not stable for use

<br>

## **Rootkit functionalities Summary**

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
- server hidden, and only visible to certain IP adresses if correct port knocking sequence used

<br>

## **LKM Build and Installation**
### Build
to build the LKM, install the src directory, and run the Makefile with the `make` command. a number of files will be created, but the rootkit.ko is the important one.
### installation
if built on a different computer, copy the rootkit.ko file to the target machine, then run `insmod core.ko`. normally requires root priveleges to use `insmod`
> **Warning**: if compiled on a different OS or kernel version, the rootkit.ko file may not work, as it is OS and Version specific

<br>

## **Rootkit functionalities**

### Remote Access Server
when the rootkit is installed, a TCP server will be set up, but will hide the port. If a host at a certain IP adress 'port knocks' the correct pre-defined sequence, all clients at the IP adress are able to view the port. communication with the rootkit is easiest to be done through the [BlackOps Armarda C&C server](https://github.com/ArtemisesAngel/BlackOps-Armarda/tree/main/CommandServer), however writing custom tools for communication is possible (but not advised). If extra or different functionality is required, create a python extension for the server (see [here](https://github.com/ArtemisesAngel/BlackOps-Armarda/tree/main/CommandServer/docs/extensions.md))


### hide files/directories
the rootkit hooks the getdents64 and getdents to hide directories and files. if a directory is hidden so is all of the contents. hidden files/diretories will still be visible in memory.  the rootkit can hide/unhide any file by using the command `rootkit:hide <filename/directory>`. the rootkit automatically creates a directory in the /boot directory. files automatically created in the directory includes the configuration file, a file containing data to be sent to the server and a file containing cryptographic information. all files in this directory are encrypted with AES-256 and a hard-coded key, and decrypted in memory when needed to be accesed.
