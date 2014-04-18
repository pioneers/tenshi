"""
LCM WebSocket Bridge: server application

Usage:
  First modify the SETTINGS section below to have the correct LCM setup
  Then run this script with no arguments
"""
import tornado.ioloop
import tornado.web
import tornado.websocket
import tornado.httpserver
import json
import lcm
import threading

### SETTINGS
import settings
import forseti2
LCM_URI = settings.LCM_URI
TYPES_ROOT = forseti2
### END SETTINGS

class WSHandler(tornado.websocket.WebSocketHandler):
    def open(self):
        """
        Called when a client opens the websocket
        """
        self.lc = lcm.LCM(LCM_URI)
        self.thread = threading.Thread(target=self.lcm_loop)
        self.thread.daemon = True
        self.thread.start()
        self.subscriptions = {}

    def close(self):
        """
        Called when the websocket closes
        """
        # No thread shutdown and LCM cleanup here, because we assume that the
        # program is quitting anyway
        pass

    ### Websocket-related

    def on_message(self, message):
        """
        Called when a message is received over the websocket
        """

        obj = json.loads(message)
        msg_type = obj["type"]
        data = obj["data"]

        if msg_type == "subscribe":
            self.add_subscription(data["channel"],
                                  data["msg_type"],
                                  data["subscription_id"])
        elif msg_type == "unsubscribe":
            self.remove_subscription(data["subscription_id"])
        elif msg_type == "publish":
            self.lc.publish(data["channel"],
                            self.dict_to_lcm(data["data"]).encode())
        else:
            raise Exception, "Invalid websocket message type: " + msg_type

    def ws_send(self, type, data):
        """
        Convenience method for sending data over the websocket
        """
        self.write_message(json.dumps({"type": type, "data": data}))

    ### LCM-related

    def lcm_loop(self):
        """
        Runs the LCM handling loop
        """
        while True:
            try:
                self.lc.handle()
            except Exception as e:
                print "Got exception while handling lcm message", e
                raise

    def add_subscription(self, channel, msg_type, subscription_id):
        """
        Creates an LCM subscription (based on data from a websocket request)
        Forwards any LCM messages received to javascript via websockets
        """
        def handle(channel, data):
            msg = TYPES_ROOT.__getattribute__(msg_type).decode(data)
            self.ws_send("packet", {"subscription_id": subscription_id,
                                    "msg": self.lcm_to_dict(msg)})
        self.subscriptions[subscription_id] = self.lc.subscribe(channel, handle)

    def remove_subscription(self, subscription_id):
        print "UNSUBSCRIBING"
        self.lc.unsubscribe(self.subscriptions[subscription_id])
        del self.subscriptions[subscription_id]

    ### Data conversion

    def is_lcm_object(self, obj):
        """
        Checks if an object is an instance of an LCM type

        LCM offers no official way to do this, so test for a uniquely-named method
        that is present in all LCM types
        """
        return '_get_packed_fingerprint' in dir(obj)

    def lcm_to_dict(self, obj):
        """
        Converts an instance of an LCM object into a dictionary
        """

        res = {}
        for slot in obj.__slots__:
            value = obj.__getattribute__(slot)
            if self.is_lcm_object(value):
                res[slot] = self.lcm_to_dict(value)
            else:
                res[slot] = value
        return res

    def dict_to_lcm(self, d):
        """
        Convert a dictionary holding data for fields into an LCM message object
        """
        msg_cls = TYPES_ROOT.__getattribute__(d["__type__"])
        msg = msg_cls()

        for k, v in d.items():
            if k not in msg.__slots__:
                continue
            if type(v) == dict:
                v = self.dict_to_lcm(v)
            msg.__setattr__(k, v)

        return msg

application = tornado.web.Application([
    (r'/', WSHandler)
])

if __name__ == '__main__':
    http_server = tornado.httpserver.HTTPServer(application)
    http_server.listen(8000)
    tornado.ioloop.IOLoop.instance().start()

