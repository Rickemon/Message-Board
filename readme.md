Message Board Protocol
This document outlines the protocol for communication between the message board server and clients for the CAPS assignment. It describes the general format of exchanges, including request and response types, which are defined using regular expressions and examples.

Request/Response Format
The server understands five types of requests:

POST
LIST
COUNT
READ
EXIT
Requests can be issued in any order, and each occupies a single line of text. The server responds with a single line for each request, with specific formats for different types of requests. Invalid requests will result in a blank line response.

1. POST Request and Response
   The POST request allows users to post a message to a topic.

Format:
^POST(@[^@#]*)#(.+)$

Examples:
POST@Solar system#The dark side of the @moon is #dark.
POST@#x
POST@night_sky#cloudy!
POST@Solar system#Mercury, Venus, Earth, Mars, Jupiter, Saturn, Neptune, Uranus
The topicId starts with '@' and may contain any character except '@' and '#'.
The message is separated by a '#' and must be non-empty (minimum one character).
Both topicId and message can be up to 140 characters long.

Response:
The server appends the message and responds with postId (a non-negative integer).
If the topic does not exist, it is created, and the first post receives postId 0.
2. LIST Request and Response
   The LIST request lists all available topicIds.

Format:
^LIST$

Example Response:
@Solar system#@#@night_sky
The order of topicIds is unspecified.
3. COUNT Request and Response
   The COUNT request retrieves the number of posts for a specific topic.

Format:
^COUNT(@[^@#]*)$

Examples:
COUNT@Solar system
COUNT@solar_system
If the topicId exceeds 140 characters, it will be truncated.

Response:
The server responds with the number of posts or 0 if the topic does not exist.
4. READ Request and Response
   The READ request retrieves a specific post by its postId.

Format:
^READ(@[^@#]*)#([0-9]+)$

Examples:
READ@#0
READ@Solar system#1
READ@night_sky#2
READ@solar_system#0
If the topicId exceeds 140 characters, it will be truncated.

Response:
The server responds with the requested message.
A blank line is returned if the topic does not exist or if postId is invalid.
5. EXIT Request and Response
   The EXIT request terminates the server.

Format:
^EXIT$

Response:
TERMINATING
After receiving this command, the server will not respond to any further requests.
This protocol ensures effective communication between clients and the message board server, promoting a structured interaction format.

