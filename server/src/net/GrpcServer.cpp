#include "GrpcServer.h"

#include "Logger.h"

GrpcServer::GrpcServer(GameServiceImpl *service, int port) : service_(service), port_(port) {}

GrpcServer::~GrpcServer()
{
	shutdown();
}

void GrpcServer::start()
{
	std::string address = "0.0.0.0:" + std::to_string(port_);

	grpc::ServerBuilder builder;
	builder.AddListeningPort(address, grpc::InsecureServerCredentials());
	builder.RegisterService(service_);

	server_ = builder.BuildAndStart();
	if (!server_)
	{
		Logger::LogErr("Failed to start gRPC server on " + address);
		return;
	}

	Logger::Log("gRPC server listening on " + address);
	server_->Wait(); // blocks until shutdown() is called
}

void GrpcServer::shutdown()
{
	if (server_)
	{
		server_->Shutdown();
	}
}
