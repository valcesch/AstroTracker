import requests
import base64, struct, codecs, folium
from datetime import datetime
import pandas as pd
import json
import pytz
import paho.mqtt.client as mqtt

# This script has been tested on python 3.6
########################################################################################################################
# Astrocast API credentials/keys
apiToken = ""
deviceGuid = ""

# Ublox Thingstream MQTT credentials/keys
hostname = "mqtt.thingstream.io"
device_id = "device:"
username = ""
password = ""

startReceivedDate = "2022-10-1T00:00:00"
filename = "data"
########################################################################################################################


# Global variables
latitude = float("nan")  # Latitude of the tracker in [deg]
longitude = float("nan")  # Longitude of the tracker in [deg]
battery = float("nan")  # Battery voltage in [V]
temperature = float("nan")  # Temperature of the tracker in [degC]
locationDate = float("nan")  # UNIX epoch of the measurement in [s]
SIV = float("nan")  # Number of satellites visible [cnt]
gSpeed = float("nan")  # Ground speed in [m/s]


def message_handler(client, userdata, message):
    json_resp = message.payload.decode('utf-8')
    print(f"received message (Server): {json_resp}")
    print(f"message topic (Server): {message.topic}")
    print(f"message QoS: {message.qos}")

    json_resp = json.loads(json_resp)
    if "Lat" in json_resp:
        global latitude, longitude
        latitude = json_resp["Lat"]
        longitude = json_resp["Lon"]

    # calling disconnect will cause blocking loop to exit, effectively ending the program
    client.disconnect()


# Get data from API
response = requests.get("https://api.astrocast.com/v1/messages?startReceivedDate=" +
                        str(startReceivedDate) + "&deviceGuid=" +
                        str(deviceGuid),
                        headers={"X-Api-Key": str(apiToken)})

# Format data
out_dict = {}
out_entry_idx = 0
if response.status_code == 200:
    if len(response.json()) > 0:
        df = pd.DataFrame.from_dict(response.json())

        for index, row in df.iterrows():

            data = base64.b64decode(row.data).hex()

            if len(data) == 12: # AstroTracker protocol V1
                latitude = row.latitude
                longitude = row.longitude
                temperature = int(data[0:2], 16)
                battery = int(data[2:4], 16) / 10
                locationDate = datetime.utcfromtimestamp(int(struct.unpack("<i", codecs.decode(data[4:12], "hex"))[0])).replace(tzinfo=pytz.UTC)

            elif len(data) == 40: # AstroTracker protocol V2 - LOGGER_TAG_PVT_SLOT
                slot_tag = int(data[0:2], 16)
                locationDate = datetime.utcfromtimestamp(int(struct.unpack("<i", codecs.decode(data[2:10], "hex"))[0])).replace(tzinfo=pytz.UTC)
                latitude = struct.unpack("<i", codecs.decode(data[10:18], "hex"))[0] * 1e-7
                longitude = struct.unpack("<i", codecs.decode(data[18:26], "hex"))[0] * 1e-7
                SIV = int(data[26:28], 16)
                gSpeed = struct.unpack("<i", codecs.decode(data[28:36], "hex"))[0]
                battery = int(data[36:38], 16) / 10
                temperature = int(data[38:40], 16)

            elif len(data) == 42: # AstroTracker protocol V2 - LOGGER_TAG_PVT_SLOT
                slot_tag = int(data[0:2], 16)
                base64_enc_payload = base64.b64encode(bytes.fromhex(data[2:42])).decode()
                str_epoch_payload = row.createdDate

                mqtt_msg = f"{{\"body\": \"{base64_enc_payload}\",\"headers\":{{\"UTCDateTime\":\"{str_epoch_payload}\"}}}}"
                print(f"JSON formatted payload: {mqtt_msg}")

                # Configuration parameters for connecting to MQTT Broker
                mqtt_pub_topic = "CloudLocate/GNSS/request"
                mqtt_sub_topic = f"CloudLocate/{device_id}/GNSS/response"

                if len(mqtt_msg) > 8192:
                    print("Cannot send MQTT message greater than 8KB. Please reduce the number of EPOCHS in configuration parameters to reduce the size.")
                    exit()

                # Configure MQTT client
                client = mqtt.Client(device_id)
                client.username_pw_set(username=username, password=password)
                client.connect(hostname)
                client.on_message = message_handler

                # Subscribe to topic
                if mqtt_sub_topic:
                    client.subscribe(mqtt_sub_topic)

                # Publish message
                client.publish(mqtt_pub_topic, mqtt_msg)
                print(f"message published (client): {mqtt_msg} ")

                # Wait for a position back from CloudLocate
                client.loop_forever()

            out_dict[out_entry_idx] = {"latitude": latitude,
                                       "longitude": longitude,
                                       "battery": battery,
                                       "temperature": temperature,
                                       "locationDate": locationDate,
                                       "SIV": SIV,
                                       "gSpeed": gSpeed}
            out_entry_idx = out_entry_idx + 1
    else:
        print("There is no message.")
else:
    print("Error, something is wrong in the request.")

# Export data to CSV
out_df = pd.DataFrame.from_dict(out_dict, "index")
out_df = out_df.sort_values(by='locationDate')
out_df = out_df[out_df['latitude'].notna()]
out_df.to_csv(filename + ".csv")

# Plot data in a map
points = []
for index, row in out_df.iterrows():
    points.append(tuple([row['latitude'], row['longitude']]))

my_map = folium.Map(location=[out_df['latitude'].mean(), out_df['longitude'].mean()], zoom_start=14)

for each in points:
    folium.Marker(each).add_to(my_map)

folium.PolyLine(points, color="red", weight=2.5, opacity=1).add_to(my_map)

my_map.save(filename + ".html")
