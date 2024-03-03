#!/bin/bash

# Check for root/sudo permissions
if [ "$EUID" -ne 0 ]; then
    echo "Please run this script with sudo or as root."
    exit 1
fi

# Remove existing files/directories
rm -f /usr/bin/kiwi
rm -rf /usr/lib/kiwilib

# Check for errors in removal process
if [ $? -ne 0 ]; then
    echo "Failed to remove existing files/directories. Installation aborted."
    exit 1
fi

# Install kiwi
cp ./bin/kiwi /usr/bin/kiwi
cp -r ./lib/kiwilib /usr/lib/kiwilib

# Check for errors in installation process
if [ $? -ne 0 ]; then
    echo "Failed to install kiwi. Please check permissions and try again."
    exit 1
fi

echo "Kiwi installed successfully."
