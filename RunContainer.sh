#!/bin/bash

HOST=$1
CONTAINER=animiral/mandalay                       # Replace with local build name if necessary

# XAUTHORITY default
XAUTHORITY="${XAUTHORITY:-$HOME/.Xauthority}"

if [[ -z $HOST ]]
then
	echo "Run Mandalay server."
	DOCKER_ARGS="-p 1234:1234 --name mandalay_server"  # Forward port 1234 for network play
	GAME_ARGS=""
else
	echo "Run Mandalay client, connect to: $HOST"
	DOCKER_ARGS="--net=host --name mandalay_client"    # Let the client see all the servers that the host can see
	GAME_ARGS="$HOST"                             # Pass the server hostname to the game
fi

docker run -t                                    `# Run with a TTY (just for the game to print when it loads config)` \
    --ipc=host                                   `# Allegro 4 requires access to host shared memory facilities` \
    -e DISPLAY=$DISPLAY                          `# Forward the X11 display so we can open a window` \
    -e XAUTHORITY=$XAUTHORITY                    `# Set .Xauthority file path so the container can authenticate with X server` \
    -v $XAUTHORITY:$XAUTHORITY                   `# Mount .Xauthority into the container` \
    -u $(id -u):$(id -g)                         `# Run as host user/group to get access to X11` \
    -v /tmp/.X11-unix:/tmp/.X11-unix             `# Mount the X11 Unix socket for GUI display support` \
    $DOCKER_ARGS                                 `# Server/client specific args` \
    $CONTAINER                                   `# Name of the Docker image to run` \
    $GAME_ARGS                                   `# If host is specified, run client.`
