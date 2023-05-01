import socket


HOST = "10.1.1.14"  # The server's hostname or IP address
PORT = 42069  # The port used by the server

while True:
	try:
		with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
			sock.connect((HOST, PORT))
			print(sock.recv(1024).decode())
			sock.send(b'Command has been expecting you...')
			while True:
				print(sock.recv(1024).decode())
	except KeyboardInterrupt:
		break
	except:
		pass
