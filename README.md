# chat-server
A chat-server application for the Network Computing course


# Features to implement
* Multiple clients, one server
* Create and delete chatrooms
    * With password
* Timestamps on messages
* Chat logs (per room)

# Client Designs
1. Lobby
    * Display a list of available chatrooms (with # of users currently connected to each room). 
    * Commandline at the bottom
    * Instructions on available commands above the commandline
    * List of commands
        * C: create a room
            * Prompted to enter a room name
        * E: enter a room
            * Select a room by a room name or an ID (a number displayed)
                * ID is converted to a room name on client-side
        * D: delete a room
            * Can't delete a room if someone is in it
        * R: refresh
            * retrieve possibly updated info from the server
        * T: toggle instructions
        * Scroll using updown & jk
2. Chatroom
    * Commandline at the bottom 

# Modules
1. Server-side
    * chat\_server.c 
        * main file
        * entry point for the server
    * server\_data.c 
        * persistent data management on server side
        * save chatroom info in files 
2. Client-side
    * chat\_client.c
        * main file
        * entry point for the client
    * client\_ui.c
        * deal with ui
        * ncurses
3. Misc
    * c\_vector.c
        * vector implementation in c
