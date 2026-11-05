FROM gcc:latest

WORKDIR /app

# Copy all files from the subfolder
COPY . .

# Compile server.c into binary executable named app
RUN gcc -Wall -Wextra -O2 -o app server.c

EXPOSE 10000

CMD ["./app"]
