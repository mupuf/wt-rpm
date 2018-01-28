#!/usr/bin/env python3

import socketserver
import http.server
import threading
import argparse
import socket
import fcntl
import json
import time
import sys
import os

from datetime import datetime
from urllib.parse import urlparse, parse_qs
from mako.template import Template

conf_page="""
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">

<html xmlns="http://www.w3.org/1999/xhtml">
    <head>
        <title>${title}</title>
    </head>
    <body>
        <h1>${title}</h1>

        <p>
            Welcome to the configuration page of WtRPM.
        </p>

        <form action="./">

        <h2>Simple configuration</h2>

        <h3>Machine names</h3>
        <p>
            % for machine_id in state['db']['machines']:
            ${machine_id}'s name: <input type="text" name="${machine_id}" value="${state['db']['machines'][machine_id]['name']}"><br/>
            % endfor
        </p>

        <h3>WtRPM configuration</h3>
        <p>
            WtRPM server's Host: <input type="text" name="wtrpm_host" value="${state['db']['wtrpm']['host']}"><br/>
            WtRPM server's Port: <input type="number" name="wtrpm_port" value="${state['db']['wtrpm']['port']}">
        </p>

        <h2>Advanced configuration (should not be needed)</h2>
        % for machine_id in state['db']['machines']:
        <h3>${machine_id}</h3>
        <table>
            <th>
                <td>Connected?</td>
                <td>GPIO name</td>
                <td>Input?</td>
                <td>Inverted?</td>
                <td>Current state</td>
            <th>
            % for gpio_id in state['db']['machines'][machine_id]['gpios']:
            <%
                gpio = state['gpios'][gpio_id]
            %>
            <tr>
                <td>${gpio.label}</td>
                <td><input type="checkbox" name="${gpio.gpio_id}_connected" value="true" ${"checked" if gpio.connected else ""}></td>
                <td><input type="text" name="${gpio.gpio_id}_name" value="${gpio.gpio_number}"></td>
                <td><input type="checkbox" name="${gpio.gpio_id}_input" value="true" ${"checked" if gpio.input else ""}></td>
                <td><input type="checkbox" name="${gpio.gpio_id}_inverted" value="true" ${"checked" if gpio.inverted else ""}></td>
                <td>${"HIGH" if gpio.poll() else "LOW"}</td>
            <tr>
            % endfor
        </table>
        % endfor

        <p>
            <input type="submit" name="action" value="Save">
        </p>

        </form>
    </body>
</html>
"""

class GPIO:
    def __init__(self, gpio_id, gpio_number, label, input=True, inverted=False, connected=True):
        self.gpio_id = gpio_id
        self.gpio_number = gpio_number
        self.label = label
        self.input = input
        self.inverted = inverted
        self.connected = connected

        self.gpio_dir = "/home/mupuf/Programmation/wt-rpm/scripts/gpio/{}".format(gpio_number)

        with open("{}/direction".format(self.gpio_dir), 'w') as f:
            f.write("in" if input else "out")

    def poll(self):
        with open("{}/value".format(self.gpio_dir), 'r') as f:
            val = int(f.read()) == 1
            if self.inverted:
                return not val
            else:
                return val

class State:
    def __init__(self, state_file="db.json"):
        self.state_file = state_file
        self.lock_file = "db.lock"

    def __grab_lock(self):
        self.lock_fd = open(self.lock_file, 'w')
        try:
            fcntl.flock(self.lock_fd, fcntl.LOCK_EX)
            return True
        except IOError as e:
            print("ERROR: Could not lock the report: " + str(e))
            return False

    def release_lock(self):
        try:
            fcntl.flock(self.lock_fd, fcntl.LOCK_UN)
            self.lock_fd.close()
        except Exception as e:
            print("ERROR: Cannot release the lock: " + str(e))
            pass

    def __reload_state_unlocked(self):
        # check if a report already exists
        try:
            with open(self.state_file, 'rt') as f:
                try:
                    db = json.loads(f.read())
                except Exception as e:
                    print("ERROR: Exception while reading the db: " + str(e))
                    db = dict()

                state = dict()
                state['db'] = db

                # Create the GPIOs
                state['gpios'] = dict()
                for gpio_id in state['db']['gpios']:
                    gpio = state['db']['gpios'][gpio_id]
                    state['gpios'][gpio_id] = GPIO(gpio_id, gpio['name'],
                                                   gpio['label'],
                                                   gpio['input'],
                                                   gpio['inverted'],
                                                   gpio['connected'])

                return state
        except IOError as e:
            self.__log(Criticality.WW, "Cannot open the state file: " + str(e))
            pass
        return None

    def reload_state(self, keep_lock = False):
        self.__grab_lock()
        ret = self.__reload_state_unlocked()
        if not keep_lock:
            self.release_lock()
        return ret

    def save_state(self, state):
        try:
            state_tmp = str(self.state_file) + ".tmp"
            with open(state_tmp, 'wt') as f:
                f.write(json.dumps(state['db'], sort_keys=True, indent=4, separators=(',', ': ')))
                f.close()
                os.rename(state_tmp, self.state_file)
                return True
        except IOError:
            print("ERROR: Could not dump the current state to a file!")
            return False

