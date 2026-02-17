#!/usr/bin/env bash
# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    init_docker.sh                                     :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: aguiot-- <aguiot--@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2019/11/18 08:17:08 by aguiot--          #+#    #+#              #
#    Updated: 2020/02/20 14:34:42 by aguiot--         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# https://github.com/alexandregv/42toolbox

# Ensure USER variable is set
[ -z "${USER}" ] && export USER=$(whoami)

################################################################################

# Config
docker_destination="/goinfre/$USER/docker" #=> Select docker destination (goinfre is a good choice)

################################################################################

# Colors
COLOR_INFO=$'\033[1;36m'
COLOR_WARNING=$'\033[1;33m'
COLOR_ERROR=$'\033[1;31m'
COLOR_RESET=$'\033[0m'
COLOR_DATE=$'\033[1;37m'

# Check if the goinfre folder exists
if [ ! -d "/goinfre" ]; then
    echo "${COLOR_DATE}$(date +'%H:%M:%S')${COLOR_RESET} ${COLOR_ERROR}error${COLOR_RESET} Can only install Docker automatically on 42 Macs. The 'goinfre' folder is missing."
    exit 1
fi

# Kill Docker if started, so it doesn't create files during the process
echo "${COLOR_DATE}$(date +'%H:%M:%S')${COLOR_RESET} ${COLOR_INFO}info${COLOR_RESET} Stopping any running Docker processes..."
pkill Docker

# Unlinks all symlinks, if they are
echo "${COLOR_DATE}$(date +'%H:%M:%S')${COLOR_RESET} ${COLOR_INFO}info${COLOR_RESET} Unlinking existing Docker symlinks..."
unlink ~/Library/Containers/com.docker.docker &>/dev/null ;:
unlink ~/Library/Containers/com.docker.helper &>/dev/null ;:
unlink ~/.docker &>/dev/null ;:

# Delete directories if they were not symlinks
echo "${COLOR_DATE}$(date +'%H:%M:%S')${COLOR_RESET} ${COLOR_INFO}info${COLOR_RESET} Removing existing Docker directories..."
rm -rf ~/Library/Containers/com.docker.{docker,helper} ~/.docker &>/dev/null ;:

# Create destination directories in case they don't already exist
echo "${COLOR_DATE}$(date +'%H:%M:%S')${COLOR_RESET} ${COLOR_INFO}info${COLOR_RESET} Creating destination directories for Docker..."
mkdir -p "$docker_destination"/{com.docker.{docker,helper},.docker}

# Make symlinks
echo "${COLOR_DATE}$(date +'%H:%M:%S')${COLOR_RESET} ${COLOR_INFO}info${COLOR_RESET} Creating symlinks for Docker directories..."
ln -sf "$docker_destination"/com.docker.docker ~/Library/Containers/com.docker.docker
ln -sf "$docker_destination"/com.docker.helper ~/Library/Containers/com.docker.helper
ln -sf "$docker_destination"/.docker ~/.docker

# Start Docker for Mac
echo "${COLOR_DATE}$(date +'%H:%M:%S')${COLOR_RESET} ${COLOR_INFO}info${COLOR_RESET} Starting Docker for Mac..."
open -g -a Docker

# Infinite loop to check if Docker daemon started successfully
echo "${COLOR_DATE}$(date +'%H:%M:%S')${COLOR_RESET} ${COLOR_INFO}info${COLOR_RESET} Waiting for Docker daemon to start..."
while true; do
    if docker info > /dev/null 2>&1; then
        echo "${COLOR_DATE}$(date +'%H:%M:%S')${COLOR_RESET} ${COLOR_INFO}info${COLOR_RESET} Docker daemon started successfully!"
        break
    else
        echo "${COLOR_DATE}$(date +'%H:%M:%S')${COLOR_RESET} ${COLOR_WARNING}warning${COLOR_RESET} Docker daemon is not running yet. Retrying in 5 seconds..."
        sleep 5
    fi
done
