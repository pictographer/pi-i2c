/*
	Socket server with an interface similar to the Arduino Serial class
*/

int CSocket::available() { return 0; } 

void CSocket::begin() { // create the socket and bind
}

int CSocket::read() { 
  // if data is available in the buffer, ,
  // advance the pointer and return a character.
  // otherwise, return -1.
  return 'x'; } 

int CSocket::print(long, format) { // sprintf a long into the buffer
// if there is space available in the buffer.
}

int CSocket::print(char* s) {
// sprintf a string into the buffer if there is space available.
}

int CSocket::print(char c) {
// copy a character into the buffer
}
int CSocket::println(char c);
