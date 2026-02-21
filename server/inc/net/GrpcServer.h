#ifndef GRPC_SERVER_H
#define GRPC_SERVER_H

#include "GameServiceImpl.h"

#include <grpcpp/grpcpp.h>

#include <memory>
#include <string>

class GrpcServer
{
  public:
	explicit GrpcServer(GameServiceImpl *service, int port = 4444);
	~GrpcServer();

	GrpcServer(const GrpcServer &) = delete;
	GrpcServer &operator=(const GrpcServer &) = delete;

	// Start serving (blocks the calling thread)
	void start();

	// Shut down the server (can be called from any thread)
	void shutdown();

  private:
	GameServiceImpl *service_;
	int port_;
	std::unique_ptr<grpc::Server> server_;
};

#endif // GRPC_SERVER_H
