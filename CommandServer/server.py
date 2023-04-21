#!/bin/python3.10
"""
this programn manages all the clients/sessions, and runs the 'smart_shell'
"""
import socket
import sys
import os

from collections import deque
from dataclasses import dataclass
from _thread import *

@dataclass
class Client:
    conn: socket
    addr: tuple
    active: bool = False
    id: str
    message_log: list

class ToolTemplate:
    def __init__(self):
        pass

class Server:
    def __init__(self): #initiates server
        self.__sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM) #socket for server to run on

        self.__max_num_clients_waiting = 10 #max backlog for listen
        self.__sessions = deque([]) #holds all of the clients for main thread to interact with

        self.server_on = True #function to turn server thread on or off
        self.port = int() #will hold port of server
        self.ipaddr = str() #will hold ip adress of server

    def init_server(self):
        try:
            self.__sock.bind((self.ipaddr, self.port))
            self.__sock.listen(self.__max_num_clients_waiting)
        except Exception as e:
            print(e)
            return -1
        print("server sucsessfuly created")
        return 0

    def add_client(self, client: Client) -> int: #
        Client.recv()

        self.__sessions.append(client)

    def run_server(self):
        while self.server_on:
            conn, addr = self.__sock.accept()
            client = Client(conn, addr)
            print("client from %s:%i", adrr[0], addr[1])
            add_client(client)
        self.server_on = True
        return 0

    def handle_session(session_number):
        session = self.__sessions[session_number] #gets the thread from the stack
        if session.active: #if session in use by thread
            return -1 #return -1 because in use
        session.active = True #tells other threads that session is active
        #handle sessionn here
        return 0;

if __name__ == '__main__':
    server = Server()
    server.port = 42069
    server.ipaddr = "0.0.0.0"
    server.init_server()
    server.run_server()
