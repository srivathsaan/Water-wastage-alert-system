import paho.mqtt.client as mqtt
import sys
import time
# insert at 1, 0 is the script path (or '' in REPL)
sys.path.insert(1, '/home/kaerhikeyan/lcd')

import drivers
from time import sleep
from datetime import datetime

MQTT_ADDRESS = '192.168.250.56'
MQTT_USER = 'sv123'
MQTT_PASSWORD = 'sv123'
MQTT_TOPIC = 'home/+/+'
display = drivers.Lcd()

def on_connect(client, userdata, flags, rc):
    """ The callback for when the client receives a CONNACK response from the server."""
    print('Connected with result code ' + str(rc))
    client.subscribe(MQTT_TOPIC)


def on_message(client, userdata, msg):
    """The callback for when a PUBLISH message is received from the server."""
    print(msg.topic + ' : ' + str(float(msg.payload))+"\n")
    if "output" in msg.topic:
        if float(msg.payload) > 100.0:
            if "floor_2" in msg.topic:
                ar = "Water Lvl High:2"
            else:
                ar = "Water Lvl High:1"
            display.lcd_display_string(ar, 1)
            display.lcd_display_string("Please Turn Off", 2)
            time.sleep(15)
            display.lcd_clear()
            


def main():
    mqtt_client = mqtt.Client()
    mqtt_client.username_pw_set(MQTT_USER, MQTT_PASSWORD)
    mqtt_client.on_connect = on_connect
    mqtt_client.on_message = on_message

    mqtt_client.connect(MQTT_ADDRESS, 1883)
    mqtt_client.loop_forever()


if __name__ == '__main__':
    print('MQTT to InfluxDB bridge')
    main()