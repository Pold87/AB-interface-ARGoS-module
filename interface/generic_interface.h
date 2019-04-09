#ifndef GENERIC_INTERFACE
#define GENERIC_INTERFACE

#include <string>
// By default, all functions are executed in the background and do not return 
// a value

class GethInterface {
private: 
  int robot;
  std::string contractABI;
  std::string contractAddress;
  std::string containerName;
  std::string containerNameBase;
  std::string containerExt;
  std::string containerNameFull;
  std::string containerLocation;
  std::string templatePath;
public:
  GethInterface(int robot, std::string ab, std::string ad, std::string cn, std::string cnb, std::string cl, std::string p);
  std::string readStringFromFile(std::string fileName);
  std::string exec(std::string cmdStr);
  std::string removeSpace(std::string str);
  void dockerExec(std::string cmd);
  void dockerExecForeground(std::string cmd);
  std::string dockerExecReturn(std::string cmd);
  void dockerExecBackground(std::string cmd);
  void execTemplate(std::string templateName);
  void scInterface(std::string function, long long wei);
  void scInterface(std::string function, int arg, long long wei);
  void scInterfaceCall0(std::string function, long long wei);
  std::string scReturn0(std::string function, long long wei);
  void execTemplate(std::string templateName, int numArgs, int args[], long long wei);
  void execGethCmd(std::string command);
  void execGethCmdBackground(std::string command);
  void unlockAccount();
  void startMining();
  void stopMining();
  void addPeer(std::string enode);
  void removePeer(std::string enode);
  std::string getContractABI();
  std::string getEnode();
  void setContractABI(std::string abi);
  std::string getContainerExtension(std::string cn);
  void wrapSSH(std::string cmd);
};
#endif
