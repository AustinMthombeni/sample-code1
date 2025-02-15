from socket import * 
import sys
import os
import mimetypes  # To determine file type based on extension

if len(sys.argv) <= 1:
    print('Usage: "python ProxyServer.py server_ip"\n[server_ip: It is the IP Address Of Proxy Server]')
    sys.exit(2)

# Create a server socket, bind it to a port and start listening 
serverPort = 6789
tcpSerSock = socket(AF_INET, SOCK_STREAM)
tcpSerSock.bind(('', serverPort))
tcpSerSock.listen(1)
print(f"Proxy server is ready to serve on port {serverPort}...")

while True:
    # Start receiving data from the client 
    print('Ready to serve...')
    tcpCliSock, addr = tcpSerSock.accept()
    print('Received a connection from:', addr)
    
    try:
        message = tcpCliSock.recv(1024).decode()
        print(message)
        
        # Extract the filename from the given message
        try:
            filename = message.split()[1].partition("/")[2]
            filepath = "/" + filename
        except IndexError:
            print("Malformed request.")
            tcpCliSock.close()
            continue

        # Check whether the file exists in the cache
        if os.path.exists(filepath[1:]):
            # Read from the cache
            with open(filepath[1:], "rb") as f:
                outputdata = f.read()

            # Send HTTP headers and cached content to the client
            content_type = mimetypes.guess_type(filepath[1:])[0] or "text/html"
            tcpCliSock.send("HTTP/1.0 200 OK\r\n".encode())
            tcpCliSock.send(f"Content-Type: {content_type}\r\n".encode())
            tcpCliSock.send("\r\n".encode())
            tcpCliSock.send(outputdata)
            print('Read from cache')
        
        else:
            # Create a socket to connect to the web server
            c = socket(AF_INET, SOCK_STREAM)
            hostn = filename.replace("www.", "", 1)
            print(hostn)
            
            try:
                # Connect to the web server on port 80
                c.connect((hostn, 80))
                
                # Send a request to the web server
                c.send(f"GET /{filename} HTTP/1.0\r\nHost: {hostn}\r\n\r\n".encode())
                
                # Read the response from the web server
                buffer = []
                while True:
                    data = c.recv(1024)
                    if not data:
                        break
                    buffer.append(data)

                # Save the response in a cache file
                with open(f"./{filename}", "wb") as tmpFile:
                    for data in buffer:
                        tmpFile.write(data)
                
                # Send the response to the client
                tcpCliSock.send(b"HTTP/1.0 200 OK\r\n")
                content_type = mimetypes.guess_type(filename)[0] or "text/html"
                tcpCliSock.send(f"Content-Type: {content_type}\r\n".encode())
                tcpCliSock.send(b"\r\n")
                for data in buffer:
                    tcpCliSock.send(data)
            
            except Exception as e:
                print(f"Error: {e}")
                tcpCliSock.send(b"HTTP/1.0 404 Not Found\r\n")
                tcpCliSock.send(b"Content-Type: text/html\r\n")
                tcpCliSock.send(b"\r\n")
                tcpCliSock.send(b"<html><body><h1>404 Not Found</h1></body></html>")
            
            c.close()
        
    except Exception as e:
        print(f"General error: {e}")
        tcpCliSock.send(b"HTTP/1.0 404 Not Found\r\n")
        tcpCliSock.send(b"Content-Type: text/html\r\n")
        tcpCliSock.send(b"\r\n")
        tcpCliSock.send(b"<html><body><h1>404 Not Found</h1></body></html>")
    
    finally:
        # Ensure the client socket is closed
        tcpCliSock.close()

tcpSerSock.close()
