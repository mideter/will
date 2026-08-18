#include "proto/messenger.grpc.pb.h"

#include <grpcpp/grpcpp.h>

template class grpc::ClientReaderWriter<will::v1::ClientEvent, will::v1::ServerEvent>;
