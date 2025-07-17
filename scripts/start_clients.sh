#!/bin/bash
tmux kill-ses -t SamuraiCaptures
tmux new-session -d -s SamuraiCaptures -n ceil
tmux new-window -t SamuraiCaptures -n sta
tmux new-window -t SamuraiCaptures -n ap

sh_dir="$(dirname "$(realpath "$0")")"
cam_ip="$1"

# ceil cam (in this same machine)
tmux send-keys -t SamuraiCaptures:ceil "$sh_dir/../builds/ceil_cam.exe $cam_ip" Enter

# sta (minnowboard)
tmux send-keys -t SamuraiCaptures:sta "sshpass -p 'samuraimovel' ssh samurai@10.0.0.31" Enter
tmux send-keys -t SamuraiCaptures:sta "sh ./Samurai/samurai-servers/scripts/STA/start_radio.sh | sleep 5" Enter
tmux send-keys -t SamuraiCaptures:sta "echo \"samuraimovel\" | sudo -S ./Samurai/samurai-servers/builds/sta.exe 10.0.0.20" Enter

# ap (raspberry)
tmux send-keys -t SamuraiCaptures:ap "sshpass -p 'lasse' ssh pi@10.0.0.39" Enter
tmux send-keys -t SamuraiCaptures:ap "./Samurai/samurai-servers/builds/ap.exe 10.0.0.20" Enter

tmux attach -t SamuraiCaptures:sta