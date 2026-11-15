#!/usr/bin/env python3
"""Tiny MQTT 3.1.1 sink for tests: accepts CONNECT, SUBSCRIBE, PINGREQ and
prints every PUBLISH as 'PUBLISH <topic> <payload>'.  Not a real broker.
  python3 tests/mini_mqtt_broker.py [port]"""
import socket
import sys
import threading


def read_packet(c):
    h = c.recv(1)
    if not h:
        return None, None
    mult, length = 1, 0
    while True:
        b = c.recv(1)[0]
        length += (b & 127) * mult
        mult *= 128
        if not b & 128:
            break
    body = b''
    while len(body) < length:
        chunk = c.recv(length - len(body))
        if not chunk:
            return None, None
        body += chunk
    return h[0], body


def client(c):
    with c:
        while True:
            h, body = read_packet(c)
            if h is None:
                return
            kind = h >> 4
            if kind == 1:                      # CONNECT -> CONNACK accepted
                print('CONNECT', flush=True)
                c.sendall(b'\x20\x02\x00\x00')
            elif kind == 3:                    # PUBLISH
                n = int.from_bytes(body[:2], 'big')
                topic = body[2:2 + n].decode()
                rest = body[2 + n:]
                if (h >> 1) & 3:               # QoS>0: packet id, PUBACK
                    c.sendall(b'\x40\x02' + rest[:2])
                    rest = rest[2:]
                print('PUBLISH', topic, rest.decode(errors='replace'), flush=True)
            elif kind == 8:                    # SUBSCRIBE -> SUBACK QoS 0
                c.sendall(b'\x90\x03' + body[:2] + b'\x00')
            elif kind == 12:                   # PINGREQ
                c.sendall(b'\xd0\x00')
            elif kind == 14:                   # DISCONNECT
                return


def main():
    port = int(sys.argv[1]) if len(sys.argv) > 1 else 18830
    s = socket.socket()
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind(('127.0.0.1', port))
    s.listen()
    print('listening', port, flush=True)
    while True:
        c, _ = s.accept()
        threading.Thread(target=client, args=(c,), daemon=True).start()


if __name__ == '__main__':
    main()
