/* HOW TO RUN
   1) Configure things in the Configuration class
   2) Compile: g++ -o bot.exe bot.cpp
   3) Run in loop: while true; do ./bot.exe; sleep 1; done
*/

/* C includes for networking things */
using namespace std;
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

// #include "helper.h"

/* C++ includes */
#include <string>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <sstream>

string buyBond(int id, int price, int size)
{
  string s = "ADD " + to_string(id) + " BOND BUY " + to_string(price) + " " + to_string(size) + "\n";
  return s;
}

string sellBond(int id, int price, int size)
{
  string s = "ADD " + to_string(id) + " BOND SELL " + to_string(price) + " " + to_string(size) + "\n";
  return s;
}

string buyThing(int id, string name, int price, int size){
  string s = "ADD " + to_string(id) + " " + name + " BUY " + to_string(price) + " " + to_string(size) + "\n";
  return s;

}

string sellThing(int id, string name, int price, int size){
  string s = "ADD " + to_string(id) + " " + name + " SELL " + to_string(price) + " " + to_string(size) + "\n";
  return s;

}

string convertADR(int id, string name, string buySell,  int size){
  string s = "CONVERT " + to_string(id) + " " + name + " "  + buySell + " " + to_string(size) + "\n";
  return s;

}

vector<string> parse(string s)
{
  vector<string> v;
  string tmp = "";
  for (auto c : s)
  {
    if (c == ' ')
    {
      v.push_back(tmp);
      tmp = "";
    }
    else
      tmp += c;
  }
  return v;
}

/* The Configuration class is used to tell the bot how to connect
   to the appropriate exchange. The `test_exchange_index` variable
   only changes the Configuration when `test_mode` is set to `true`.
*/
class Configuration {
private:
  /*
    0 = prod-like
    1 = slower
    2 = empty
  */
  static int const test_exchange_index = 0;
public:
  std::string team_name;
  std::string exchange_hostname;
  int exchange_port;
  /* replace REPLACEME with your team name! */
  Configuration(bool test_mode) : team_name("ORCHARD"){
    exchange_port = 20000; /* Default text based port */
    if(true == test_mode) {
      exchange_hostname = "test-exch-" + team_name;
      exchange_port += test_exchange_index;
    } else {
      exchange_hostname = "production";
    }
  }
};

/* Connection establishes a read/write connection to the exchange,
   and facilitates communication with it */
class Connection {
private:
  FILE * in;
  FILE * out;
  int socket_fd;
public:
  Connection(Configuration configuration){
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
      throw std::runtime_error("Could not create socket");
    }
    std::string hostname = configuration.exchange_hostname;
    hostent *record = gethostbyname(hostname.c_str());
    if(!record) {
      throw std::invalid_argument("Could not resolve host '" + hostname + "'");
    }
    in_addr *address = reinterpret_cast<in_addr *>(record->h_addr);
    std::string ip_address = inet_ntoa(*address);
    struct sockaddr_in server;
    server.sin_addr.s_addr = inet_addr(ip_address.c_str());
    server.sin_family = AF_INET;
    server.sin_port = htons(configuration.exchange_port);

    int res = connect(sock, ((struct sockaddr *) &server), sizeof(server));
    if (res < 0) {
      throw std::runtime_error("could not connect");
    }
    FILE *exchange_in = fdopen(sock, "r");
    if (exchange_in == NULL){
      throw std::runtime_error("could not open socket for writing");
    }
    FILE *exchange_out = fdopen(sock, "w");
    if (exchange_out == NULL){
      throw std::runtime_error("could not open socket for reading");
    }

    setlinebuf(exchange_in);
    setlinebuf(exchange_out);
    this->in = exchange_in;
    this->out = exchange_out;
    this->socket_fd = res;
  }

  /** Send a string to the server */
  void send_to_exchange(std::string input) {
    std::string line(input);
    /* All messages must always be uppercase */
    std::transform(line.begin(), line.end(), line.begin(), ::toupper);
    int res = fprintf(this->out, "%s\n", line.c_str());
    if (res < 0) {
      throw std::runtime_error("error sending to exchange");
    }
  }

  /** Read a line from the server, dropping the newline at the end */
  std::string read_from_exchange()
  {
    /* We assume that no message from the exchange is longer
       than 10,000 chars */
    const size_t len = 10000;
    char buf[len];
    if(!fgets(buf, len, this->in)){
      throw std::runtime_error("reading line from socket");
    }

    int read_length = strlen(buf);
    std::string result(buf);
    /* Chop off the newline */
    result.resize(result.length() - 1);
    return result;
  }
};

/** Join a vector of strings together, with a separator in-between
    each string. This is useful for space-separating things */
std::string join(std::string sep, std::vector<std::string> strs) {
  std::ostringstream stream;
  const int size = strs.size();
  for(int i = 0; i < size; ++i) {
    stream << strs[i];
    if(i != (strs.size() - 1)) {
      stream << sep;
    }
  }
  return stream.str();
}


