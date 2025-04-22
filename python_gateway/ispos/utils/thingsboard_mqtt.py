import paho.mqtt.client as mqtt
import json

class ThingsBoardClient:
    def __init__(self, host, port, token):
        self.client = mqtt.Client()
        self.client.username_pw_set(token)
        self.client.connect(host, port, 60)

    def send_telemetry(self, data):
        topic = "v1/devices/me/telemetry"
        payload = json.dumps(data)
        self.client.publish(topic, payload)