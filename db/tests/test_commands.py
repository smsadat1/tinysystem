import socket


client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

HOST = '127.0.0.1'
PORT = 9736
client_socket.connect((HOST, PORT))

message = "SET Apple 123\n"
client_socket.sendall(message.encode('utf-8'))
response = client_socket.recv(1024).decode('utf-8')
print(response)

message = "GET Apple\n"
client_socket.sendall(message.encode('utf-8'))
response = client_socket.recv(1024).decode('utf-8')
print(response)

message = "SET Apple 456\n"
client_socket.sendall(message.encode('utf-8'))
response = client_socket.recv(1024).decode('utf-8')
print(response)

message = "GET Apple\n"
client_socket.sendall(message.encode('utf-8'))
response = client_socket.recv(1024).decode('utf-8')
print(response)

message = "EXIST Apple\n"
client_socket.sendall(message.encode('utf-8'))
response = client_socket.recv(1024).decode('utf-8')
print(response)

message = "DELETE Apple\n"
client_socket.sendall(message.encode('utf-8'))
response = client_socket.recv(1024).decode('utf-8')
print(response)

message = "EXIST Apple\n"
client_socket.sendall(message.encode('utf-8'))
response = client_socket.recv(1024).decode('utf-8')
print(response)


# client_socket.close()
