import json
import ssl
import os
from datetime import datetime
from pathlib import Path
from paho.mqtt import client as mqtt_client

broker = ""
port = 0
topic = ""
username = ""
password = ""

current_file_path = "./dataset"

def create_log_file(directory_path):
    log_dir = Path(directory_path)
    log_dir.mkdir(parents=True, exist_ok=True) #create path if it doesn't exits

    date = datetime.now().strftime("%Y-%m-%d")

    session_num = 1
    while True:
        file_name = f"{date}_log_{session_num}.json" #construct file name
        file_path = log_dir / file_name

        if not file_path.exists(): #if this file path not exist, break out of the search loop
            break
    
        session_num += 1 #increment the session number and check again

    with open(file_path, 'w', encoding='utf-8') as json_file:
        json_file.write("[\n]")

    print(f"Log file created successfully at: {file_path}")
    return file_path

def append_datasample(file_path, data_msg):
    start_idx = data_msg.find('{')
    if start_idx == -1:
        print("Error: No valid JSON payload found in the log string.")
        return
    
    json_payload = data_msg[start_idx:].strip() #get the json section
    
    file = Path(file_path)
    with open(file, 'rb+') as f:
        #Goes to the end of file and find the last ']'
        f.seek(0, os.SEEK_END)
        pos = f.tell()
        found_bracket = False
        while pos > 0:
            pos -= 1
            f.seek(pos)
            if f.read(1) == b']':
                found_bracket = True
                break
        if not found_bracket:
            raise ValueError(f"Malformed JSON: Close bracket ']' not found in {file_path}")

        #Check stuff before ']' to see if the array is empty
        has_elements = False
        check_pos = pos
        while check_pos > 0:
            check_pos -= 1
            f.seek(check_pos)
            prev_char = f.read(1)
            if prev_char not in (b' ', b'\n', b'\r', b'\t'): #Ignore whitespace, newline or carriage return characters while scanning backwards
                if prev_char == b'}':
                    has_elements = True
                    pos = check_pos + 1 
                elif prev_char == b'[':
                    has_elements = False
                break

        #Goes back to the ']' character
        f.seek(pos)

        #Construct json string
        try:
            log_dict = json.loads(json_payload)
        except json.JSONDecodeError:
            print("Error: Extracted string is not valid JSON.")
            return
        dumped_json = json.dumps(log_dict, indent=4)
        formatted_json_string = dumped_json.replace('\n', '\n    ')
        payload_bytes = formatted_json_string.encode('utf-8')
        
        prefix = b",\n    " if has_elements else b"\n    "
        suffix = b"\n]"

        #Overwrite the closing ']' and write back the closed array
        f.write(prefix + payload_bytes + suffix)
        f.truncate()


def connect_mqtt() -> mqtt_client:
    def on_connect(client, userdata, flags, reason_code, properties):
        if reason_code == 0:
            print("Connected to MQTT Broker!")
        else:
            print(f"Failed to connect, return code {reason_code}")
            
    
    client = mqtt_client.Client(
        callback_api_version=mqtt_client.CallbackAPIVersion.VERSION2,
    )
    # client.username_pw_set(username, password)
    client.on_connect = on_connect
    client.tls_set(ca_certs="isrgrootx1.pem", tls_version=ssl.PROTOCOL_TLSv1_2)
    client.username_pw_set(username, password)
    client.connect(broker, port)
    return client


def subscribe(client: mqtt_client):
    def on_message(client, userdata, msg):
        msg_string = msg.payload.decode()
        print(f"Received `{msg_string}` from `{msg.topic}` topic")
        append_datasample(current_file_path, msg_string)

    client.subscribe(topic)
    client.on_message = on_message


def run():
    client = connect_mqtt()
    subscribe(client)
    client.loop_forever()
    

if __name__ == '__main__':
    current_file_path = create_log_file(current_file_path)
    run()