#ifndef GENERIC_INTERFACE
#define GENERIC_INTERFACE

// By default, all functions are executed in the background and do not return 
// a value

void execTemplate(int robot, std::string templateName);
void execGethCmd(int robot, std::string command);
void startMining(int robot, std::string enode);
void stopMining(int robot);
void addPeer(int robot);
void removePeer(int robot, std::string enode);

#endif
