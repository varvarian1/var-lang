#!/bin/bash

echo "Uninstalling VAR..."

sudo rm -f /usr/local/bin/var
sudo rm -rf /usr/local/include/var
sudo rm -f /usr/local/lib/libcore.a
sudo rm -f /usr/local/lib/libutils.a
sudo rm -rf /usr/local/share/var

echo "VAR uninstalled!"