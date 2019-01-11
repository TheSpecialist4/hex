# Hex
A command line implementation of the game of Hex.

## The game
Like Tic-Tac-Toe, but smarter.

Two players play the game, and the first one to connect the left wall of the board to the right wall (player 1), or the top wall of the board to the right wall (player 2) wins.
### Example
        OX.O
       .OXO
      .XX.
     .OX.
The player with 'X' wins the above game (top and bottom walls of the board connected).

## Usage
~$: `hex p1type p2type [height width | filename]`


#### Player type:
 - m - manual 
 - a - computer
 
 #### Other args:
 - height - height of the board
 - width - width of the board
 - filename - name of the file to load a previously saved game from
 
## Installation
Just run `make` in the directory to create the executable.
