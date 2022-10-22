# Extra Credit Assignment
## Assumptions
The server needs to:
 - accept connections from multiple clients
 - receive messages, parse out the recipient based on the format `ID::message`, and send the message to the recipient with the ID stripped off.
 - if the recipient is invalid, send an error message to the sender

The client needs to:
 - connect to the server
 - wait for user input, which must be less than 140 characters (including the `ID::`) and send the message to the server
 - receive and print out messages from the server

## Implementation
In order to simplify the client, we will simply check for messages when the user presses enter. This means that the client will not be able to receive messages while the user is typing. This is a reasonable tradeoff for the simplicity of the client.


## To Run

### Server
Same as the base assignment, `./chat_extra` to start the server


### Client
Again, same as base assignment
`./chat_extra -s <server IP> -h <server port>` to start the client