state = State()

class CustomHTTPHandler(http.server.SimpleHTTPRequestHandler):
    def parse_request(self, *args, **kwargs):
        return super().parse_request(*args, **kwargs)

    def show_main_page(self):
        response = 200
        html = ""

        html = Template(conf_page).render(title="WtRPM configuration page",
                                          state=state.reload_state())

        # Add a footer
        if response == 200:
            date = datetime.now().strftime("%A, %d. %B %Y %H:%M:%S")
            f = "<footer>Autogenerated on {}.</footer>".format(date)
            html += f

        self.send_response(response)
        self.send_header("Content-type", "text/html")
        self.send_header("Content-length", len(html))
        self.send_header("Cache-Control", "no-cache, no-store, must-revalidate")
        self.send_header("Pragma", "no-cache")
        self.send_header("Expires", "0")
        self.end_headers()
        self.wfile.write(str.encode(html))

    def do_GET(self):
        parameters = parse_qs(urlparse(self.path).query)

        action = parameters.get("action", None)

        if action is not None:
            if action == ["Save"]:
                s = state.reload_state(keep_lock=True)
                db = s['db']

                print(db)

                for machine in db['machines']:
                    name = parameters.get(machine, [""])
                    if len(name) > 0:
                        db['machines'][machine]['name'] = name[0]

                wtrpm_host = parameters.get("wtrpm_host", [""])
                if len(wtrpm_host) > 0:
                    db['wtrpm']['host'] = wtrpm_host[0]

                wtrpm_port = parameters.get("wtrpm_port", [""])
                if len(wtrpm_port) > 0:
                    db['wtrpm']['port'] = int(wtrpm_port[0])

                # Then, map the gpios
                for gpio in db['gpios']:
                    for attr in db['gpios'][gpio]:
                        if attr == "label":
                            continue

                        parameter = "{}_{}".format(gpio, attr)
                        if parameter in parameters:
                            value = parameters[parameter][0]

                            if attr == "input" or attr == "connected" or attr == "inverted":
                                db['gpios'][gpio][attr] = True

                            db['gpios'][gpio][attr] = value
                        else:
                            db['gpios'][gpio][attr] = False

                state.save_state(s)
                state.release_lock()

            self.send_response(301)
            self.send_header('Location','/')
            self.end_headers()
        else:
            self.show_main_page()


class ThreadedTCPServer(socketserver.ThreadingMixIn, socketserver.TCPServer):
    def __init__(self, bind_ip, port):
        super().__init__((bind_ip, port), CustomHTTPHandler, bind_and_activate=False)
        self.allow_reuse_address = True
        self.server_bind()
        self.server_activate()
        self.server_thread = threading.Thread(target=self.serve_forever)
        self.server_thread.daemon = True
        self.server_thread.start()

    def __del__(self):
        self.shutdown()
        self.server_close()


parser = argparse.ArgumentParser()
parser.add_argument("--http_server", help="Start an HTTP server to handle configuration. Format: listen_ip:port")
args = parser.parse_args()

if args.http_server is not None:
    fields = args.http_server.split(":")
    conf_server = ThreadedTCPServer(fields[0], int(fields[1]))

while True:
    time.sleep(1)