int main(int argc, char *argv[])
{
    // Be very careful with this boolean! It switches between test and prod
    bool test_mode = true;
    Configuration config(test_mode);
    Connection conn(config);

    std::vector<std::string> data;
    data.push_back(std::string("HELLO"));
    data.push_back(config.team_name);
    /*
      A common mistake people make is to conn.send_to_exchange() > 1
      time for every conn.read_from_exchange() response.
      Since many write messages generate marketdata, this will cause an
      exponential explosion in pending messages. Please, don't do that!
    */
    conn.send_to_exchange(join(" ", data));
    std::string line = conn.read_from_exchange();
    std::cout << "The exchange replied: " << line << std::endl;
    
    
    int id=1;
    int count=0;
    int sellColon =0;
    int sellIndex=0;
    vector<string> tokens;
    int colon =0;
    int valbzBuyPrice =0;
    int valbzSellPrice = 0;
    int valeBuyPrice = 0;
    int valeSellPrice = 0;
    int buff = 20;

    while(true) {
      std::string message = conn.read_from_exchange();

      if (std::string(message).find("OPEN") == 0) {
        std::cout << "The round has started"<< 
        '\n';
      }

      if (std::string(message).find("ACK") == 0)
      {
        std::cout << "successfully placed" << '\n';
      }

      if (std::string(message).find("FILL") == 0)
      {
        std::cout << message << '\n';
      }

      //bond strategy
      // if(count%20==0){
      //   conn.send_to_exchange(buyBond(id++, 999, 100));
      //   conn.send_to_exchange(sellBond(id++, 1001, 100));
      // }

      tokens = parse(message);
      bool canBuy = true;
      if(tokens[0] == "BOOK" && tokens[1] == "VALE"){
          string firstValeTuple = tokens[3];
          canBuy = true;
          if(!isdigit(firstValeTuple[0]) || firstValeTuple.size()==0) canBuy = false;

          for(int i=0;i<firstValeTuple.size();i++){
            if(firstValeTuple[i]==':'){
               colon = i;
               break;
            }
          }

          sellIndex = find(tokens.begin(), tokens.end(), "SELL") - tokens.begin();
          string sellValeTuple = tokens[sellIndex+1];
          for(int i=0;i<sellValeTuple.size();i++){
            if(sellValeTuple[i]==':'){
               sellColon = i;
               break;
            }
          }

          if(!isdigit(sellValeTuple[0]) || sellValeTuple.size()==0) canBuy = false;

          //cout << "found value of " << firstValeTuple.substr(0, colon) << '\n';
          if(canBuy) valeBuyPrice = stoi(firstValeTuple.substr(0, colon));
          //cout << "Found vale sell pric eof " << sellValeTuple.substr(0, sellColon) << '\n';
          if(canBuy) valeSellPrice = stoi(sellValeTuple.substr(0, sellColon));
      }

      if(tokens[0]=="BOOK" && tokens[1] == "VALBZ"){
        string firstValbzTuple = tokens[3];

          canBuy = true;
          if(!isdigit(firstValbzTuple[0]) || firstValbzTuple.size()==0) canBuy = false;
          for(int i=0;i<firstValbzTuple.size();i++){
            if(firstValbzTuple[i]==':'){
               colon = i;
               break;
            }
          }
          
          sellIndex = find(tokens.begin(), tokens.end(), "SELL") - tokens.begin();
          string sellValbzTuple = tokens[sellIndex+1];
          for(int i=0;i<sellValbzTuple.size();i++){
            if(sellValbzTuple[i] == ':'){
              sellColon =i;
              break;
            }

          } 

          if(!isdigit(sellValbzTuple[0]) || sellValbzTuple.size()==0) canBuy = false;


          cout << "valbz buy price of " << firstValbzTuple.substr(0, colon) << '\n'; 
          if(canBuy) valbzBuyPrice = stoi(firstValbzTuple.substr(0, colon));
          cout << "valbz sell price of " << sellValbzTuple.substr(0, sellColon) << '\n';
          if(canBuy) valbzSellPrice = stoi(sellValbzTuple.substr(0, sellColon));
          

      }

      if(valeBuyPrice < valbzSellPrice - buff){
          conn.send_to_exchange(buyThing(id, "VALE", valeBuyPrice, 5));
          conn.send_to_exchange(convertADR(id, "VALE", "SELL", 5));
          conn.send_to_exchange(sellThing(id, "VALBZ", valbzSellPrice, 5));
          id++;
      }

      if(valbzBuyPrice < valeSellPrice - buff){
          conn.send_to_exchange(buyThing(id, "VALBZ", valbzBuyPrice, 5));
          conn.send_to_exchange(convertADR(id, "VALBZ", "BUY", 5));
          conn.send_to_exchange(sellThing(id, "VALE", valeSellPrice, 5));
          id++;

      }



      



      


      
      
      
      if(std::string(message).find("CLOSE") == 0) {
        std::cout << "The round has ended" << std::endl;
        break;
      }
      count++;

    }//end while





    return 0;
}
