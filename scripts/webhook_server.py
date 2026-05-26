import json
import sys
import time
from http.server import BaseHTTPRequestHandler, HTTPServer
from threading import Thread

port = int(sys.argv[1])
log_path = sys.argv[2]
received = []


class Handler(BaseHTTPRequestHandler):
    def log_message(self, format, *args):
        return

    def do_POST(self):
        length = int(self.headers.get("Content-Length", "0"))
        body = self.rfile.read(length).decode("utf-8")
        received.append(body)
        with open(log_path, "w", encoding="utf-8") as f:
            f.write("\n".join(received))
        self.send_response(200)
        self.end_headers()


server = HTTPServer(("127.0.0.1", port), Handler)
thread = Thread(target=server.serve_forever, daemon=True)
thread.start()
print(f"webhook server on {port}", flush=True)

while True:
    time.sleep(1)
