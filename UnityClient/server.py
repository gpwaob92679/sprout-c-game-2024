from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer

class CacheHTTPRequestHandler(SimpleHTTPRequestHandler):
    def end_headers(self):
        self.send_header('Cache-Control', 'max-age=86400')
        super().end_headers()

if __name__ == '__main__':
    httpd = ThreadingHTTPServer(('', 12345), CacheHTTPRequestHandler)
    httpd.serve_forever()

