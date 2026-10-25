  sn

Minecraft Classic and ClassiCube server
software.

 = TODO List

    DONE Better command handling
    DONE   Checking if the first character of player message
	DONE   is '/' and passing the rest to a separate command
	DONE   parser rather than using if statements for full
	DONE   message content checking for potential command.

    PARTIALLY DONE Moderation commands
	          DONE   A set of useful commands such as '/kick [player] <reason>',
	         NDONE   '/mute [player] <reason>', '/ban [player] <reason>'.
	         NDONE   Requires implementation of parsing ops.txt and banned.txt
	
    Building commands
	  (CHECK: building commands as a convenience for building
	  does not fit project concept
	  Project concept is to be close to the original Java
	  vanilla server software)
	  A set of convenience commands such as '/fill'

    DONE Third party service's API-level authentication
	DONE   Check verification key sent by player if it's equal to
	DONE   server salt + username, if not, disconnect the player

    Physics (tree/grass growing)
	  Every countdown on the server will rely on
	  global time change. No local timers inside
	  the server process.
	  Every tick is one-twentieth of a second.
	  Every 2000 ticks the server will randomly determine if
	  a tree sapling will turn into a fully grown tree.
	  Every 200 ticks the server will give a dirt block
	  25% chance to turn into a grass block if
	  no other grass blocks are around it,
	  50% chance to turn into a grass block if
	  at least two other grass blocks are around it,
	  60% chance to turn into a grass block if
	  more than two other grass blocks are around it,
	  75% chance to turn into a grass block if
	  two or less dirt blocks are around it.
	  ("around it": blocks placed linearly or diagonally)
    Ping counter for CPE-capable clients

-----------------------------------------------------------
ZONK! Earlier versions than v0.1.7 have been not tagged
      in the repository.
