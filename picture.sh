#!/bin/bash

# Executes the Realsense code that generates both images and saves them to the current directory and afterwards calls the python script that creates the POST request to the server.
./libs/librealsense/photoboard/main2 && python combine.py && python staticpic.py
