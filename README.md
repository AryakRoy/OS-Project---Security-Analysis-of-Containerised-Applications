# Security Analysis of Containerizd Applications

> A security analysis conducted on containerised applications in virtualised environments.

## Introduction

Containerization is defined as a form of operating system virtualization, through which applications are run in isolated user spaces called containers, all using the same shared operating system (OS).
The concept of Containerization is used in many fields such as DevOps, Microservice Architecture, Web servers and Database Sharding. This makes security a major concern. Container technology is increasingly adopted by the industrial community due to two major advantages:

- First, the container orchestration tools such as Docker and Kubernetes facilitate the deployment, scaling, and management of the containerized applications. Therefore, containers are increasingly used in the production environment. Also, the cloud vendors begin to provide container services, like Amazon Fargate, Microsoft Azure Kubernetes Service, etc.

- Second, as an OS-level virtualization technology, the container mechanism is lightweight thus more attractive to the resource-constraint mobile platform.

This aim for this project is to conduct a Security Analysis of Containerized Applications in Virtualized Environments to check for vulnerabilities in the host Operating System by using several methods such as inspecting the Network bridge between the Virtualized and Host Operating System, Figuring Kernel Level Virtualization Errors, Creating a Malicious Application and Test a proof of concept, if the Host OS can be accessed from a container that is running in a virtualized environment.

