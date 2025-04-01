# TODOERMINAL

This is a TUI that allows you to add, edit and delete TODOs.

## Dependencies:

- ncurses.
- sqlite3.

## Usage:

First run the program.

```bash
./main.out
```

Then something like this will appear in your terminal:
![Image](https://github.com/user-attachments/assets/7deedf64-7858-4190-8611-7bdb0eb841ef)
On the right is the information of the selected TODO.
At the right is the list of TODOs, the one with inverted color is the `selected` TODO.
At right is the information of the `selected` TODO.
The main actions are:

- Add a TODO.
- Delete selected TODO.
- Edit selected TODO.
- See completed TODO.
- Quit.

They are triggered when you press the associated key (case insensitive). You can use your arrow keys to select a TODO.

### Editing a TODO:

![Image](https://github.com/user-attachments/assets/31b25bbe-0e12-44f8-bfa1-e8bc30e3fcc2)
Change the information of the selected TODO, you have to press the key in brackets to select which field to change.
