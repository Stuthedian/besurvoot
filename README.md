## besurvoot
besurvoot (/bɜsär'vut/ - 'illiterate' in [talysh](https://en.wikipedia.org/wiki/Talysh_language) language) - interactive 'tmux send-keys' with vi-like keybindings. Basically a list of commands to send to other panes.

## In action
![Sample text](https://raw.githubusercontent.com/Stuthedian/besurvoot/master/besurvoot.gif)


## Navigation
### Basic
Standard vi-like navigation: '*jk*'. Arrow keys also supported.
### Getting faster
To quickly access particular item type its number followed by '*gg*' or '*G*'.
Up to 3 digits in number supported.


## Managing items
To add an item press '*O*'. There is no feature to somehow "cancel" an input: pressing escape key will insert escape character 
in command.

To delete an item press '*D*'.

If you unsatisfied with the order of items you can rearrange them manually. To do that 
locate the file *.besurvoot_commands* and open it with you favourite text editor.

## Sending a command
To send the command navigate to it and press '*Enter*'. The command will be sent to a previous active pane.

If you want to target specific pane type pane number before pressing '*Enter*'. To get to know pane numbers type *\<tmux-prefix\>q*


## Quit
Just press '*q*'


## Dependencies
Brackets indicate version that is installed on my system:

tmux(2.9a)

ncurses(6.1)

## Acknowledgements

[Terminalizer](https://github.com/faressoft/terminalizer)

[DG_misc.h](https://github.com/DanielGibson/Snippets)
