# ADN SPARE — C Web Server Project

An interactive car and motorcycle spare-parts website served by a small HTTP server written in C.

## Features
- Bright responsive theme with six selectable vehicle catalogues (84 educational parts): Mustang GT, Toyota GR Supra, Honda Civic Type R, Yamaha YZF-R1, Royal Enfield Classic 350, and Ducati Panigale V4.
- Filter the vehicle picker by car or motorcycle.
- Tap a vehicle to reveal an animated exploded-parts view and model-specific component catalogue.
- Rotate real-world component reference photography in a CSS 3D-style viewer, with credits and links to the original Wikimedia Commons files.
- Smooth component fly-in animation and animated catalogue cards with part-photo thumbnails.
- Select components in the exploded view or catalogue to see their descriptions.
- Search and filter parts by engine, braking, electrical, body and suspension.
- Save parts to a browser-persisted shortlist and filter to saved components.
- Vehicle and component photography is illustrative; actual equipment varies by model year, trim and market.
- Accessible keyboard controls and reduced-motion support.
- Responsive layout for mobile and desktop.
- C program handles HTTP requests and serves the website files.

## Project structure
```text
auto_spares_explorer/
├── server.c
├── index.html
├── style.css
├── enhancements.css
├── app.js
└── README.md
```

## Run on Windows
1. Install GCC with MinGW-w64 (if it is not already installed).
2. Open a terminal in this folder.
3. Compile:
   ```bash
   gcc -Wall -Wextra -std=c11 server.c -o server.exe -lws2_32
   ```
   If port 8080 is already in use, compile with `-DPORT=8081` and open **http://localhost:8081** instead.
4. Start the server:
   ```bash
   .\server.exe
   ```
5. Open **http://localhost:8080** in your browser.

Stop the server with `Ctrl + C`.

## Run on Linux / macOS
1. Install GCC (if it is not already installed).
2. Open a terminal in this folder.
3. Compile with `gcc -Wall -Wextra -std=c11 server.c -o server`.
4. Start it with `./server`, then open **http://localhost:8080**.
5. Stop the server with `Ctrl + C`.

## How it works
- `server.c` creates a TCP socket, listens on port 8080, accepts browser connections, and serves requested files with HTTP response headers. It uses Winsock on Windows and POSIX sockets on Linux/macOS.
- `index.html` defines the website structure.
- `style.css` and `enhancements.css` provide the bright automotive theme, responsive layout and animations.
- `app.js` handles model-specific vehicle selection, part-photo rotation, search, category filters and saved parts.

## Important
Vehicle and spare-part reference photos are loaded from Wikimedia Commons, so an internet connection is needed to display them. Vehicle photo credits are listed in the About section; component photo credits and license links appear in the rotating part viewer. Components without a verified photo are explicitly marked instead of showing an unrelated image. The 3D-style rotation is a CSS photo presentation, not a CAD model. The rest of the website runs locally.
