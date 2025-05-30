pwd=$(pwd)
# Get your host's UID and GID
export HOST_UID=$(id -u)
export HOST_GID=$(id -g)

if [ "$1" == "build" ]; then
    # Build the Docker image
    docker image build --build-arg UID=$HOST_UID --build-arg GID=$HOST_GID --no-cache -t so2 .

elif [ "$1" == "init" ]; then
    # Run the Docker container
    docker run \
      -e WAYLAND_DISPLAY=$WAYLAND_DISPLAY \
      -v /run/user/$(id -u)/wayland-0:/run/user/$(id -u)/wayland-0 \
      -v /tmp/.X11-unix:/tmp/.X11-unix \
      -e DISPLAY=$DISPLAY \
      -v /etc/machine-id:/etc/machine-id \
      -v /dev/dri:/dev/dri \
      --device=/dev/dri \
      --net=host \
      --volume="$pwd:/home/ubuntu/" \
      -it so2

elif [ "$1" == "Xinit" ]; then
    # Run the Docker container
    xhost +local:docker
    docker run --net=host --env="DISPLAY" --volume="$HOME/.Xauthority:/home/so2/.Xauthority:rw" --volume="$pwd:/home/ubuntu/"  -it so2
    xhost -local:docker
 
elif [ "$1" == "rm" ]; then
    # Remove the Docker container
    docker image rm -f so2
else
    echo "Invalid command. Use './script.sh build' to build or './script.sh init' to initialize."
fi
