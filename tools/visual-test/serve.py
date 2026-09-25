#!/usr/bin/env python3
"""Static file server for visual tests.

usage: serve.py <root dir> <port>

- serves <root dir> at /  (world-relative URLs like /externprotos/... resolve)
- serves this directory at /__vt/  (viewer.html)
- /__hold?ms=N sleeps N ms then returns a 1x1 gif: holds the page load event
  open so headless Chrome's --screenshot waits for X_ITE to render
- threaded, with a large listen backlog: FreeWRL fetches all textures at once
  and python's default backlog (5) refuses connections
"""
import functools, http.server, os, sys, time, urllib.parse

HERE = os.path.dirname(os.path.abspath(__file__))
GIF = b'GIF89a\x01\x00\x01\x00\x80\x00\x00\x00\x00\x00\xff\xff\xff!\xf9\x04\x01\x00\x00\x00\x00,\x00\x00\x00\x00\x01\x00\x01\x00\x00\x02\x02D\x01\x00;'


class Handler(http.server.SimpleHTTPRequestHandler):
    def translate_path(self, path):
        if path.startswith('/__vt/'):
            return os.path.join(HERE, path[len('/__vt/'):].split('?')[0])
        return super().translate_path(path)

    def do_GET(self):
        url = urllib.parse.urlparse(self.path)
        if url.path == '/__hold':
            ms = int(urllib.parse.parse_qs(url.query).get('ms', ['15000'])[0])
            time.sleep(ms / 1000)
            self.send_response(200)
            self.send_header('Content-Type', 'image/gif')
            self.end_headers()
            self.wfile.write(GIF)
            return
        return super().do_GET()


class Server(http.server.ThreadingHTTPServer):
    request_queue_size = 128
    daemon_threads = True
    allow_reuse_address = True


if __name__ == '__main__':
    root, port = sys.argv[1], int(sys.argv[2])
    Server(('127.0.0.1', port), functools.partial(Handler, directory=root)).serve_forever()
