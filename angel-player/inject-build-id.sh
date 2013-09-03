#!/bin/bash -xe

sed -ie "s/BuildID=0/BuildID=${BUILD_NUMBER:-0}/g" application.ini
