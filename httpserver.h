#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <boost/asio.hpp>

#include <thread>
#include <iostream>

class HttpServer {    
public:
    HttpServer(const std::string &address, unsigned short port, const std::string &directory);
    void Start();    
    void Stop();        

private:
    boost::asio::io_service io_service_;
    boost::asio::ip::tcp::acceptor acceptor_;
    std::vector<std::thread> threads_;
    std::string address_;
    unsigned short port_;  
    std::string directory_;

    static const int THREADS_COUNT = 4;

private:
    void Accept(); 
    void ReadRequest(std::shared_ptr<boost::asio::ip::tcp::socket> socket);
    std::string GetPathFromRequest(std::istream& stream) const;    
    void WriteResponse(std::shared_ptr<boost::asio::ip::tcp::socket> socket, const std::string &requestPath);                   
};
#endif /* HTTP_SERVER_H */
