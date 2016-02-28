#include "httpserver.h"
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

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

    std::string fullPath = directory_ + "/" + requestPath;
    
    struct stat sb;
    if (stat(fullPath.c_str(), &sb) != -1)
    {
        if (S_ISDIR(sb.st_mode))
	{
	    if (fullPath[fullPath.length() - 1] != '/')
	    {
	       fullPath += "/";
	    }
	    
	    fullPath +="/index.html";
	}
	
	if (stat(fullPath.c_str(), &sb) != -1)
	{
	    if (S_ISREG(sb.st_mode))
	    {
		size_t length = sb.st_size;
		
		std::ifstream ifs;
		ifs.open(fullPath, std::ifstream::in | std::ios::binary);
		    
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
        
    std::string notFound = "<html><head><title>Not Found</title></head><body>Not Found</body></html>";
    response_stream << "HTTP/1.0 404 NOT FOUND\r\nContent-Length: " << notFound.length() << "\r\nContent-Type: text/html\r\n\r\n";    
    response_stream.write(notFound.c_str(), notFound.length());
    boost::asio::write(*socket, streambuf, ec);                        
            
    ReadRequest(socket);       
}
