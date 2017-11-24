# exo_boricj
This is a sample agent for servicing the lyrics of the_99 bottles of beer_ song:

```99 bottles of beer on the wall, 99 bottles of beer,
take one down, pass it around, 98 bottles of beer on the wall.
98 bottles of beer on the wall, 98 bottles of beer...```

## Protocol
The agent expects messages with `bottles` as a subject. Each part of the message
shall contain a integer, as a string literal, between 0 and 99 inclusive. The
agent then answers each request with a multi-part message containing the lyrics
requested. The agent supports both stream and mailbox messages.

In addition, the agent supports the following pipe messages:

 - `drink_beer`: orders the agent to drink a beer. May malfunction as a result.

## Server
The server supports the following flags:

 - `-d/--drink`: drink a beer at startup. Flag is cumulative.

## Client
A client is supplied for demonstrating purposes. For each line in the standard
input, the client creates a `bottles` message, each part being delimited by
commas in the input. For example, the line `99,98,97` will request the server to
provide the first six lyrics.

The client supports the following flags:

 - `-s/--stream`: operate in stream mode. The client defaults to mailbox mode.
