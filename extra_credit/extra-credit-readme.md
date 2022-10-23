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

## Test Cases

### Single messages
Any number of messages may be sent at any time using the program. Users never need to wait to send a new message.

The program can handle messages directed to either another user or to the server itself. Messages to another user must be of the format `ID::message`. The server does not announce which user IDs are in use, only the ID of the connected client is made known.

Messages to the server are of the format `message` where `message` does not contain `ID::` as a prefix. These messages are reported on the server standard output for debugging purposes but are not displayed to any client. The server will kindly inform the user that it's message is not formatted correctly for sending to other users.

If a message cannot be delivered because the specified ID is not in use, the client is informed by the server. Error messages from the server are formatted as `Friend: Server: message`.

### Multiple messages
Since a user must press ENTER on the client to receive their unread messages, they may receive multiple messages from doing so. Each message will display as `Friend: message`.

### Spotty connections
Clients may disconnect at will, but they will be given a new user ID if they reconnect. The old user ID is not reused.

## Limitations
- Number of ongoing connections is limited by available resources of the server. While an arbitrary number of connections is possible, it is not reasonably possible to test all potential scenarios involving large numbers of connections.
- The requirements of the assignment note that messages should be delivered to users with the `ID::` portion stripped away. Due to this fact, clients cannot distinguish where messages for them originated. All messages from other clients will begin as `Friend: message`.
- Due to the above limitation, error messages to a client from the server will arrive as `Friend: Server: message`