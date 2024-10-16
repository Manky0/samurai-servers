import socket
import os
import json
import time
import traceback
import ast
import numpy as np
import argparse

def main(args):
    measurement_interval = 0.03

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        # Connect to wil6210_server via TCP socket
        server_addr = (args.server_ip, args.server_port)
        print('Connecting')
        sock.connect(server_addr)
        buf_len = 10240

        # Send scan command
        data = {}
        print('Sending start scan command...')
        data['cmd'] = 'start_scan'
        data['args'] = {}

        m2 = json.dumps(data).encode('utf-8')

        # Send get per beam rss command
        data = {}
        print('Sending per beam rss command...')
        data['cmd'] = 'per_beam_rss_v2x'
        data['args'] = {}

        m = json.dumps(data).encode('utf-8')
        d_last = b''
        skip_cnt = 0
        write_cnt = 0

        end_time = time.time() + args.measurement_time

        while (time.time() < end_time):
            sock.sendall(m2)
            d = b''
            while True:
                d_tmp = sock.recv(buf_len)
                d = d + d_tmp
                if not d_tmp or d_tmp[-1] == 41 or d_tmp[-1] == 93 or d_tmp[-1] == 125:
                    break

            sock.sendall(m)
            d = b''
            while True:
                d_tmp = sock.recv(buf_len)
                d = d + d_tmp
                if not d_tmp or d_tmp[-1] == 41 or d_tmp[-1] == 93 or d_tmp[-1] == 125:
                    break

            time.sleep(measurement_interval/2)
            if d_last == d:
                skip_cnt += 1
                # print('Skip %d replicate RSS.\n' % skip_cnt)
                continue
            else:
                skip_cnt = 0
                write_cnt += 1
                d_last = d

            # Capture Image
            # capture_frame(out)  

            d_dict = d.decode("UTF-8")
            d_dict = ast.literal_eval(d_dict)
            # print('\033c\r')
            print('\nWrite %d RSS:' % write_cnt)

            # Get SNR from patch 
            snr_data = np.array(d_dict['snr_data'])

            print(snr_data)

            time.sleep(measurement_interval/2)

    except Exception as e:
        traceback.print_exc()
        print(e)
    finally:
        print('connection close')
        sock.close()

        # cv2.destroyAllWindows()
        # video_capture.release()

if __name__=="__main__":
    parser = argparse.ArgumentParser(description='Collect per beam RSS trace')
    parser.add_argument('--output', '-o', type=str, default=f'{time.strftime("%Y%m%d_%H%M%S")}', help='Name of the h5 file to save the measurements')
    parser.add_argument('--measurement_time', '--time','-t', type=int, default=60, help='Time in seconds the measurement should take')
    parser.add_argument('--server_ip' , '--ip', type=str, default='200.239.93.46', help='The IP address of client radio')
    parser.add_argument('--server_port', '--port','-p', type=int, default=8000, help='Port number of the Python server')
    parser.add_argument('--radio', type=str, default='', help='Useful if you are measuring at multiple radios at once') # Not used rn
    args = parser.parse_args()

    main(args)