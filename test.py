import socket

HOST = "10.1.1.14"  # The server's hostname or IP address
PORT = 42069  # The port used by the server

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
	s.connect((HOST, PORT))
	data = s.recv(1024)
	s.sendall(b"Hello, world")

print(f"Received {data!r}")
