# Linux Shell

The Linux Shell project is a command-line interpreter that provides a user interface to interact with the Unix based operating system. It allows users to execute commands, run programs, and manage files and processes through a text-based interface.

## Features

- Builtin Utilities: Includes commonly used commands like `cd`, `echo`, `pwd`, and `source`.
- Symbol Table: Stores information about builtin utilities and their corresponding functions.
- Environment Variables: Manage environment variables and set values that affect the behavior of commands.
- Dump Utility: Prints the contents of the local symbol table.
- Command Execution: Executes various commands and utilities available in the shell.
- Pipeline Support: Enables connecting multiple commands together using pipes to pass output as input.

## Getting Started

To use the Linux Shell, follow these steps:

1. Clone the repository:

   ```shell
   git clone https://github.com/Gazi10/linux-shell.git
   
2. Move to the src directory

   ```shell
   cd src
3. Compile the code

   ```shell
   gcc -o shell executor.c init.c main.c node.c parser.c prompt.c token.c source.c builtins/builtins.c builtins/dump.c symbol-table/symbol-table.c
4. Run the executable:

   ```shell
   ./shell

## Presentation
Video/ Demo:
https://drive.google.com/file/d/16E3pkdbvtsaiknKmusZVSzSijSQHB64_/view?usp=sharing
Note: If the video is not playable on the google drive video player, download it and open the file using chrome.

Slides:
https://docs.google.com/presentation/d/1mmDXzEpo4vXf9DQApKq4u2LAKOexP49Ysy-90o0Mtew/edit?usp=sharing
   
## License
This project is licensed under the MIT License.
