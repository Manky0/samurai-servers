#!/bin/bash
sh_dir="$(dirname "$(realpath "$0")")"

tmux new-session -d -s MikrotikRadio -n STA1

# Send python server to tmp folder
tmux send-keys -t MikrotikRadio:STA1 "sshpass -p 'lassesemfio' scp $sh_dir/../wil6210_server-samurai-2.2.0 root@192.168.0.1:/tmp/" Enter

# Connect to radio
tmux send-keys -t MikrotikRadio:STA1 "sshpass -p 'lassesemfio' ssh root@192.168.0.1" Enter

# Go to dir
tmux send-keys -t MikrotikRadio:STA1 "cd /tmp/" Enter

# Start mmWave drivers
tmux send-keys -t MikrotikRadio:STA1 "ifconfig wlan0 up" Enter
tmux send-keys -t MikrotikRadio:STA1 "iw dev wlan0 scan" Enter # Fix first first time measurement

# Start server at port 8000
tmux send-keys -t MikrotikRadio:STA1 "python wil6210_server-samurai-2.2.0 8000" Enter

# tmux attach -t MikrotikRadio:STA1
