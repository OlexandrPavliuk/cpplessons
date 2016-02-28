#include "httpserver.h"
#include <fstream>
#include <boost/filesystem.hpp>


HttpServer::HttpServer(const std::string &address, unsigned short port, const std::string &directory) : 
    address_(address),
    port_(port),
    directory_(directory),
    acceptor_(io_service_)    
{}

void HttpServer::Start()
{    
    if(io_service_.stopped())
    {
        io_service_.reset();
    }

    boost::asio::ip::tcp::endpoint endpoint= boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(address_), port_);

    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(boost::asio::socket_base::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();
     
    Accept(); 
                
    threads_.clear();
    
    for (size_t i = 0; i < THREADS_COUNT; ++i) 
    {
        threads_.emplace_back([this]()
        {
            io_service_.run();
        });
    }
        
    io_service_.run();
    
    for (auto& t: threads_) 
    {
        t.join();
    }
}

void HttpServer::Stop()
{
    acceptor_.close();
    io_service_.stop();
}

void HttpServer::Accept()
{
    std::shared_ptr<boost::asio::ip::tcp::socket> socket(new boost::asio::ip::tcp::socket(io_service_));
                        
    acceptor_.async_accept(*socket, [this, socket](const boost::system::error_code& ec)
    {
        Accept();
                                
        if (!ec) 
        {
            boost::asio::ip::tcp::no_delay option(true);
            socket->set_option(option);
                    
            ReadRequest(socket);
        }
    });
}

void HttpServer::ReadRequest(std::shared_ptr<boost::asio::ip::tcp::socket> socket)
{    
    boost::asio::streambuf streambuf;
    std::istream input(&streambuf);    

    boost::system::error_code ec;
    boost::asio::read_until(*socket, streambuf, "\r\n\r\n", ec);        
    if(!ec) 
    {                                    
        std::string path = GetPathFromRequest(input);                                                    
        WriteResponse(socket, path); 
    }      
}

std::string HttpServer::GetPathFromRequest(std::istream& stream) const
{
    std::string path;

    std::string line;
    getline(stream, line);
    
    size_t method_end = line.find(' ');
    if (method_end != std::string::npos) 
    {
        size_t path_end = line.find(' ', method_end + 1);
        
        path = line.substr(method_end + 1, path_end - method_end - 1);                    
        size_t param = path.find('?');
        if (param != std::string::npos)
        {
            path = path.substr(0, param);
        }
    }

    return path;
}

void HttpServer::WriteResponse(std::shared_ptr<boost::asio::ip::tcp::socket> socket, const std::string &requestPath)
{    
    boost::asio::streambuf streambuf;
    std::ostream response_stream(&streambuf);
    boost::system::error_code ec;
    
    std::ofstream log("/home/box/log.txt");

    boost::filesystem::path root_path(directory_);
    if(boost::filesystem::exists(root_path))    
    {
        auto path = root_path;
        path += requestPath;

        log << "1\n";
                 
        if(boost::filesystem::exists(path)) 
        {
            log << "2\n";
            
            if(boost::filesystem::canonical(root_path) <= boost::filesystem::canonical(path)) 
            {
                log << "3\n";
                
                if(boost::filesystem::is_directory(path))
                {
                    log << "4\n";
                    path+="/index.html";
                }

                if(boost::filesystem::exists(path) && boost::filesystem::is_regular_file(path)) 
                {
                    log << "5\n";
                    size_t length = boost::filesystem::file_size(path);

                    std::ifstream ifs;
                    ifs.open(path.string(), std::ifstream::in | std::ios::binary);
                        
                    if(ifs) 
                    {                                                        
                        response_stream << "HTTP/1.0 200 OK\r\nContent-Length: " << length << "\r\nContent-Type:  text/html\r\n\r\n";
                                                        
                        std::vector<char> buffer;
                        buffer.resize(length);                            
                        
                        ifs.read(&buffer[0], length);
                        response_stream.write(&buffer[0], length);                        
                        boost::asio::write(*socket, streambuf, ec);                        
                        
                        ifs.close();
                        return;
                    }
                }
            }
        }
    }
        
    std::string notFound = "<html><head><title>Not Found</title></head><body>Not Found</body></html>";
    response_stream << "HTTP/1.0 404 NOT FOUND\r\nContent-Length: " << notFound.length() << "\r\nContent-Type: text/html\r\n\r\n";    
    response_stream.write(notFound.c_str(), notFound.length());
    boost::asio::write(*socket, streambuf, ec);                        
            
    ReadRequest(socket);       
}
