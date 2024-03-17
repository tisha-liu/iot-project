# Automated Urban Farming

## Content Page
- [Introduction](#introduction)
- [Problem Statement](#problem-statement)
- [Objectives](#objectives)
- [System Design](#system-design)
- [Software and Hardware](#software-and-hardware)

## Introduction
Global warming has led to unpredictability in Earth’s climate system, erratic weather patterns, rising temperatures, and extreme events like droughts, floods, and heatwaves. This disrupts agriculture, crippling crop yields and increasing costs. Traditional methods of farming are struggling to adapt to this rapidly changing climate. 

This is where the Internet of Things can be implemented to help aid farmers. By strategically deploying sensors in fields and greenhouses, we can create a real-time, data-driven automated system for plants. These sensors can track crucial factors like soil moisture, and temperature, providing farmers with valuable insights to better maintain their plants. By leveraging the data collected, processes can be automated and adjusted to meet the needs of the different types of plants.

## Problem Statement
In contemporary agriculture, the effective monitoring of environmental conditions is pivotal for optimising crop yields and resource utilisation. However, existing systems often encounter limitations in data granularity, real-time data transmission, and decision-making processes. Challenges persist in seamlessly integrating diverse sensor data, ensuring reliable communication protocols, and facilitating actionable insights for farmers. Additionally, there is a need for user-friendly interfaces that enable intuitive interpretation of data and responsive control of agricultural tools. Addressing these challenges requires innovative solutions that bridge the gap between sensor technologies, data transmission protocols, and user interfaces to enhance agricultural productivity and sustainability.

## Objectives
Our objective is to build an automated solution for urban farming where factors such as soil moisture, temperature, and light are monitored by sensors. Data collected from these sensors will be transmitted to a central server, where users can set thresholds and the respective actions that will take place if the readings fall below the set thresholds. For example, when the water level drops below a set threshold, a valve can be turned on to water the plants.

## System Design
![IoT_Diagram_Updated](https://github.com/tisha-liu/iot-project/assets/46911283/2c737ef7-8d10-4766-bad9-203c916ba21d)

In our proposed system architecture, we intend to employ six M5StickC Plus devices. Each pot will be outfitted with three M5StickC Plus units, with each unit connected to one of three distinct sensors: a Light Sensor, a Soil Moisture Sensor, or a DHT sensor (which measures both temperature and humidity). The data gathered from these sensors will then be consolidated by a fourth M5StickC Plus, which serves as the data collation point. This consolidated data is then transmitted to a fifth M5StickC Plus via Painless Mesh, acting as an intermediary in the data flow. The fifth M5StickC Plus further relays the data to the sixth and final M5StickC Plus, designated as the Central Hub. This Central Hub functions as an MQTT client, publishing the collected data to various topics on the MQTT Broker. Utilising Gravio Studio, we will subscribe to the MQTT Broker to visually present the data. In the event that any sensor anomalies are detected, Gravio will facilitate the sending of push notifications to the end users via email.

## Software and Hardware 

### Hardware
Sensor  | Amount
------------- | -------------
M5StickC Plus   | 7
Light Sensor Module  | 1
Light Sensor Module  | 1
DHT Sensor  | 1
Soil Moisture Sensor  | 1
LED  | 3


### Software
Arduino IDE
More Info: https://www.arduino.cc/en/software

Gravio Studio
More Info: https://www.gravio.com/en/download
