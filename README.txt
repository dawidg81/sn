  sn

Minecraft Classic and ClassiCube server
software.

 = TODO List

    Better command handling
      Checking if the first character of player message
	  is '/' and passing the rest to a separate command
	  parser rather than using if statements for full
	  message content checking for potential command.
    Moderation commands
	  A set of useful commands such as '/kick [player] <reason>',
	  '/mute [player] <reason>', '/ban [player] <reason>'.
	  Requires implementation of parsing ops.txt and banned.txt
	Building commands
	  (CHECK: building commands as a convenience for building
	  does not fit project concept
	  Project concept is to be close to the original Java
	  vanilla server software)
	  A set of convenience commands such as '/fill'

    Third party service's API-level authentication
	  Check verification key sent by player if it's equal to
	  server salt + username, if not, disconnect the player
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